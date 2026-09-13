#!/usr/bin/env bash
set -Eeuo pipefail

BASE="$HOME/Transferências"

die() {
    echo "[ERRO] $*" >&2
    exit 1
}

command -v docker >/dev/null 2>&1 || die "docker não encontrado"
command -v python3 >/dev/null 2>&1 || die "python3 não encontrado"

# Reaproveita a exportação V2 mais recente.
if [[ -n "${EXPORT_DIR:-}" ]]; then
    OUT="$EXPORT_DIR"
else
    OUT="$(
        find "$BASE" -maxdepth 1 -type d \
            -name 'summit_sqlserver_full_export_*' \
            -printf '%T@|%p\n' 2>/dev/null |
        sort -nr |
        head -n1 |
        cut -d'|' -f2-
    )"
fi

[[ -n "${OUT:-}" ]] || die "nenhum summit_sqlserver_full_export_* encontrado"
[[ -f "$OUT/tables.tsv" ]] || die "tables.tsv ausente em $OUT"
[[ -f "$OUT/exports_v2.tsv" ]] || die "exports_v2.tsv ausente em $OUT"

TABLES="$OUT/tables.tsv"
V2="$OUT/exports_v2.tsv"
FINAL="$OUT/exports_final.tsv"
ERRORS="$OUT/errors_v3.log"
RECOVERY="$OUT/recovery_v3.tsv"
FINAL_DATA="$OUT/data_final"
ARCHIVE="${OUT}_FINAL.tar.gz"

mkdir -p "$FINAL_DATA"
: > "$ERRORS"

echo "===================================================================="
echo "R25 - SQL SERVER EXPORT V3 / RECUPERAR SOMENTE TABELAS COM FALHA"
echo "===================================================================="
echo
echo "Base reaproveitada:"
echo "  $OUT"
echo

# --------------------------------------------------------------------
# Container
# --------------------------------------------------------------------
if [[ -n "${MSSQL_CONTAINER:-}" ]]; then
    CTR="$MSSQL_CONTAINER"
    docker inspect "$CTR" >/dev/null 2>&1 \
        || die "MSSQL_CONTAINER não encontrado: $CTR"
else
    mapfile -t candidates < <(
        docker ps --format '{{.ID}}|{{.Image}}|{{.Names}}' |
        awk -F'|' '
            BEGIN { IGNORECASE=1 }
            $2 ~ /mssql|sqlserver|sql-server/ { print $1 }
        '
    )

    (( ${#candidates[@]} == 1 )) || {
        echo "[ERRO] esperado exatamente 1 container SQL Server; encontrei ${#candidates[@]}" >&2
        docker ps --format '  {{.ID}}  {{.Image}}  {{.Names}}' |
            grep -Ei 'mssql|sqlserver|sql-server' >&2 || true
        exit 2
    }

    CTR="${candidates[0]}"
fi

CTR_NAME="$(docker inspect -f '{{.Name}}' "$CTR" | sed 's#^/##')"
echo "Container: $CTR_NAME"
echo

sqlcmd_json() {
    docker exec -i "$CTR" sh -lc '
        set -eu

        if [ -x /opt/mssql-tools18/bin/sqlcmd ]; then
            SQLCMD=/opt/mssql-tools18/bin/sqlcmd
        elif [ -x /opt/mssql-tools/bin/sqlcmd ]; then
            SQLCMD=/opt/mssql-tools/bin/sqlcmd
        elif command -v sqlcmd >/dev/null 2>&1; then
            SQLCMD="$(command -v sqlcmd)"
        else
            echo "[ERRO] sqlcmd não encontrado" >&2
            exit 90
        fi

        PASS="${MSSQL_SA_PASSWORD:-${SA_PASSWORD:-}}"
        [ -n "$PASS" ] || {
            echo "[ERRO] MSSQL_SA_PASSWORD/SA_PASSWORD ausente" >&2
            exit 91
        }

        export SQLCMDPASSWORD="$PASS"

        exec "$SQLCMD" \
            -S localhost \
            -U sa \
            -C \
            -b \
            -y 0
    '
}

sqlcmd_scalar() {
    docker exec -i "$CTR" sh -lc '
        set -eu

        if [ -x /opt/mssql-tools18/bin/sqlcmd ]; then
            SQLCMD=/opt/mssql-tools18/bin/sqlcmd
        elif [ -x /opt/mssql-tools/bin/sqlcmd ]; then
            SQLCMD=/opt/mssql-tools/bin/sqlcmd
        elif command -v sqlcmd >/dev/null 2>&1; then
            SQLCMD="$(command -v sqlcmd)"
        else
            exit 90
        fi

        PASS="${MSSQL_SA_PASSWORD:-${SA_PASSWORD:-}}"
        [ -n "$PASS" ] || exit 91
        export SQLCMDPASSWORD="$PASS"

        exec "$SQLCMD" \
            -S localhost \
            -U sa \
            -C \
            -b \
            -h-1 \
            -W
    '
}

# --------------------------------------------------------------------
# Junta tables.tsv + exports_v2.tsv e identifica somente falhas.
# --------------------------------------------------------------------
JOINED="$OUT/_v3_failed_join.tsv"

python3 - "$TABLES" "$V2" "$JOINED" <<'PY'
import csv
import sys

tables_path, v2_path, out_path = sys.argv[1:4]

tables = {}
with open(tables_path, newline="", encoding="utf-8", errors="replace") as f:
    for row in csv.reader(f, delimiter="|"):
        if len(row) < 7:
            continue
        db, schema, table, qdb, qschema, qtable, rows = [x.strip() for x in row[:7]]
        if not db:
            continue
        tables[(db, schema, table)] = (qdb, qschema, qtable, rows)

failed = []
with open(v2_path, newline="", encoding="utf-8", errors="replace") as f:
    r = csv.reader(f, delimiter="|")
    header = next(r, None)
    for row in r:
        if len(row) < 8:
            continue
        db, schema, table = [x.strip() for x in row[:3]]
        status = row[7].strip()
        if status == "OK":
            continue
        key = (db, schema, table)
        if key not in tables:
            raise SystemExit(f"tabela da V2 não encontrada em tables.tsv: {key}")
        qdb, qschema, qtable, rows = tables[key]
        failed.append((db, schema, table, qdb, qschema, qtable, rows, status))

with open(out_path, "w", newline="", encoding="utf-8") as f:
    w = csv.writer(f, delimiter="|", lineterminator="\n")
    for row in failed:
        w.writerow(row)

print(f"FAILED={len(failed)}")
PY

FAILED_COUNT="$(grep -cve '^[[:space:]]*$' "$JOINED" || true)"
echo "Tabelas a recuperar: $FAILED_COUNT"
[[ "$FAILED_COUNT" -gt 0 ]] || die "nenhuma tabela com falha encontrada na V2"

echo
echo "== 1. Tabelas que serão recuperadas =="

awk -F'|' '{printf "  %s.%s.%s rows_manifest=%s status_v2=%s\n",$1,$2,$3,$7,$8}' "$JOINED"

# --------------------------------------------------------------------
# Export seguro.
#
# Tipos que frequentemente quebram FOR JSON são normalizados:
# - binary/varbinary/image/timestamp/rowversion -> HEX 0x...
# - text/ntext/xml/sql_variant -> NVARCHAR(MAX)
# - hierarchyid -> ToString()
# - geometry/geography -> "SRID:<n>;WKB:<hex>"
#
# O metadata original continua preservado em columns.tsv, portanto a
# futura importação sabe quais campos precisam ser reconvertidos.
# --------------------------------------------------------------------
echo
echo "== 2. Recuperação com serialização segura por tipo =="

printf '%s\n' \
    "database|schema|table|rows_manifest|rows_live|rows_json|json_file|bytes|status" \
    > "$RECOVERY"

N=0
FAIL=0

while IFS='|' read -r db schema table qdb qschema qtable rows_manifest old_status; do
    [[ -n "${db//[[:space:]]/}" ]] || continue
    N=$((N+1))

    safe_db="$(printf '%s' "$db" | sed 's#[^A-Za-z0-9._-]#_#g')"
    safe_schema="$(printf '%s' "$schema" | sed 's#[^A-Za-z0-9._-]#_#g')"
    safe_table="$(printf '%s' "$table" | sed 's#[^A-Za-z0-9._-]#_#g')"

    mkdir -p "$FINAL_DATA/$safe_db"

    seq="$(
        awk -F'|' -v d="$db" -v s="$schema" -v t="$table" '
            $1==d && $2==s && $3==t {print NR; exit}
        ' "$TABLES"
    )"
    [[ -n "$seq" ]] || die "não consegui determinar sequência para $db.$schema.$table"

    rel="data_final/$safe_db/$(printf '%04d' "$seq")__${safe_schema}__${safe_table}.json"
    dest="$OUT/$rel"
    tmp="${dest}.tmp"
    err="${dest}.err"

    printf '[%d/%d] %s.%s.%s ... ' \
        "$N" "$FAILED_COUNT" "$db" "$schema" "$table"

    rows_live="$(
        {
            cat <<SQL
SET NOCOUNT ON;
SELECT COUNT_BIG(*)
FROM ${qdb}.${qschema}.${qtable};
GO
SQL
        } | sqlcmd_scalar 2>>"$ERRORS" | tr -d '\r' | sed '/^[[:space:]]*$/d' | tail -n1 | tr -d '[:space:]'
    )"

    [[ "$rows_live" =~ ^[0-9]+$ ]] || {
        echo "FALHA COUNT"
        echo "COUNT inválido para $db.$schema.$table: [$rows_live]" >> "$ERRORS"
        FAIL=1
        continue
    }

    # O T-SQL abaixo gera dinamicamente um SELECT list seguro.
    if {
        cat <<SQL
:XML ON
SET NOCOUNT ON;
USE ${qdb};

DECLARE @schema sysname = N'${schema//\'/\'\'}';
DECLARE @table  sysname = N'${table//\'/\'\'}';
DECLARE @obj int = OBJECT_ID(QUOTENAME(@schema) + N'.' + QUOTENAME(@table));
DECLARE @cols nvarchar(max);
DECLARE @stmt nvarchar(max);

IF @obj IS NULL
BEGIN
    THROW 50001, 'Tabela não encontrada', 1;
END;

SELECT @cols =
    STRING_AGG(
        CAST(
            CASE
                WHEN ty.name IN (N'binary',N'varbinary',N'image',N'timestamp',N'rowversion')
                    THEN N'CASE WHEN ' + QUOTENAME(c.name) +
                         N' IS NULL THEN NULL ELSE sys.fn_varbintohexstr(CONVERT(varbinary(max),' +
                         QUOTENAME(c.name) + N')) END AS ' + QUOTENAME(c.name)

                WHEN ty.name IN (N'text',N'ntext',N'xml',N'sql_variant')
                    THEN N'CONVERT(nvarchar(max),' + QUOTENAME(c.name) +
                         N') AS ' + QUOTENAME(c.name)

                WHEN ty.name = N'hierarchyid'
                    THEN N'CASE WHEN ' + QUOTENAME(c.name) +
                         N' IS NULL THEN NULL ELSE ' + QUOTENAME(c.name) +
                         N'.ToString() END AS ' + QUOTENAME(c.name)

                WHEN ty.name IN (N'geometry',N'geography')
                    THEN N'CASE WHEN ' + QUOTENAME(c.name) +
                         N' IS NULL THEN NULL ELSE N''SRID:'' + CONVERT(nvarchar(20),' +
                         QUOTENAME(c.name) + N'.STSrid) + N'';WKB:'' + ' +
                         N'sys.fn_varbintohexstr(' + QUOTENAME(c.name) +
                         N'.STAsBinary()) END AS ' + QUOTENAME(c.name)

                ELSE QUOTENAME(c.name)
            END
        AS nvarchar(max)),
        N','
    ) WITHIN GROUP (ORDER BY c.column_id)
FROM sys.columns c
INNER JOIN sys.types ty
    ON ty.user_type_id = c.user_type_id
WHERE c.object_id = @obj;

SET @stmt =
    N'SELECT ' + @cols +
    N' FROM ' + QUOTENAME(@schema) + N'.' + QUOTENAME(@table) +
    N' FOR JSON PATH, INCLUDE_NULL_VALUES;';

EXEC sys.sp_executesql @stmt;
GO
SQL
    } | sqlcmd_json > "$tmp" 2> "$err"; then

        if python3 - "$tmp" "$rows_live" <<'PY' > "${tmp}.count" 2>>"$err"
import json
import sys

path = sys.argv[1]
expected = int(sys.argv[2])

raw = open(path, "rb").read()
text = raw.decode("utf-8-sig").strip()
if not text:
    text = "[]"

data = json.loads(text)
if not isinstance(data, list):
    raise SystemExit("JSON root não é array")

actual = len(data)
if actual != expected:
    raise SystemExit(f"row_count mismatch live={expected} json={actual}")

with open(path, "w", encoding="utf-8") as f:
    json.dump(data, f, ensure_ascii=False, separators=(",", ":"))

print(actual)
PY
        then
            rows_json="$(cat "${tmp}.count")"
            rm -f "${tmp}.count" "$err"
            mv "$tmp" "$dest"
            bytes="$(stat -c '%s' "$dest")"

            echo "OK live=$rows_live json=$rows_json bytes=$bytes"

            printf '%s|%s|%s|%s|%s|%s|%s|%s|OK\n' \
                "$db" "$schema" "$table" "$rows_manifest" "$rows_live" \
                "$rows_json" "$rel" "$bytes" >> "$RECOVERY"
        else
            echo "FALHA JSON"
            {
                echo "===== $db.$schema.$table ====="
                cat "$err" 2>/dev/null || true
                echo
            } >> "$ERRORS"
            mv "$tmp" "${dest}.invalid" 2>/dev/null || true
            rm -f "${tmp}.count" "$err"

            printf '%s|%s|%s|%s|%s|0|%s|0|FAIL\n' \
                "$db" "$schema" "$table" "$rows_manifest" "$rows_live" \
                "${rel}.invalid" >> "$RECOVERY"
            FAIL=1
        fi
    else
        echo "FALHA SQL"
        {
            echo "===== $db.$schema.$table ====="
            cat "$err" 2>/dev/null || true
            echo
        } >> "$ERRORS"
        rm -f "$tmp" "$err"

        printf '%s|%s|%s|%s|%s|0|%s|0|SQL_ERROR\n' \
            "$db" "$schema" "$table" "$rows_manifest" "$rows_live" \
            "$rel" >> "$RECOVERY"
        FAIL=1
    fi

done < "$JOINED"

# --------------------------------------------------------------------
# 3. Montar data_final com as 318 tabelas boas da V2 + recuperadas V3.
# --------------------------------------------------------------------
echo
echo "== 3. Montando dataset final 329 tabelas =="

python3 - "$OUT" "$TABLES" "$V2" "$RECOVERY" "$FINAL" <<'PY'
import csv
import json
import os
import shutil
import sys

root, tables_path, v2_path, recovery_path, final_path = sys.argv[1:6]

tables = []
with open(tables_path, newline="", encoding="utf-8", errors="replace") as f:
    for idx, row in enumerate(csv.reader(f, delimiter="|"), 1):
        if len(row) < 7:
            continue
        db, schema, table, qdb, qschema, qtable, manifest_rows = [x.strip() for x in row[:7]]
        if db:
            tables.append((idx, db, schema, table, int(manifest_rows)))

v2 = {}
with open(v2_path, newline="", encoding="utf-8", errors="replace") as f:
    r = csv.reader(f, delimiter="|")
    next(r, None)
    for row in r:
        if len(row) < 8:
            continue
        key = tuple(x.strip() for x in row[:3])
        v2[key] = {
            "rows": int(row[4]) if row[4].strip().isdigit() else 0,
            "rel": row[5].strip(),
            "bytes": int(row[6]) if row[6].strip().isdigit() else 0,
            "status": row[7].strip(),
        }

rec = {}
with open(recovery_path, newline="", encoding="utf-8", errors="replace") as f:
    r = csv.reader(f, delimiter="|")
    next(r, None)
    for row in r:
        if len(row) < 9:
            continue
        key = tuple(x.strip() for x in row[:3])
        rec[key] = {
            "rows_live": int(row[4]),
            "rows": int(row[5]),
            "rel": row[6].strip(),
            "bytes": int(row[7]),
            "status": row[8].strip(),
        }

os.makedirs(os.path.join(root, "data_final"), exist_ok=True)

out_rows = []
total = 0

for seq, db, schema, table, manifest_rows in tables:
    key = (db, schema, table)
    safe_db = "".join(ch if ch.isalnum() or ch in "._-" else "_" for ch in db)
    safe_schema = "".join(ch if ch.isalnum() or ch in "._-" else "_" for ch in schema)
    safe_table = "".join(ch if ch.isalnum() or ch in "._-" else "_" for ch in table)

    final_rel = f"data_final/{safe_db}/{seq:04d}__{safe_schema}__{safe_table}.json"
    final_abs = os.path.join(root, final_rel)
    os.makedirs(os.path.dirname(final_abs), exist_ok=True)

    if key in rec:
        src_rel = rec[key]["rel"]
        rows = rec[key]["rows"]
        status = rec[key]["status"]
        source = "V3_SAFE"
    else:
        info = v2.get(key)
        if not info or info["status"] != "OK":
            raise SystemExit(f"sem fonte válida para {key}")
        src_rel = info["rel"]
        rows = info["rows"]
        status = "OK"
        source = "V2"

    if status != "OK":
        raise SystemExit(f"tabela ainda falhou: {key} status={status}")

    src_abs = os.path.join(root, src_rel)

    if os.path.abspath(src_abs) != os.path.abspath(final_abs):
        shutil.copy2(src_abs, final_abs)

    # Validação final de parse e count.
    with open(final_abs, "r", encoding="utf-8") as f:
        data = json.load(f)

    if not isinstance(data, list):
        raise SystemExit(f"root JSON inválido: {key}")

    if len(data) != rows:
        raise SystemExit(f"count final diverge: {key} expected={rows} got={len(data)}")

    size = os.path.getsize(final_abs)
    total += rows
    out_rows.append((db, schema, table, manifest_rows, rows, final_rel, size, source, "OK"))

with open(final_path, "w", newline="", encoding="utf-8") as f:
    w = csv.writer(f, delimiter="|", lineterminator="\n")
    w.writerow([
        "database","schema","table","rows_manifest","rows_final",
        "json_file","bytes","source","status"
    ])
    w.writerows(out_rows)

print(f"FINAL_TABLES={len(out_rows)}")
print(f"FINAL_ROWS={total}")
PY

# --------------------------------------------------------------------
# 4. Validação global final
# --------------------------------------------------------------------
echo
echo "== 4. Validação final =="

FINAL_TABLES="$(awk -F'|' 'NR>1 && $9=="OK"{n++} END{print n+0}' "$FINAL")"
FINAL_ROWS="$(awk -F'|' 'NR>1 && $9=="OK"{s+=$5} END{print s+0}' "$FINAL")"
RECOVERED="$(awk -F'|' 'NR>1 && $9=="OK"{n++} END{print n+0}' "$RECOVERY")"
RECOVERY_FAILS="$(awk -F'|' 'NR>1 && $9!="OK"{n++} END{print n+0}' "$RECOVERY")"

echo "Tabelas finais     : $FINAL_TABLES"
echo "Tabelas recuperadas: $RECOVERED"
echo "Falhas recuperação : $RECOVERY_FAILS"
echo "Linhas finais      : $FINAL_ROWS"

if [[ "$FAIL" -ne 0 || "$RECOVERY_FAILS" -ne 0 || "$FINAL_TABLES" -ne 329 ]]; then
    echo
    echo "===================================================================="
    echo "EXPORT V3: PARCIAL"
    echo "===================================================================="
    echo "Erros: $ERRORS"
    exit 4
fi

# O total esperado pelo inventário inicial foi 1391. Se a DB mudou entre
# inventário e V3, aceitamos a soma live/final, mas registramos a diferença.
MANIFEST_ROWS="$(awk -F'|' 'NF>=7 {gsub(/[[:space:]]/,"",$7); if($7~/^[0-9]+$/)s+=$7} END{print s+0}' "$TABLES")"

echo "Linhas manifest    : $MANIFEST_ROWS"

if [[ "$FINAL_ROWS" -ne "$MANIFEST_ROWS" ]]; then
    echo "[AVISO] A quantidade de linhas mudou desde o inventário."
    echo "        Isso pode ocorrer se o SQL Server recebeu alterações entre V2 e V3."
fi

cat > "$OUT/README_FINAL.txt" <<EOF
Summit SQL Server export FINAL
==============================

Finalizado em: $(date --iso-8601=seconds)
Container: $CTR_NAME

SQL Server: READ ONLY

Dataset canônico:
- exports_final.tsv
- data_final/<database>/*.json

Metadata SQL Server:
- databases.tsv
- tables.tsv
- columns.tsv
- indexes.tsv

Histórico:
- exports_v2.tsv / data_v2
- recovery_v3.tsv
- errors_v3.log

Serialização V3 para tipos especiais:
- binary/varbinary/image/timestamp/rowversion => string hexadecimal 0x...
- text/ntext/xml/sql_variant => texto Unicode
- hierarchyid => ToString()
- geometry/geography => SRID + WKB hexadecimal

O tipo original de cada coluna permanece registrado em columns.tsv,
permitindo reconversão controlada na importação para SQLite.
EOF

rm -f "$ARCHIVE"
tar -czf "$ARCHIVE" -C "$BASE" "$(basename "$OUT")"

echo
echo "===================================================================="
echo "EXPORT V3: PASS"
echo "===================================================================="
echo "Dataset : $OUT/data_final"
echo "Manifest: $FINAL"
echo "Archive : $ARCHIVE"
sha256sum "$ARCHIVE"
