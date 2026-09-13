#!/usr/bin/env bash
set -Eeuo pipefail

BASE="$HOME/Transferências"

die() {
    echo "[ERRO] $*" >&2
    exit 1
}

command -v docker >/dev/null 2>&1 || die "docker não encontrado"
command -v python3 >/dev/null 2>&1 || die "python3 não encontrado"

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
[[ -f "$OUT/tables.tsv" ]] || die "tables.tsv ausente"
[[ -f "$OUT/exports_v2.tsv" ]] || die "exports_v2.tsv ausente"

TABLES="$OUT/tables.tsv"
V2="$OUT/exports_v2.tsv"
FAILED="$OUT/_v4_failed.tsv"
RECOVERY="$OUT/recovery_v4.tsv"
FINAL="$OUT/exports_final.tsv"
ERRORS="$OUT/errors_v4.log"
ARCHIVE="${OUT}_FINAL.tar.gz"

mkdir -p "$OUT/data_final"
: > "$ERRORS"

echo "===================================================================="
echo "R25 - EXPORT SQL SERVER V4 / FECHAR 329 DE 329"
echo "===================================================================="
echo
echo "Base:"
echo "  $OUT"
echo

if [[ -n "${MSSQL_CONTAINER:-}" ]]; then
    CTR="$MSSQL_CONTAINER"
else
    mapfile -t cands < <(
        docker ps --format '{{.ID}}|{{.Image}}|{{.Names}}' |
        awk -F'|' '
            BEGIN { IGNORECASE=1 }
            $2 ~ /mssql|sqlserver|sql-server/ { print $1 }
        '
    )
    (( ${#cands[@]} == 1 )) || die "esperado exatamente 1 container SQL Server"
    CTR="${cands[0]}"
fi

docker inspect "$CTR" >/dev/null 2>&1 || die "container inválido: $CTR"

CTR_NAME="$(docker inspect -f '{{.Name}}' "$CTR" | sed 's#^/##')"
echo "Container: $CTR_NAME"
echo

sqlcmd_raw() {
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
            -y 0 \
            -w 65535
    '
}

sqlcmd_scalar() {
    docker exec -i "$CTR" sh -lc '
        set -eu

        if [ -x /opt/mssql-tools18/bin/sqlcmd ]; then
            SQLCMD=/opt/mssql-tools18/bin/sqlcmd
        elif [ -x /opt/mssql-tools/bin/sqlcmd ]; then
            SQLCMD=/opt/mssql-tools/bin/sqlcmd
        else
            SQLCMD="$(command -v sqlcmd)"
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

python3 - "$TABLES" "$V2" "$FAILED" <<'PY'
import csv, sys

tables_path, v2_path, out_path = sys.argv[1:4]

meta = {}
with open(tables_path, newline="", encoding="utf-8", errors="replace") as f:
    for seq, row in enumerate(csv.reader(f, delimiter="|"), 1):
        if len(row) < 7:
            continue
        vals = [x.strip() for x in row[:7]]
        if not vals[0]:
            continue
        db, schema, table, qdb, qschema, qtable, rows = vals
        meta[(db,schema,table)] = (seq,qdb,qschema,qtable,int(rows))

failed = []
with open(v2_path, newline="", encoding="utf-8", errors="replace") as f:
    r = csv.reader(f, delimiter="|")
    next(r, None)
    for row in r:
        if len(row) < 8:
            continue
        db, schema, table = [x.strip() for x in row[:3]]
        status = row[7].strip()
        if status == "OK":
            continue
        seq,qdb,qschema,qtable,rows = meta[(db,schema,table)]
        failed.append((seq,db,schema,table,qdb,qschema,qtable,rows,status))

with open(out_path,"w",newline="",encoding="utf-8") as f:
    w = csv.writer(f,delimiter="|",lineterminator="\n")
    w.writerows(failed)

print(f"FAILED={len(failed)}")
PY

FAILED_COUNT="$(grep -cve '^[[:space:]]*$' "$FAILED" || true)"
echo "Tabelas pendentes: $FAILED_COUNT"
[[ "$FAILED_COUNT" -eq 11 ]] || die "esperava 11 falhas da V2, encontrei $FAILED_COUNT"

echo
echo "== 1. Self-test sem XML mode =="

SELF="$OUT/_v4_selftest.json"

{
cat <<'SQL'
SET NOCOUNT ON;
SELECT
    REPLICATE(CAST(N'abc123' AS nvarchar(max)), 1000) AS [long_text],
    N'áéíóú ç' AS [unicode]
FOR JSON PATH, INCLUDE_NULL_VALUES;
GO
SQL
} | sqlcmd_raw 2>>"$ERRORS" | tr -d '\r\n' > "$SELF"

python3 - "$SELF" <<'PY'
import json, sys
p=sys.argv[1]
data=json.load(open(p,encoding="utf-8-sig"))
assert len(data)==1
assert len(data[0]["long_text"])==6000
assert data[0]["unicode"]=="áéíóú ç"
print("[OK] JSON longo remontado corretamente")
PY

rm -f "$SELF"

echo
echo "== 2. Recuperando somente as 11 tabelas =="

printf '%s\n' \
  "database|schema|table|rows_manifest|rows_live|rows_json|json_file|bytes|status" \
  > "$RECOVERY"

N=0
FAIL=0

while IFS='|' read -r seq db schema table qdb qschema qtable rows_manifest old_status; do
    [[ -n "$db" ]] || continue
    N=$((N+1))

    safe_db="$(printf '%s' "$db" | sed 's#[^A-Za-z0-9._-]#_#g')"
    safe_schema="$(printf '%s' "$schema" | sed 's#[^A-Za-z0-9._-]#_#g')"
    safe_table="$(printf '%s' "$table" | sed 's#[^A-Za-z0-9._-]#_#g')"

    mkdir -p "$OUT/data_final/$safe_db"

    rel="data_final/$safe_db/$(printf '%04d' "$seq")__${safe_schema}__${safe_table}.json"
    dest="$OUT/$rel"
    tmp="${dest}.tmp"
    err="${dest}.err"

    printf '[%d/%d] %s.%s.%s ... ' "$N" "$FAILED_COUNT" "$db" "$schema" "$table"

    rows_live="$(
        {
            cat <<SQL
SET NOCOUNT ON;
SELECT COUNT_BIG(*) FROM ${qdb}.${qschema}.${qtable};
GO
SQL
        } | sqlcmd_scalar 2>>"$ERRORS" | tr -d '\r' | sed '/^[[:space:]]*$/d' | tail -n1 | tr -d '[:space:]'
    )"

    [[ "$rows_live" =~ ^[0-9]+$ ]] || {
        echo "FALHA COUNT"
        echo "$db.$schema.$table COUNT inválido [$rows_live]" >> "$ERRORS"
        FAIL=1
        continue
    }

    # FOR JSON já escapa controles dentro dos valores.
    # O tr remove SOMENTE quebras físicas inseridas pelo sqlcmd na saída.
    if {
        cat <<SQL
SET NOCOUNT ON;
SELECT *
FROM ${qdb}.${qschema}.${qtable}
FOR JSON PATH, INCLUDE_NULL_VALUES;
GO
SQL
    } | sqlcmd_raw 2>"$err" | tr -d '\r\n' > "$tmp"; then

        if python3 - "$tmp" "$rows_live" <<'PY' > "${tmp}.count" 2>>"$err"
import json, sys

path=sys.argv[1]
expected=int(sys.argv[2])

raw=open(path,"rb").read()
text=raw.decode("utf-8-sig").strip()
if not text:
    text="[]"

data=json.loads(text)
if not isinstance(data,list):
    raise SystemExit("root JSON não é array")

actual=len(data)
if actual != expected:
    raise SystemExit(f"row_count mismatch live={expected} json={actual}")

with open(path,"w",encoding="utf-8") as f:
    json.dump(data,f,ensure_ascii=False,separators=(",",":"))

print(actual)
PY
        then
            rows_json="$(cat "${tmp}.count")"
            rm -f "${tmp}.count" "$err"
            mv "$tmp" "$dest"
            bytes="$(stat -c '%s' "$dest")"

            echo "OK rows=$rows_json bytes=$bytes"

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

done < "$FAILED"

echo
echo "== 3. Montando dataset final =="

python3 - "$OUT" "$TABLES" "$V2" "$RECOVERY" "$FINAL" <<'PY'
import csv, json, os, shutil, sys

root,tables_path,v2_path,recovery_path,final_path=sys.argv[1:6]

tables=[]
with open(tables_path,newline="",encoding="utf-8",errors="replace") as f:
    for seq,row in enumerate(csv.reader(f,delimiter="|"),1):
        if len(row)<7: continue
        vals=[x.strip() for x in row[:7]]
        if not vals[0]: continue
        db,schema,table,qdb,qschema,qtable,rows=vals
        tables.append((seq,db,schema,table,int(rows)))

v2={}
with open(v2_path,newline="",encoding="utf-8",errors="replace") as f:
    r=csv.reader(f,delimiter="|")
    next(r,None)
    for row in r:
        if len(row)<8: continue
        key=tuple(x.strip() for x in row[:3])
        v2[key]={
            "rows": int(row[4]) if row[4].strip().isdigit() else 0,
            "rel": row[5].strip(),
            "status": row[7].strip(),
        }

rec={}
with open(recovery_path,newline="",encoding="utf-8",errors="replace") as f:
    r=csv.reader(f,delimiter="|")
    next(r,None)
    for row in r:
        if len(row)<9: continue
        key=tuple(x.strip() for x in row[:3])
        rec[key]={
            "rows": int(row[5]),
            "rel": row[6].strip(),
            "status": row[8].strip(),
        }

out=[]
total=0

for seq,db,schema,table,manifest_rows in tables:
    key=(db,schema,table)
    safe_db="".join(c if c.isalnum() or c in "._-" else "_" for c in db)
    safe_schema="".join(c if c.isalnum() or c in "._-" else "_" for c in schema)
    safe_table="".join(c if c.isalnum() or c in "._-" else "_" for c in table)

    final_rel=f"data_final/{safe_db}/{seq:04d}__{safe_schema}__{safe_table}.json"
    final_abs=os.path.join(root,final_rel)
    os.makedirs(os.path.dirname(final_abs),exist_ok=True)

    if key in rec:
        info=rec[key]
        source="V4"
    else:
        info=v2.get(key)
        source="V2"

    if not info or info["status"]!="OK":
        raise SystemExit(f"sem export válido para {key}")

    src_abs=os.path.join(root,info["rel"])
    if os.path.abspath(src_abs) != os.path.abspath(final_abs):
        shutil.copy2(src_abs,final_abs)

    with open(final_abs,encoding="utf-8") as f:
        data=json.load(f)

    if not isinstance(data,list):
        raise SystemExit(f"JSON root inválido: {key}")

    if len(data) != info["rows"]:
        raise SystemExit(f"count diverge: {key} expected={info['rows']} got={len(data)}")

    size=os.path.getsize(final_abs)
    total += len(data)

    out.append((
        db,schema,table,manifest_rows,len(data),
        final_rel,size,source,"OK"
    ))

with open(final_path,"w",newline="",encoding="utf-8") as f:
    w=csv.writer(f,delimiter="|",lineterminator="\n")
    w.writerow([
        "database","schema","table","rows_manifest","rows_final",
        "json_file","bytes","source","status"
    ])
    w.writerows(out)

print(f"FINAL_TABLES={len(out)}")
print(f"FINAL_ROWS={total}")
PY

FINAL_TABLES="$(awk -F'|' 'NR>1 && $9=="OK"{n++} END{print n+0}' "$FINAL")"
FINAL_ROWS="$(awk -F'|' 'NR>1 && $9=="OK"{s+=$5} END{print s+0}' "$FINAL")"
REC_OK="$(awk -F'|' 'NR>1 && $9=="OK"{n++} END{print n+0}' "$RECOVERY")"
REC_BAD="$(awk -F'|' 'NR>1 && $9!="OK"{n++} END{print n+0}' "$RECOVERY")"
MANIFEST_ROWS="$(awk -F'|' 'NF>=7 {gsub(/[[:space:]]/,"",$7); if($7~/^[0-9]+$/)s+=$7} END{print s+0}' "$TABLES")"

echo
echo "== 4. Validação global =="
echo "Tabelas finais      : $FINAL_TABLES"
echo "Recuperadas V4      : $REC_OK"
echo "Falhas V4           : $REC_BAD"
echo "Linhas manifest     : $MANIFEST_ROWS"
echo "Linhas finais       : $FINAL_ROWS"

if [[ "$FAIL" -ne 0 || "$REC_BAD" -ne 0 || "$FINAL_TABLES" -ne 329 ]]; then
    echo
    echo "===================================================================="
    echo "EXPORT V4: PARCIAL"
    echo "===================================================================="
    echo "Erros: $ERRORS"
    exit 4
fi

if [[ "$FINAL_ROWS" -ne "$MANIFEST_ROWS" ]]; then
    echo "[AVISO] row count mudou desde o inventário inicial."
    echo "Manifest=$MANIFEST_ROWS Final=$FINAL_ROWS"
fi

cat > "$OUT/README_FINAL.txt" <<EOF
Summit SQL Server export FINAL
==============================

Finalizado em: $(date --iso-8601=seconds)
Container: $CTR_NAME

SQL Server foi acessado somente para leitura.

Dataset canônico:
- exports_final.tsv
- data_final/<database>/*.json

Metadata:
- databases.tsv
- tables.tsv
- columns.tsv
- indexes.tsv

As 318 tabelas válidas da V2 foram reaproveitadas.
As 11 tabelas restantes foram recuperadas pela V4,
removendo apenas quebras físicas inseridas pelo sqlcmd na saída FOR JSON.
EOF

rm -f "$ARCHIVE"
tar -czf "$ARCHIVE" -C "$BASE" "$(basename "$OUT")"

echo
echo "===================================================================="
echo "EXPORT V4: PASS"
echo "===================================================================="
echo "Dataset : $OUT/data_final"
echo "Manifest: $FINAL"
echo "Archive : $ARCHIVE"
sha256sum "$ARCHIVE"
