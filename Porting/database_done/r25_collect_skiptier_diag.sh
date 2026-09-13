#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="/home/gabriel/Área de Trabalho/summit"
RUNTIME="$ROOT/Servidor_R25_Wine"
DB="$RUNTIME/Database/summit.sqlite3"
LOG="$RUNTIME/LOGS/r25_skiptier_diag.log"

echo "===================================================================="
echo "R25 - COLETA SKIPTIER DIAG"
echo "===================================================================="

echo
echo "== LOG =="
if [[ -f "$LOG" ]]; then
    tail -n 120 "$LOG"
else
    echo "LOG_MISSING=$LOG"
fi

echo
echo "== QUESTDB ROWS pike / 10010-10013 =="

python3 - "$DB" <<'PY'
import sqlite3, sys
db=sys.argv[1]
con=sqlite3.connect("file:"+db+"?mode=ro", uri=True)
con.row_factory=sqlite3.Row

for table in ("QuestDB__SingleQuest","QuestDB__Progress","QuestDB__MonsterKilled","QuestDB__Complete"):
    print(f"-- {table} --")
    cols={r["name"] for r in con.execute(f'PRAGMA table_info("{table}")')}
    if "QuestID" not in cols:
        print("NO QuestID")
        continue

    where=['QuestID IN (10010,10011,10012,10013)']
    params=[]
    if "PlayerName" in cols:
        where.append("lower(PlayerName)=lower(?)")
        params.append("pike")

    rows=con.execute(
        f'SELECT * FROM "{table}" WHERE ' + " AND ".join(where) + " ORDER BY QuestID",
        params
    ).fetchall()

    if not rows:
        print("NO ROWS")
    else:
        for r in rows:
            print(dict(r))

print()
print("quick_check=", con.execute("PRAGMA quick_check").fetchone()[0])
con.close()
PY

echo
echo "===================================================================="
echo "COLETA CONCLUÍDA"
echo "===================================================================="
