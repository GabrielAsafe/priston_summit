#!/usr/bin/env bash
set -Eeuo pipefail

BASE="$HOME/Transferências"

die() {
    echo "[ERRO] $*" >&2
    exit 1
}

ok() {
    echo "[OK] $*"
}

command -v docker >/dev/null 2>&1 || die "docker não encontrado"

# Reaproveita o último inventário completo/ parcial já gerado.
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

[[ -n "${OUT:-}" ]] || die "nenhum diretório summit_sqlserver_full_export_* encontrado"
[[ -f "$OUT/tables.tsv" ]] || die "tables.tsv ausente em $OUT"

TABLES="$OUT/tables.tsv"
EXPORTS="$OUT/exports_v2.tsv"
ERRORS="$OUT/errors_v2.log"
ARCHIVE="${OUT}_v2.tar.gz"

echo "===================================================================="
echo "R25 - EXPORT COMPLETO SQL SERVER DOCKER V2"
echo "===================================================================="
echo
echo "Inventário reaproveitado:"
echo "  $OUT"
echo

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

    if (( ${#candidates[@]} == 0 )); then
        die "nenhum container SQL Server em execução detectado"
    elif (( ${#candidates[@]} > 1 )); then
        echo "[ERRO] mais de um SQL Server detectado:" >&2
        docker ps --format '  {{.ID}}  {{.Image}}  {{.Names}}' |
            grep -Ei 'mssql|sqlserver|sql-server' >&2 || true
        echo "Use MSSQL_CONTAINER=<nome-ou-id>." >&2
        exit 2
    fi

    CTR="${candidates[0]}"
fi

CTR_NAME="$(docker inspect -f '{{.Name}}' "$CTR" | sed 's#^/##')"
CTR_IMAGE="$(docker inspect -f '{{.Config.Image}}' "$CTR")"

echo "Container: $CTR_NAME"
echo "Imagem   : $CTR_IMAGE"
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
            echo "[ERRO] sqlcmd não encontrado no container" >&2
            exit 90
        fi

        PASS="${MSSQL_SA_PASSWORD:-${SA_PASSWORD:-}}"
        if [ -z "$PASS" ]; then
            echo "[ERRO] MSSQL_SA_PASSWORD/SA_PASSWORD ausente no container" >&2
            exit 91
        fi

        export SQLCMDPASSWORD="$PASS"

        # IMPORTANTE:
        # - sem -h/-W junto de -y 0;
        # - :XML ON no input para FOR JSON, conforme sqlcmd.
        exec "$SQLCMD" \
            -S localhost \
            -U sa \
            -C \
            -b \
            -y 0
    '
}

: > "$ERRORS"

echo "== 1. Self-test JSON do sqlcmd =="

SELFTEST="$OUT/_json_selftest_v2.json"
SELFERR="$OUT/_json_selftest_v2.err"

if ! {
    cat <<'SQL'
:XML ON
SET NOCOUNT ON;
SELECT 1 AS [ok], N'áéíóú ç' AS [unicode]
FOR JSON PATH, INCLUDE_NULL_VALUES;
GO
SQL
} | sqlcmd_json > "$SELFTEST" 2> "$SELFERR"; then
    echo "[ERRO] self-test SQL falhou"
    cat "$SELFERR" >&2 || true
    exit 3
fi

python3 - "$SELFTEST" <<'PY'
import json, sys
p = sys.argv[1]
raw = open(p, "rb").read()
text = raw.decode("utf-8-sig").strip()
obj = json.loads(text)
assert isinstance(obj, list)
assert obj and obj[0]["ok"] == 1
print("[OK] JSON sqlcmd válido:", text)
PY

rm -f "$SELFERR"

echo
echo "== 2. Exportando todas as tabelas =="

mkdir -p "$OUT/data_v2"

printf '%s\n' \
    "database|schema|table|rows_manifest|rows_json|json_file|bytes|status" \
    > "$EXPORTS"

TABLE_COUNT="$(grep -cve '^[[:space:]]*$' "$TABLES" || true)"
N=0
FAIL=0

while IFS='|' read -r db schema table qdb qschema qtable rows; do
    [[ -n "${db//[[:space:]]/}" ]] || continue

    trim() {
        printf '%s' "$1" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//'
    }

    db="$(trim "$db")"
    schema="$(trim "$schema")"
    table="$(trim "$table")"
    qdb="$(trim "$qdb")"
    qschema="$(trim "$qschema")"
    qtable="$(trim "$qtable")"
    rows="$(trim "$rows")"

    N=$((N+1))

    safe_db="$(printf '%s' "$db" | sed 's#[^A-Za-z0-9._-]#_#g')"
    safe_schema="$(printf '%s' "$schema" | sed 's#[^A-Za-z0-9._-]#_#g')"
    safe_table="$(printf '%s' "$table" | sed 's#[^A-Za-z0-9._-]#_#g')"

    dir="$OUT/data_v2/$safe_db"
    mkdir -p "$dir"

    rel="data_v2/$safe_db/$(printf '%04d' "$N")__${safe_schema}__${safe_table}.json"
    dest="$OUT/$rel"
    tmp="${dest}.tmp"
    err="${dest}.err"

    printf '[%d/%d] %s.%s.%s rows=%s ... ' \
        "$N" "$TABLE_COUNT" "$db" "$schema" "$table" "$rows"

    if {
        cat <<SQL
:XML ON
SET NOCOUNT ON;
SELECT *
FROM ${qdb}.${qschema}.${qtable}
FOR JSON PATH, INCLUDE_NULL_VALUES;
GO
SQL
    } | sqlcmd_json > "$tmp" 2> "$err"; then

        if python3 - "$tmp" "$rows" <<'PY' > "${tmp}.count" 2>>"$err"
import json, sys

p = sys.argv[1]
expected = int(sys.argv[2])

raw = open(p, "rb").read()
text = raw.decode("utf-8-sig").strip()

if not text:
    text = "[]"

data = json.loads(text)

if not isinstance(data, list):
    raise SystemExit("JSON root is not an array")

actual = len(data)

if actual != expected:
    raise SystemExit(f"row count mismatch expected={expected} actual={actual}")

# Reescreve em UTF-8 normalizado, sem BOM e sem whitespace supérfluo.
with open(p, "w", encoding="utf-8") as f:
    json.dump(data, f, ensure_ascii=False, separators=(",", ":"))

print(actual)
PY
        then
            actual="$(cat "${tmp}.count")"
            rm -f "${tmp}.count" "$err"
            mv "$tmp" "$dest"
            bytes="$(stat -c '%s' "$dest")"

            echo "OK rows=$actual bytes=$bytes"

            printf '%s|%s|%s|%s|%s|%s|%s|OK\n' \
                "$db" "$schema" "$table" "$rows" "$actual" \
                "$rel" "$bytes" >> "$EXPORTS"
        else
            echo "FALHA JSON/COUNT"
            {
                echo "===== $db.$schema.$table ====="
                cat "$err" 2>/dev/null || true
                echo
            } >> "$ERRORS"

            mv "$tmp" "${dest}.invalid" 2>/dev/null || true
            rm -f "${tmp}.count" "$err"

            printf '%s|%s|%s|%s|0|%s|0|INVALID_JSON_OR_COUNT\n' \
                "$db" "$schema" "$table" "$rows" "${rel}.invalid" \
                >> "$EXPORTS"

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

        printf '%s|%s|%s|%s|0|%s|0|SQL_ERROR\n' \
            "$db" "$schema" "$table" "$rows" "$rel" \
            >> "$EXPORTS"

        FAIL=1
    fi
done < "$TABLES"

echo
echo "== 3. Validação global =="

OK_COUNT="$(grep -c '|OK$' "$EXPORTS" || true)"
BAD_COUNT="$(grep -Ec '\|(INVALID_JSON_OR_COUNT|SQL_ERROR)$' "$EXPORTS" || true)"

MANIFEST_ROWS="$(awk -F'|' 'NF >= 7 {gsub(/[[:space:]]/,"",$7); if ($7 ~ /^[0-9]+$/) s += $7} END {print s+0}' "$TABLES")"
JSON_ROWS="$(awk -F'|' 'NR > 1 && $8 == "OK" {s += $5} END {print s+0}' "$EXPORTS")"

echo "Bancos                : $(cut -d'|' -f1 "$TABLES" | sed '/^[[:space:]]*$/d' | sort -u | wc -l)"
echo "Tabelas inventariadas : $TABLE_COUNT"
echo "Tabelas exportadas OK : $OK_COUNT"
echo "Tabelas com falha     : $BAD_COUNT"
echo "Rows manifest         : $MANIFEST_ROWS"
echo "Rows JSON             : $JSON_ROWS"

if [[ "$FAIL" -ne 0 || "$OK_COUNT" -ne "$TABLE_COUNT" || "$MANIFEST_ROWS" -ne "$JSON_ROWS" ]]; then
    echo
    echo "===================================================================="
    echo "EXPORT V2: PARCIAL"
    echo "===================================================================="
    echo "Erros:"
    echo "  $ERRORS"
    exit 4
fi

rm -f "$SELFTEST"

cat > "$OUT/README_V2.txt" <<EOF
Summit SQL Server full export V2
================================

Gerado/validado em: $(date --iso-8601=seconds)
Container: $CTR_NAME
Imagem: $CTR_IMAGE

Modo SQL Server: READ ONLY

Inventário original:
- databases.tsv
- tables.tsv
- columns.tsv
- indexes.tsv

Dados V2:
- exports_v2.tsv
- data_v2/<database>/*.json
- errors_v2.log

Validação:
- todas as tabelas do manifest exportadas
- cada JSON validado por parser
- row_count de cada JSON comparado ao manifest
- total de rows manifest == total de rows JSON
EOF

rm -f "$ARCHIVE"
tar -czf "$ARCHIVE" -C "$BASE" "$(basename "$OUT")"

echo
echo "Tamanho:"
du -sh "$OUT" "$ARCHIVE"

echo
echo "===================================================================="
echo "EXPORT V2: PASS"
echo "===================================================================="
echo "Diretório: $OUT"
echo "Arquivo  : $ARCHIVE"
sha256sum "$ARCHIVE"
