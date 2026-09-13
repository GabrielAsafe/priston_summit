#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="/home/gabriel/Área de Trabalho/summit"
RUNTIME="$ROOT/Servidor_R25_Wine"
DB="$RUNTIME/Database/summit.sqlite3"
BASE="$HOME/Transferências"

die() {
    echo "[ERRO] $*" >&2
    exit 1
}

[[ -f "$DB" ]] || die "SQLite não encontrado: $DB"

if [[ -n "${EXPORT_DIR:-}" ]]; then
    EXP="$EXPORT_DIR"
else
    EXP="$(
        find "$BASE" -maxdepth 1 -type d \
            -name 'summit_sqlserver_full_export_*' \
            -printf '%T@|%p\n' 2>/dev/null |
        sort -nr |
        head -n1 |
        cut -d'|' -f2-
    )"
fi

[[ -n "${EXP:-}" ]] || die "export SQL Server não encontrado"
[[ -f "$EXP/exports_final.tsv" ]] || die "exports_final.tsv ausente em $EXP"
[[ -f "$EXP/columns.tsv" ]] || die "columns.tsv ausente em $EXP"
[[ -d "$EXP/data_final" ]] || die "data_final ausente em $EXP"

STAMP="$(date +%Y%m%d_%H%M%S)"
OUT="$BASE/r25_sqlserver_to_sqlite_plan_$STAMP.txt"
TSV="$BASE/r25_sqlserver_to_sqlite_plan_$STAMP.tsv"

echo "===================================================================="
echo "R25 - PLANO READ-ONLY SQL SERVER -> SQLITE"
echo "===================================================================="
echo "Export : $EXP"
echo "SQLite : $DB"
echo

python3 - "$EXP" "$DB" "$OUT" "$TSV" <<'PY'
import csv
import json
import os
import sqlite3
import sys
from collections import defaultdict

exp, db_path, out_path, tsv_path = sys.argv[1:5]

manifest_path = os.path.join(exp, "exports_final.tsv")
columns_path = os.path.join(exp, "columns.tsv")

# ----------------------------
# Load export manifest
# ----------------------------
manifest = []
with open(manifest_path, newline="", encoding="utf-8", errors="replace") as f:
    r = csv.DictReader(f, delimiter="|")
    for row in r:
        row = {k:(v.strip() if isinstance(v,str) else v) for k,v in row.items()}
        if row.get("status") != "OK":
            raise SystemExit(f"manifest contém status != OK: {row}")
        row["rows_final"] = int(row["rows_final"])
        manifest.append(row)

# ----------------------------
# Load SQL Server columns
# ----------------------------
src_cols = defaultdict(list)
with open(columns_path, newline="", encoding="utf-8", errors="replace") as f:
    r = csv.reader(f, delimiter="|")
    for row in r:
        if len(row) < 14:
            continue
        db, schema, table = row[0].strip(), row[1].strip(), row[2].strip()
        if not db:
            continue
        try:
            ordinal = int(row[3].strip())
        except Exception:
            continue
        src_cols[(db,schema,table)].append({
            "ordinal": ordinal,
            "name": row[4].strip(),
            "type": row[5].strip(),
            "nullable": row[9].strip(),
            "identity": row[10].strip(),
            "computed": row[11].strip(),
        })

for k in src_cols:
    src_cols[k].sort(key=lambda x:x["ordinal"])

# ----------------------------
# Open SQLite strictly read-only
# ----------------------------
uri = "file:" + db_path + "?mode=ro"
con = sqlite3.connect(uri, uri=True)
con.row_factory = sqlite3.Row
cur = con.cursor()

target_tables = [
    r[0] for r in cur.execute(
        "SELECT name FROM sqlite_master "
        "WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY name"
    )
]
target_set = set(target_tables)

def qident(s):
    return '"' + s.replace('"','""') + '"'

def target_info(table):
    cols = list(cur.execute(f'PRAGMA table_info({qident(table)})'))
    col_names = [r["name"] for r in cols]

    # Primary key columns in PK order.
    pk = sorted(
        [(int(r["pk"]), r["name"]) for r in cols if int(r["pk"]) > 0],
        key=lambda x:x[0]
    )
    pk_cols = [name for _,name in pk]

    unique_sets = []
    for idx in cur.execute(f'PRAGMA index_list({qident(table)})'):
        if int(idx["unique"]) != 1:
            continue
        idxname = idx["name"]
        cols_idx = [
            x["name"] for x in cur.execute(f'PRAGMA index_info({qident(idxname)})')
            if x["name"] is not None
        ]
        if cols_idx:
            unique_sets.append(cols_idx)

    if pk_cols:
        unique_sets.insert(0, pk_cols)

    count = cur.execute(f'SELECT COUNT(*) FROM {qident(table)}').fetchone()[0]
    return cols, col_names, unique_sets, count

target_meta = {t: target_info(t) for t in target_tables}

# ----------------------------
# Evaluate collisions
# ----------------------------
def load_rows(relpath):
    p = os.path.join(exp, relpath)
    with open(p, encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise RuntimeError(f"JSON root not list: {relpath}")
    return data

def choose_conflict_key(unique_sets, source_names):
    lower_src = {x.lower():x for x in source_names}
    for keyset in unique_sets:
        if all(k.lower() in lower_src for k in keyset):
            return keyset
    return []

def count_collisions(table, rows, keycols):
    if not rows or not keycols:
        return 0

    # Map source row keys case-insensitively.
    total = 0
    for row in rows:
        lrow = {str(k).lower(): v for k,v in row.items()}
        vals = []
        ok = True
        for c in keycols:
            if c.lower() not in lrow:
                ok = False
                break
            vals.append(lrow[c.lower()])
        if not ok:
            continue

        where = " AND ".join(f'{qident(c)} IS ?' for c in keycols)
        sql = f'SELECT 1 FROM {qident(table)} WHERE {where} LIMIT 1'
        if cur.execute(sql, vals).fetchone():
            total += 1
    return total

results = []
mapped_targets = set()
total_source_rows = 0

for m in manifest:
    db = m["database"]
    schema = m["schema"]
    table = m["table"]
    source_rows = int(m["rows_final"])
    total_source_rows += source_rows

    # R25 naming convention for dbo legacy DBs.
    target = f"{db}__{table}"
    mapped_targets.add(target)

    sc = [x["name"] for x in src_cols.get((db,schema,table), [])]
    sc_lower = {x.lower():x for x in sc}

    result = {
        "source": f"{db}.{schema}.{table}",
        "target": target,
        "source_rows": source_rows,
        "target_rows": "",
        "collision_key": "",
        "collisions": "",
        "missing_source_cols_in_target": "",
        "extra_target_cols": "",
        "classification": "",
    }

    if target not in target_set:
        result["classification"] = "BLOCK_TARGET_MISSING"
        results.append(result)
        continue

    cols, tc, unique_sets, target_rows = target_meta[target]
    result["target_rows"] = target_rows

    tc_lower = {x.lower():x for x in tc}
    missing = [c for c in sc if c.lower() not in tc_lower]
    extra = [c for c in tc if c.lower() not in sc_lower]

    result["missing_source_cols_in_target"] = ",".join(missing)
    result["extra_target_cols"] = ",".join(extra)

    if missing:
        result["classification"] = "BLOCK_SCHEMA_MISMATCH"
        results.append(result)
        continue

    if source_rows == 0:
        result["classification"] = "NOOP_SOURCE_EMPTY"
        results.append(result)
        continue

    rows = load_rows(m["json_file"])
    if len(rows) != source_rows:
        result["classification"] = "BLOCK_JSON_COUNT"
        results.append(result)
        continue

    keycols = choose_conflict_key(unique_sets, sc)
    result["collision_key"] = ",".join(keycols)

    collisions = count_collisions(target, rows, keycols) if keycols else 0
    result["collisions"] = collisions

    if target_rows == 0:
        result["classification"] = "SAFE_EMPTY_TARGET"
    elif not keycols:
        result["classification"] = "REVIEW_NONEMPTY_NO_KEY"
    elif collisions == 0:
        result["classification"] = "SAFE_APPEND_NO_COLLISION"
    else:
        result["classification"] = "REVIEW_COLLISIONS"

    results.append(result)

# Tables that exist only in SQLite or have no legacy source mapping.
local_only = sorted(target_set - mapped_targets)

# ----------------------------
# Summary
# ----------------------------
classes = defaultdict(int)
for r in results:
    classes[r["classification"]] += 1

source_nonempty = sum(1 for r in results if int(r["source_rows"]) > 0)
source_empty = len(results) - source_nonempty

with open(tsv_path, "w", newline="", encoding="utf-8") as f:
    fields = [
        "source","target","source_rows","target_rows","collision_key",
        "collisions","missing_source_cols_in_target","extra_target_cols",
        "classification"
    ]
    w = csv.DictWriter(f, fieldnames=fields, delimiter="|", lineterminator="\n")
    w.writeheader()
    w.writerows(results)

with open(out_path, "w", encoding="utf-8") as out:
    p = lambda *a, **k: print(*a, file=out, **k)

    p("="*78)
    p("R25 - SQL SERVER -> SQLITE COMPATIBILITY PLAN (READ ONLY)")
    p("="*78)
    p(f"export={exp}")
    p(f"sqlite={db_path}")
    p(f"sqlite_user_version={cur.execute('PRAGMA user_version').fetchone()[0]}")
    p(f"sqlite_quick_check={cur.execute('PRAGMA quick_check').fetchone()[0]}")
    p(f"source_tables={len(results)}")
    p(f"source_rows={total_source_rows}")
    p(f"sqlite_tables={len(target_tables)}")
    p(f"source_nonempty_tables={source_nonempty}")
    p(f"source_empty_tables={source_empty}")
    p()

    p("== CLASSIFICATION ==")
    for k in sorted(classes):
        p(f"{k}={classes[k]}")
    p()

    p("== BLOCKERS / REVIEW ==")
    found = False
    for r in results:
        if r["classification"].startswith("BLOCK_") or r["classification"].startswith("REVIEW_"):
            found = True
            p(
                f"{r['classification']}: {r['source']} -> {r['target']} "
                f"src={r['source_rows']} dst={r['target_rows']} "
                f"key=[{r['collision_key']}] collisions={r['collisions']} "
                f"missing=[{r['missing_source_cols_in_target']}] "
                f"extra=[{r['extra_target_cols']}]"
            )
    if not found:
        p("NONE")
    p()

    p("== NONEMPTY SOURCE TABLES ==")
    for r in results:
        if int(r["source_rows"]) > 0:
            p(
                f"{r['source']} -> {r['target']} "
                f"src={r['source_rows']} dst={r['target_rows']} "
                f"class={r['classification']} "
                f"key=[{r['collision_key']}] collisions={r['collisions']}"
            )
    p()

    p("== SQLITE-ONLY / R25 EXTRA TABLES ==")
    for t in local_only:
        count = target_meta[t][3]
        p(f"{t} rows={count}")
    p()

    # Highlight important functional areas.
    p("== IMPORTANT AREAS ==")
    needles = ("CoinShopDB__", "QuestDB__", "AccountDB__GameMasters",
               "BattlePass__", "ServerDB__", "ClanDB__")
    for r in results:
        if r["target"].startswith(needles):
            p(
                f"{r['target']} src={r['source_rows']} dst={r['target_rows']} "
                f"class={r['classification']} collisions={r['collisions']}"
            )
    p()
    p("READ_ONLY=YES")

con.close()

print(f"PLAN_REPORT={out_path}")
print(f"PLAN_TSV={tsv_path}")
print(f"SOURCE_TABLES={len(results)}")
print(f"SOURCE_ROWS={total_source_rows}")
print(f"SQLITE_TABLES={len(target_tables)}")
for k in sorted(classes):
    print(f"{k}={classes[k]}")
PY

echo
echo "===================================================================="
echo "PLANO READ-ONLY: CONCLUÍDO"
echo "===================================================================="
echo "Relatório: $OUT"
echo "Tabela   : $TSV"
echo
echo "Nenhuma escrita foi feita no SQLite."
