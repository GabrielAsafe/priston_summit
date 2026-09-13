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
command -v python3 >/dev/null 2>&1 || die "python3 não encontrado"

# Nunca importar enquanto o servidor está usando o banco.
if pgrep -af '[S]erver\.exe' >/dev/null 2>&1; then
    echo "[ERRO] Server.exe está em execução."
    echo "Pare o servidor antes da importação."
    pgrep -af '[S]erver\.exe' || true
    exit 2
fi

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
[[ -f "$EXP/exports_final.tsv" ]] || die "exports_final.tsv ausente: $EXP"
[[ -f "$EXP/columns.tsv" ]] || die "columns.tsv ausente: $EXP"
[[ -d "$EXP/data_final" ]] || die "data_final ausente: $EXP"

STAMP="$(date +%Y%m%d_%H%M%S)"
BACKUP="$RUNTIME/Database/summit.before_full_legacy_import_${STAMP}.sqlite3"
STAGE="$BASE/summit_r25_import_stage_${STAMP}.sqlite3"
REPORT="$BASE/r25_full_legacy_import_${STAMP}.txt"

echo "===================================================================="
echo "R25 - IMPORT COMPLETO SQL SERVER -> SQLITE"
echo "===================================================================="
echo
echo "Export : $EXP"
echo "SQLite : $DB"
echo "Backup : $BACKUP"
echo "Stage  : $STAGE"
echo "Report : $REPORT"
echo
echo "Server.exe: parado"
echo

python3 - "$EXP" "$DB" "$BACKUP" "$STAGE" "$REPORT" <<'PY'
import base64
import csv
import datetime
import decimal
import hashlib
import json
import os
import sqlite3
import sys
import traceback
from collections import defaultdict

EXP, DB, BACKUP, STAGE, REPORT = sys.argv[1:6]

def qident(s):
    return '"' + str(s).replace('"', '""') + '"'

def cf(s):
    return str(s).casefold()

def connect_ro(path):
    return sqlite3.connect("file:" + path + "?mode=ro", uri=True)

def db_backup(src_path, dst_path):
    if os.path.exists(dst_path):
        os.remove(dst_path)
    src = connect_ro(src_path)
    dst = sqlite3.connect(dst_path)
    try:
        src.backup(dst)
        dst.commit()
    finally:
        dst.close()
        src.close()

def overwrite_by_backup(src_path, dst_path):
    src = connect_ro(src_path)
    dst = sqlite3.connect(dst_path)
    try:
        src.backup(dst)
        dst.commit()
    finally:
        dst.close()
        src.close()

def sqlite_tables(conn):
    return [
        r[0] for r in conn.execute(
            "SELECT name FROM sqlite_master "
            "WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY name"
        )
    ]

def table_cols(conn, table):
    return [dict(r) for r in conn.execute(f"PRAGMA table_info({qident(table)})")]

def unique_keys(conn, table):
    cols = table_cols(conn, table)
    pk = sorted(
        [(int(r["pk"]), r["name"]) for r in cols if int(r["pk"]) > 0],
        key=lambda x: x[0]
    )
    out = []
    if pk:
        out.append([x[1] for x in pk])

    for idx in conn.execute(f"PRAGMA index_list({qident(table)})"):
        if int(idx["unique"]) != 1:
            continue
        names = [
            x["name"] for x in conn.execute(
                f"PRAGMA index_info({qident(idx['name'])})"
            ) if x["name"] is not None
        ]
        if names and names not in out:
            out.append(names)
    return out

def type_affinity(sqlserver_type):
    t = sqlserver_type.casefold()
    if t in {
        "tinyint","smallint","int","bigint","bit",
        "timestamp","rowversion"
    }:
        return "INTEGER" if t not in {"timestamp","rowversion"} else "BLOB"
    if t in {
        "decimal","numeric","money","smallmoney","float","real"
    }:
        return "NUMERIC"
    if t in {"binary","varbinary","image"}:
        return "BLOB"
    return "TEXT"

def decode_source_value(value, source_type):
    if value is None:
        return None

    t = source_type.casefold()

    if isinstance(value, decimal.Decimal):
        return format(value, "f")

    if t in {"binary","varbinary","image","timestamp","rowversion"}:
        if isinstance(value, str):
            try:
                return base64.b64decode(value, validate=True)
            except Exception:
                # V3/V4 may have emitted an explicit hex representation.
                if value.startswith("0x"):
                    try:
                        return bytes.fromhex(value[2:])
                    except Exception:
                        return value
        return value

    if isinstance(value, bool):
        return int(value)

    return value

def normalize_for_hash(v):
    if isinstance(v, bytes):
        return {"__blob__": base64.b64encode(v).decode("ascii")}
    if isinstance(v, decimal.Decimal):
        return {"__decimal__": str(v)}
    return v

def snapshot_table(conn, table):
    cols = [r["name"] for r in table_cols(conn, table)]
    rows = []
    for row in conn.execute(f"SELECT * FROM {qident(table)}"):
        rows.append([
            normalize_for_hash(v)
            for v in tuple(row)
        ])
    rows.sort(key=lambda r: json.dumps(r, ensure_ascii=False, sort_keys=True, default=str))
    blob = json.dumps(
        {"cols": cols, "rows": rows},
        ensure_ascii=False, sort_keys=True, separators=(",",":"), default=str
    ).encode("utf-8")
    return hashlib.sha256(blob).hexdigest(), len(rows)

def row_exists_exact(conn, table, target_cols, values):
    if not target_cols:
        return False
    where = []
    params = []
    for c, v in zip(target_cols, values):
        if v is None:
            where.append(f"{qident(c)} IS NULL")
        else:
            where.append(f"{qident(c)} IS ?")
            params.append(v)
    sql = f"SELECT 1 FROM {qident(table)} WHERE " + " AND ".join(where) + " LIMIT 1"
    return conn.execute(sql, params).fetchone() is not None

def row_exists_key(conn, table, key_cols, value_by_target):
    where = []
    params = []
    for c in key_cols:
        v = value_by_target[c]
        if v is None:
            where.append(f"{qident(c)} IS NULL")
        else:
            where.append(f"{qident(c)} IS ?")
            params.append(v)
    sql = f"SELECT 1 FROM {qident(table)} WHERE " + " AND ".join(where) + " LIMIT 1"
    return conn.execute(sql, params).fetchone() is not None

def logical_user_key(target_cols):
    by_cf = {cf(c):c for c in target_cols}
    for wanted in ("userid","user_id","accountid","account_id","account"):
        if wanted in by_cf:
            return by_cf[wanted]
    return None

def gm_business_key(target_cols):
    by_cf = {cf(c):c for c in target_cols}
    for wanted in (
        "userid","user_id","accountid","account_id","account",
        "username","user_name","login","name"
    ):
        if wanted in by_cf:
            return by_cf[wanted]
    return None

def source_json(path):
    with open(path, encoding="utf-8") as f:
        return json.load(f, parse_float=decimal.Decimal)

# ------------------------------------------------------------------
# Load source manifest + SQL Server metadata
# ------------------------------------------------------------------
manifest = []
with open(os.path.join(EXP, "exports_final.tsv"), newline="", encoding="utf-8") as f:
    r = csv.DictReader(f, delimiter="|")
    for row in r:
        row = {k:(v.strip() if isinstance(v,str) else v) for k,v in row.items()}
        if row["status"] != "OK":
            raise RuntimeError(f"manifest inválido: {row}")
        row["rows_final"] = int(row["rows_final"])
        manifest.append(row)

src_meta = defaultdict(list)
with open(os.path.join(EXP, "columns.tsv"), newline="", encoding="utf-8", errors="replace") as f:
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
        src_meta[(db,schema,table)].append({
            "ordinal": ordinal,
            "name": row[4].strip(),
            "type": row[5].strip(),
            "nullable": row[9].strip(),
            "identity": row[10].strip(),
            "computed": row[11].strip(),
        })

for k in src_meta:
    src_meta[k].sort(key=lambda x:x["ordinal"])

# ------------------------------------------------------------------
# Baseline and backup via SQLite backup API.
# ------------------------------------------------------------------
orig = connect_ro(DB)
orig.row_factory = sqlite3.Row
try:
    pre_quick = orig.execute("PRAGMA quick_check").fetchone()[0]
    pre_version = orig.execute("PRAGMA user_version").fetchone()[0]
    pre_tables = sqlite_tables(orig)
    pre_fk = list(orig.execute("PRAGMA foreign_key_check"))
finally:
    orig.close()

if pre_quick != "ok":
    raise RuntimeError(f"SQLite original quick_check != ok: {pre_quick}")

db_backup(DB, BACKUP)
db_backup(DB, STAGE)

con = sqlite3.connect(STAGE)
con.row_factory = sqlite3.Row
con.execute("PRAGMA busy_timeout=10000")
con.execute("PRAGMA foreign_keys=OFF")
con.execute("PRAGMA journal_mode=WAL")

# ------------------------------------------------------------------
# Case-insensitive target map and local-only protection snapshot.
# ------------------------------------------------------------------
target_names = sqlite_tables(con)
target_by_cf = {cf(t):t for t in target_names}

legacy_targets_cf = set()
for m in manifest:
    legacy_targets_cf.add(cf(f"{m['database']}__{m['table']}"))

protected_tables = sorted(
    t for t in target_names
    if cf(t) not in legacy_targets_cf
)

protected_before = {
    t: snapshot_table(con, t)
    for t in protected_tables
}

# Also preserve the local test account by logical UserID wherever present.
local_account = "teste"
local_account_snapshots = {}

for t in target_names:
    if not cf(t).startswith("accountdb__"):
        continue
    cols = [r["name"] for r in table_cols(con, t)]
    usercol = logical_user_key(cols)
    if not usercol:
        continue
    rows = [
        tuple(r) for r in con.execute(
            f"SELECT * FROM {qident(t)} "
            f"WHERE lower(CAST({qident(usercol)} AS TEXT))=lower(?)",
            (local_account,)
        )
    ]
    if rows:
        local_account_snapshots[t] = rows

# ------------------------------------------------------------------
# Import.
# ------------------------------------------------------------------
stats = defaultdict(int)
table_report = []
created_tables = []
gm_remaps = []
preserved_conflicts = []

def create_missing_table(db, schema, table, target):
    meta = src_meta[(db,schema,table)]
    if not meta:
        raise RuntimeError(f"sem metadata para criar {db}.{schema}.{table}")

    defs = []
    for c in meta:
        # Computed columns are exported as values; make them ordinary SQLite columns.
        coltype = type_affinity(c["type"])
        defs.append(f"{qident(c['name'])} {coltype}")

    sql = f"CREATE TABLE {qident(target)} (" + ", ".join(defs) + ")"
    con.execute(sql)
    created_tables.append(target)
    target_by_cf[cf(target)] = target

def mapped_columns(db, schema, table, target):
    src = src_meta[(db,schema,table)]
    tcols = table_cols(con, target)
    t_by_cf = {cf(r["name"]):r["name"] for r in tcols}

    mapping = []
    ignored = []
    missing = []

    for c in src:
        sname = c["name"]
        stype = c["type"]

        # Known R25 compatibility mappings.
        wanted = sname
        if (db, table, cf(sname)) == ("QuestDB", "Progress", "started"):
            wanted = "StartDate"

        if (db, table, cf(sname)) == ("ServerDB", "Vip", "id"):
            ignored.append(sname)
            continue

        actual = t_by_cf.get(cf(wanted))
        if actual is None:
            missing.append(sname)
        else:
            mapping.append((sname, actual, stype))

    return mapping, ignored, missing, tcols

con.execute("BEGIN IMMEDIATE")

try:
    for m in manifest:
        db = m["database"]
        schema = m["schema"]
        table = m["table"]
        source_count = int(m["rows_final"])
        src_key = (db,schema,table)
        expected_target = f"{db}__{table}"

        if source_count == 0:
            stats["source_empty"] += 1
            continue

        target = target_by_cf.get(cf(expected_target))

        if target is None:
            # Only non-empty missing legacy tables are materialized.
            create_missing_table(db, schema, table, expected_target)
            target = expected_target
            stats["created_missing_nonempty"] += 1

        mapping, ignored, missing, tcols_meta = mapped_columns(
            db, schema, table, target
        )

        if missing:
            raise RuntimeError(
                f"schema incompatível {db}.{schema}.{table} -> {target}; "
                f"source columns sem destino={missing}"
            )

        rows = source_json(os.path.join(EXP, m["json_file"]))
        if len(rows) != source_count:
            raise RuntimeError(
                f"JSON count diverge {db}.{schema}.{table}: "
                f"manifest={source_count} json={len(rows)}"
            )

        target_cols = [x[1] for x in mapping]
        source_cols = [x[0] for x in mapping]
        source_types = {x[0]:x[2] for x in mapping}
        target_col_cf = {cf(c):c for c in target_cols}

        keys = unique_keys(con, target)
        usable_key = []
        for keyset in keys:
            if all(cf(k) in target_col_cf for k in keyset):
                usable_key = [target_col_cf[cf(k)] for k in keyset]
                break

        # Account tables in the legacy layout often have no formal PK.
        account_user_col = None
        if cf(target).startswith("accountdb__") and cf(target) != cf("AccountDB__GameMasters"):
            account_user_col = logical_user_key(target_cols)

        gm_mode = cf(target) == cf("AccountDB__GameMasters")
        gm_user_col = gm_business_key(target_cols) if gm_mode else None
        gm_id_col = None
        if gm_mode:
            for c in target_cols:
                if cf(c) == "id":
                    gm_id_col = c
                    break

        inserted = 0
        skipped_exact = 0
        preserved = 0
        remapped = 0

        for source_row in rows:
            source_lc = {cf(k):v for k,v in source_row.items()}

            value_by_target = {}
            insert_cols = []
            insert_vals = []

            for sname, tname, stype in mapping:
                raw = source_row.get(sname)
                val = decode_source_value(raw, stype)
                value_by_target[tname] = val
                insert_cols.append(tname)
                insert_vals.append(val)

            # 1) Preserve current local account row by userid.
            if account_user_col and account_user_col in value_by_target:
                uv = value_by_target[account_user_col]
                if uv is not None:
                    exists = con.execute(
                        f"SELECT 1 FROM {qident(target)} "
                        f"WHERE lower(CAST({qident(account_user_col)} AS TEXT))=lower(?) "
                        f"LIMIT 1",
                        (str(uv),)
                    ).fetchone()
                    if exists:
                        # If it is byte-for-byte/projected equal, exact duplicate.
                        if row_exists_exact(con, target, insert_cols, insert_vals):
                            skipped_exact += 1
                            stats["skipped_exact"] += 1
                        else:
                            preserved += 1
                            stats["preserved_account_identity"] += 1
                            preserved_conflicts.append(
                                f"{target}: userid={uv!r} preserved local row"
                            )
                        continue

            # 2) Special collision-safe merge for GameMasters.
            if gm_mode:
                if gm_user_col and gm_user_col in value_by_target:
                    gv = value_by_target[gm_user_col]
                    if gv is not None:
                        exists = con.execute(
                            f"SELECT 1 FROM {qident(target)} "
                            f"WHERE lower(CAST({qident(gm_user_col)} AS TEXT))=lower(?) LIMIT 1",
                            (str(gv),)
                        ).fetchone()
                        if exists:
                            if row_exists_exact(con, target, insert_cols, insert_vals):
                                skipped_exact += 1
                                stats["skipped_exact"] += 1
                            else:
                                preserved += 1
                                stats["preserved_gm_identity"] += 1
                                preserved_conflicts.append(
                                    f"{target}: {gm_user_col}={gv!r} preserved local GM"
                                )
                            continue

                if gm_id_col and gm_id_col in value_by_target:
                    old_id = value_by_target[gm_id_col]
                    if old_id is not None:
                        exists = con.execute(
                            f"SELECT 1 FROM {qident(target)} "
                            f"WHERE {qident(gm_id_col)} IS ? LIMIT 1",
                            (old_id,)
                        ).fetchone()
                        if exists:
                            if row_exists_exact(con, target, insert_cols, insert_vals):
                                skipped_exact += 1
                                stats["skipped_exact"] += 1
                                continue

                            max_id = con.execute(
                                f"SELECT COALESCE(MAX(CAST({qident(gm_id_col)} AS INTEGER)),0) "
                                f"FROM {qident(target)}"
                            ).fetchone()[0]
                            new_id = int(max_id) + 1
                            idx = insert_cols.index(gm_id_col)
                            insert_vals[idx] = new_id
                            value_by_target[gm_id_col] = new_id
                            gm_remaps.append(
                                f"{target}: id {old_id!r} -> {new_id!r}"
                            )
                            remapped += 1
                            stats["gm_id_remapped"] += 1

            # 3) Exact duplicate check.
            if row_exists_exact(con, target, insert_cols, insert_vals):
                skipped_exact += 1
                stats["skipped_exact"] += 1
                continue

            # 4) Generic unique-key collision: preserve existing local row.
            if usable_key:
                if row_exists_key(con, target, usable_key, value_by_target):
                    preserved += 1
                    stats["preserved_unique_collision"] += 1
                    preserved_conflicts.append(
                        f"{target}: unique key {usable_key} preserved local row"
                    )
                    continue

            placeholders = ",".join("?" for _ in insert_cols)
            sql = (
                f"INSERT INTO {qident(target)} "
                f"({','.join(qident(c) for c in insert_cols)}) "
                f"VALUES ({placeholders})"
            )
            con.execute(sql, insert_vals)
            inserted += 1
            stats["inserted"] += 1

        processed = inserted + skipped_exact + preserved + remapped
        if processed != source_count:
            raise RuntimeError(
                f"process count diverge {db}.{schema}.{table}: "
                f"source={source_count} processed={processed}"
            )

        table_report.append({
            "source": f"{db}.{schema}.{table}",
            "target": target,
            "source_rows": source_count,
            "inserted": inserted,
            "skipped_exact": skipped_exact,
            "preserved": preserved,
            "gm_remapped": remapped,
            "ignored_columns": ",".join(ignored),
        })

    con.commit()

except Exception:
    con.rollback()
    raise

# ------------------------------------------------------------------
# Post-import validation on stage.
# ------------------------------------------------------------------
post_quick = con.execute("PRAGMA quick_check").fetchone()[0]
if post_quick != "ok":
    raise RuntimeError(f"stage quick_check != ok: {post_quick}")

post_fk = list(con.execute("PRAGMA foreign_key_check"))
if len(post_fk) > len(pre_fk):
    raise RuntimeError(
        f"novas violações FK: pre={len(pre_fk)} post={len(post_fk)}"
    )

# Protected R25-only tables must remain byte-for-byte equivalent.
for t, before in protected_before.items():
    after = snapshot_table(con, t)
    if after != before:
        raise RuntimeError(
            f"tabela local R25 foi alterada indevidamente: {t} "
            f"before={before} after={after}"
        )

# Local test account must remain present exactly as before wherever it existed.
for t, before_rows in local_account_snapshots.items():
    cols = [r["name"] for r in table_cols(con, t)]
    usercol = logical_user_key(cols)
    after_rows = [
        tuple(r) for r in con.execute(
            f"SELECT * FROM {qident(t)} "
            f"WHERE lower(CAST({qident(usercol)} AS TEXT))=lower(?)",
            (local_account,)
        )
    ]
    if after_rows != before_rows:
        raise RuntimeError(
            f"conta local {local_account!r} alterada em {t}"
        )

# Functional minimums: Docker rows must have been integrated.
critical = {
    "CoinShopDB__ItemList": 110,
    "CoinShopDB__ItemList_T": 34,
    "CoinShopDB__UserInfo": 34,
    "CoinShopDB__UserInfo_T": 34,
    "QuestDB__Complete": 195,
    "QuestDB__ItemRewards": 20,
    "QuestDB__MonsterKilled": 14,
    "QuestDB__Progress": 20,
    "QuestDB__QuestMonsters": 21,
    "QuestDB__Rewards": 21,
    "QuestDB__SingleQuest": 5,
    "BattlePass__BP_Info": 21,
    "BattlePass__BP_Item": 40,
    "SecurityDB__WindowCheatList": 108,
    "ServerDB__Vip": 16,
}

stage_names = {cf(t):t for t in sqlite_tables(con)}
critical_counts = {}

for expected, minimum in critical.items():
    actual_name = stage_names.get(cf(expected))
    if actual_name is None:
        raise RuntimeError(f"tabela crítica ausente após import: {expected}")
    count = con.execute(
        f"SELECT COUNT(*) FROM {qident(actual_name)}"
    ).fetchone()[0]
    critical_counts[actual_name] = count
    if count < minimum:
        raise RuntimeError(
            f"tabela crítica incompleta {actual_name}: "
            f"count={count} mínimo={minimum}"
        )

# Explicitly confirm quest IDs needed by /skiptier exist in Complete metadata path.
sq_name = stage_names.get(cf("QuestDB__SingleQuest"))
if sq_name:
    qids = {
        int(r[0]) for r in con.execute(
            f"SELECT QuestID FROM {qident(sq_name)} "
            f"WHERE QuestID IN (10010,10011,10012,10013)"
        )
    }
else:
    qids = set()

# Check GameMasters final count and local auth state.
gm_name = stage_names.get(cf("AccountDB__GameMasters"))
gm_count = (
    con.execute(f"SELECT COUNT(*) FROM {qident(gm_name)}").fetchone()[0]
    if gm_name else 0
)

local_auth_count = 0
if "localauth__accountpolicy" in stage_names:
    la = stage_names["localauth__accountpolicy"]
    local_auth_count = con.execute(
        f"SELECT COUNT(*) FROM {qident(la)}"
    ).fetchone()[0]

# Checkpoint before copying staged DB back.
try:
    con.execute("PRAGMA wal_checkpoint(TRUNCATE)")
except Exception:
    pass

final_version = con.execute("PRAGMA user_version").fetchone()[0]
final_tables = len(sqlite_tables(con))
con.close()

# ------------------------------------------------------------------
# Apply validated stage to runtime DB using SQLite backup API.
# Auto-restore if final verification fails.
# ------------------------------------------------------------------
applied = False

try:
    overwrite_by_backup(STAGE, DB)
    applied = True

    final = sqlite3.connect(DB)
    final.row_factory = sqlite3.Row
    try:
        final.execute("PRAGMA journal_mode=WAL")
        final_quick = final.execute("PRAGMA quick_check").fetchone()[0]
        final_fk = list(final.execute("PRAGMA foreign_key_check"))
        final_ver = final.execute("PRAGMA user_version").fetchone()[0]
        if final_quick != "ok":
            raise RuntimeError(f"runtime pós-import quick_check={final_quick}")
        if len(final_fk) > len(pre_fk):
            raise RuntimeError(
                f"runtime pós-import ganhou violações FK: "
                f"pre={len(pre_fk)} post={len(final_fk)}"
            )
        if final_ver != pre_version:
            raise RuntimeError(
                f"user_version mudou: antes={pre_version} depois={final_ver}"
            )
        try:
            final.execute("PRAGMA wal_checkpoint(TRUNCATE)")
        except Exception:
            pass
    finally:
        final.close()

except Exception:
    if applied:
        try:
            overwrite_by_backup(BACKUP, DB)
        except Exception as restore_error:
            raise RuntimeError(
                f"falha pós-aplicação e RESTORE também falhou: {restore_error}"
            )
    raise

# ------------------------------------------------------------------
# Report
# ------------------------------------------------------------------
with open(REPORT, "w", encoding="utf-8") as out:
    p = lambda *a: print(*a, file=out)

    p("="*78)
    p("R25 - FULL LEGACY DATA IMPORT REPORT")
    p("="*78)
    p(f"source_export={EXP}")
    p(f"runtime_db={DB}")
    p(f"backup_db={BACKUP}")
    p(f"stage_db={STAGE}")
    p(f"user_version_before={pre_version}")
    p(f"user_version_after={final_version}")
    p(f"quick_check_before={pre_quick}")
    p(f"quick_check_stage={post_quick}")
    p(f"fk_violations_before={len(pre_fk)}")
    p(f"fk_violations_stage={len(post_fk)}")
    p(f"sqlite_tables_before={len(pre_tables)}")
    p(f"sqlite_tables_after={final_tables}")
    p()

    p("== SUMMARY ==")
    for k in sorted(stats):
        p(f"{k}={stats[k]}")
    p(f"created_tables={len(created_tables)}")
    p(f"protected_r25_tables={len(protected_tables)}")
    p(f"local_auth_account_policies={local_auth_count}")
    p(f"game_masters_rows={gm_count}")
    p()

    p("== CREATED TABLES ==")
    if created_tables:
        for x in created_tables:
            p(x)
    else:
        p("NONE")
    p()

    p("== GM ID REMAPS ==")
    if gm_remaps:
        for x in gm_remaps:
            p(x)
    else:
        p("NONE")
    p()

    p("== PRESERVED LOCAL COLLISIONS ==")
    if preserved_conflicts:
        for x in preserved_conflicts:
            p(x)
    else:
        p("NONE")
    p()

    p("== TABLE IMPORTS ==")
    for r in table_report:
        p(
            f"{r['source']} -> {r['target']} "
            f"source={r['source_rows']} inserted={r['inserted']} "
            f"exact={r['skipped_exact']} preserved={r['preserved']} "
            f"gm_remapped={r['gm_remapped']} "
            f"ignored=[{r['ignored_columns']}]"
        )
    p()

    p("== CRITICAL COUNTS ==")
    for t in sorted(critical_counts):
        p(f"{t}={critical_counts[t]}")
    p()

    p("== SKIPTIER QUEST DEFINITIONS ==")
    p("present=" + ",".join(str(x) for x in sorted(qids)))
    p()

    p("IMPORT_APPLIED=YES")
    p("BACKUP_API=YES")
    p("LOCAL_R25_TABLES_PRESERVED=YES")
    p("LOCAL_TEST_ACCOUNT_PRESERVED=YES")
    p("QUICK_CHECK=ok")

print("IMPORT_APPLIED=YES")
print(f"BACKUP={BACKUP}")
print(f"STAGE={STAGE}")
print(f"REPORT={REPORT}")
print(f"USER_VERSION={final_version}")
print(f"TABLES_BEFORE={len(pre_tables)}")
print(f"TABLES_AFTER={final_tables}")
print(f"INSERTED={stats['inserted']}")
print(f"SKIPPED_EXACT={stats['skipped_exact']}")
print(f"PRESERVED_ACCOUNT_IDENTITY={stats['preserved_account_identity']}")
print(f"PRESERVED_GM_IDENTITY={stats['preserved_gm_identity']}")
print(f"PRESERVED_UNIQUE_COLLISION={stats['preserved_unique_collision']}")
print(f"GM_ID_REMAPPED={stats['gm_id_remapped']}")
print(f"CREATED_MISSING_NONEMPTY={stats['created_missing_nonempty']}")
print("CRITICAL_COUNTS:")
for t in sorted(critical_counts):
    print(f"  {t}={critical_counts[t]}")
print("SKIPTIER_QUEST_IDS=" + ",".join(str(x) for x in sorted(qids)))
print("QUICK_CHECK=ok")
PY

echo
echo "===================================================================="
echo "IMPORT COMPLETO: PASS"
echo "===================================================================="
echo
echo "Backup preservado:"
echo "  $BACKUP"
echo
echo "Relatório:"
echo "  $REPORT"
echo
echo "Agora o servidor pode ser iniciado novamente."
