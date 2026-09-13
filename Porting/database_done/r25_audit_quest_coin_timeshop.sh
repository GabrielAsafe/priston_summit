#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="/home/gabriel/Área de Trabalho/summit"
RUNTIME="$ROOT/Servidor_R25_Wine"
DB="$RUNTIME/Database/summit.sqlite3"

ACCOUNT="${ACCOUNT:-teste}"
CHARACTER="${CHARACTER:-pike}"
OUT="$HOME/Transferências/r25_audit_quest_coin_timeshop_$(date +%Y%m%d_%H%M%S).txt"

[[ -f "$DB" ]] || {
    echo "[ERRO] banco não encontrado: $DB" >&2
    exit 1
}

python3 - "$DB" "$ACCOUNT" "$CHARACTER" <<'PY' | tee "$OUT"
import sqlite3, sys

db, account, character = sys.argv[1:4]
uri = "file:" + db + "?mode=ro"
con = sqlite3.connect(uri, uri=True)
con.row_factory = sqlite3.Row
cur = con.cursor()

def table_exists(name):
    return cur.execute(
        "SELECT 1 FROM sqlite_master WHERE type='table' AND name=?",
        (name,)
    ).fetchone() is not None

def count(name, where="", params=()):
    if not table_exists(name):
        return None
    q = f'SELECT COUNT(*) FROM "{name}"'
    if where:
        q += " WHERE " + where
    return cur.execute(q, params).fetchone()[0]

def print_table_count(name):
    c = count(name)
    if c is None:
        print(f"{name}: MISSING")
    else:
        print(f"{name}: rows={c}")

print("=" * 68)
print("R25 - AUDIT QUESTDB + COINSHOP + TIMESHOP (READ ONLY)")
print("=" * 68)
print(f"database={db}")
print(f"account={account}")
print(f"character={character}")
print(f"user_version={cur.execute('PRAGMA user_version').fetchone()[0]}")
print(f"quick_check={cur.execute('PRAGMA quick_check').fetchone()[0]}")
print()

print("== 1. QUESTDB TABLES ==")
quest_tables = [
    "QuestDB__SingleQuest",
    "QuestDB__Complete",
    "QuestDB__Progress",
    "QuestDB__MonsterKilled",
    "QuestDB__QuestMonsters",
    "QuestDB__Rewards",
    "QuestDB__ItemRewards",
]
for t in quest_tables:
    print_table_count(t)

print()
print("== 2. QUEST IDS 10010-10013 ==")
for qid in (10010, 10011, 10012, 10013):
    sq = count("QuestDB__SingleQuest", "QuestID=?", (qid,))
    comp = count("QuestDB__Complete", "QuestID=? AND PlayerName=?", (qid, character))
    prog = count("QuestDB__Progress", "QuestID=? AND PlayerName=?", (qid, character))
    mk = count("QuestDB__MonsterKilled", "QuestID=? AND PlayerName=?", (qid, character))
    print(f"Quest {qid}: SingleQuest={sq} Complete({character})={comp} "
          f"Progress={prog} MonsterKilled={mk}")

print()
print("== 3. EXACT READ QUERY USED BY /skiptier ==")
if table_exists("QuestDB__SingleQuest"):
    for qid in (10010, 10011, 10012, 10013):
        row = cur.execute(
            'SELECT COALESCE(MAX(QuestType),0), COUNT(*) '
            'FROM "QuestDB__SingleQuest" WHERE QuestID=?',
            (qid,)
        ).fetchone()
        print(f"{qid}: maxQuestType={row[0]} definitionCount={row[1]}")
else:
    print("SKIP: QuestDB__SingleQuest missing")

print()
print("== 4. QuestDB__Complete STRUCTURE ==")
if table_exists("QuestDB__Complete"):
    for row in cur.execute('PRAGMA table_info("QuestDB__Complete")'):
        print(dict(row))
    print("-- indexes --")
    for row in cur.execute('PRAGMA index_list("QuestDB__Complete")'):
        print(dict(row))

    print("-- compile INSERT OR REPLACE using EXPLAIN (no write) --")
    try:
        rows = cur.execute(
            'EXPLAIN INSERT OR REPLACE INTO "QuestDB__Complete" '
            '(QuestID,QuestKind,QuestType,PlayerName,EndDate) '
            "VALUES (?1,?2,?3,?4,datetime('now','localtime'))",
            (10010, 9, 0, character)
        ).fetchall()
        print(f"EXPLAIN_INSERT_OR_REPLACE=PASS opcodes={len(rows)}")
    except Exception as e:
        print(f"EXPLAIN_INSERT_OR_REPLACE=FAIL error={e!r}")
else:
    print("QuestDB__Complete missing")

print()
print("== 5. COINSHOP / TIMESHOP CATALOGS ==")
coin_tables = [
    "CoinShopDB__ItemList",
    "CoinShopDB__ItemList_T",
    "CoinShopDB__UserInfo",
    "CoinShopDB__UserInfo_T",
]
for t in coin_tables:
    print_table_count(t)

print()
print("== 6. ACCOUNT BALANCES ==")
if table_exists("CoinShopDB__UserInfo"):
    rows = cur.execute(
        'SELECT * FROM "CoinShopDB__UserInfo" WHERE lower(UserID)=lower(?)',
        (account,)
    ).fetchall()
    print("CoinShop UserInfo:", [dict(r) for r in rows] if rows else "NO ROW")
else:
    print("CoinShop UserInfo: TABLE MISSING")

if table_exists("CoinShopDB__UserInfo_T"):
    rows = cur.execute(
        'SELECT * FROM "CoinShopDB__UserInfo_T" WHERE lower(UserID)=lower(?)',
        (account,)
    ).fetchall()
    print("TimeShop UserInfo_T:", [dict(r) for r in rows] if rows else "NO ROW")
else:
    print("TimeShop UserInfo_T: TABLE MISSING")

print()
print("== 7. SAMPLE CATALOG ROWS ==")
for t in ("CoinShopDB__ItemList", "CoinShopDB__ItemList_T"):
    if table_exists(t):
        rows = cur.execute(f'SELECT * FROM "{t}" LIMIT 5').fetchall()
        print(f"-- {t} --")
        if not rows:
            print("EMPTY")
        else:
            for r in rows:
                print(dict(r))

print()
print("== 8. CLASSIFICATION ==")

issues = []

for t in ("QuestDB__SingleQuest", "QuestDB__Complete",
          "QuestDB__Progress", "QuestDB__MonsterKilled"):
    if not table_exists(t):
        issues.append(f"QUEST_TABLE_MISSING:{t}")

if table_exists("QuestDB__Complete"):
    idx = list(cur.execute('PRAGMA index_list("QuestDB__Complete")'))
    if not idx:
        issues.append("QUEST_COMPLETE_UNIQUE_INDEX_MISSING")

coin_item_count = count("CoinShopDB__ItemList")
time_item_count = count("CoinShopDB__ItemList_T")

if coin_item_count == 0:
    issues.append("COINSHOP_CATALOG_EMPTY")
if time_item_count == 0:
    issues.append("TIMESHOP_CATALOG_EMPTY")

if not table_exists("CoinShopDB__UserInfo"):
    issues.append("COINSHOP_USERINFO_MISSING")
elif count("CoinShopDB__UserInfo", "lower(UserID)=lower(?)", (account,)) == 0:
    issues.append("COINSHOP_ACCOUNT_BALANCE_ROW_MISSING")

if not table_exists("CoinShopDB__UserInfo_T"):
    issues.append("TIMESHOP_USERINFO_MISSING")
elif count("CoinShopDB__UserInfo_T", "lower(UserID)=lower(?)", (account,)) == 0:
    issues.append("TIMESHOP_ACCOUNT_BALANCE_ROW_MISSING")

if not issues:
    print("AUDIT_CLASS=SCHEMA_AND_DATA_PRESENT")
else:
    for x in issues:
        print(x)

print()
print("READ_ONLY=YES")
con.close()
PY

echo
echo "Relatório:"
echo "  $OUT"
