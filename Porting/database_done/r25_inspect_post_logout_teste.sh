#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="/home/gabriel/Área de Trabalho/summit"
SOURCE="$ROOT/SourceCode"
RUNTIME="$ROOT/Servidor_R25_Wine"

ACCOUNT="${ACCOUNT:-teste}"
CHARACTER="${CHARACTER:-teste}"

DB="$RUNTIME/Database/summit.sqlite3"
ADMIN="$SOURCE/tools/sqlite_local_admin.py"
SERVER="$RUNTIME/Server.exe"
OUT="/tmp/r25_post_logout_${ACCOUNT}.txt"

die() { echo "[ERRO] $*" >&2; exit 1; }

{
echo "============================================================"
echo "R25 - INSPEÇÃO PÓS-LOGOUT / RELOGIN"
echo "============================================================"
echo "DATE=$(date --iso-8601=seconds)"
echo "ACCOUNT=$ACCOUNT"
echo "CHARACTER=$CHARACTER"
echo

[[ -f "$DB" ]] || die "SQLite não encontrado: $DB"

echo "== 1. PROCESSO / LISTENER =="
pgrep -af '[S]erver\.exe' || echo "SERVER_PROCESS=NOT_FOUND"
if ss -ltn 2>/dev/null | grep -qE '[:.]1997[[:space:]]'; then
    echo "LISTENER_1997=YES"
else
    echo "LISTENER_1997=NO"
fi
echo

echo "== 2. SERVER.EXE ATIVO =="
if [[ -f "$SERVER" ]]; then
    file "$SERVER"
    sha256sum "$SERVER"
    echo "-- probes R25_RECORD presentes --"
    for needle in \
        'r25_record_persistence_diag.log' \
        '[R25_RECORD][FILTER]' \
        '[R25_RECORD][HANDLER]' \
        '[R25_RECORD][RECV]'
    do
        if strings -a "$SERVER" | grep -Fq "$needle"; then
            echo "[YES] $needle"
        else
            echo "[NO]  $needle"
        fi
    done
else
    echo "SERVER_EXE=NOT_FOUND"
fi
echo

echo "== 3. SQLITE LOCAL ADMIN =="
if [[ -f "$ADMIN" ]]; then
    echo "-- status --"
    python3 "$ADMIN" --db "$DB" status || true
    echo
    echo "-- show account --"
    python3 "$ADMIN" --db "$DB" show "$ACCOUNT" || true
else
    echo "ADMIN_TOOL=NOT_FOUND"
fi
echo

echo "== 4. SQLITE: ROWS RELACIONADAS À CONTA =="
python3 - "$DB" "$ACCOUNT" <<'PY'
import sqlite3, sys

db, account = sys.argv[1], sys.argv[2]
con = sqlite3.connect(db)
con.row_factory = sqlite3.Row
cur = con.cursor()

tables = [r[0] for r in cur.execute(
    "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name"
)]

interesting = []
for t in tables:
    tl = t.lower()
    if any(k in tl for k in ("account", "auth", "session", "gameuser", "login")):
        interesting.append(t)

print(f"interesting_tables={len(interesting)}")

for t in interesting:
    try:
        cols = [r[1] for r in cur.execute(f'PRAGMA table_info("{t}")')]
    except Exception:
        continue

    # Só consulta tabelas pequenas/relevantes procurando a conta em colunas textuais.
    wheres = []
    params = []
    for c in cols:
        wheres.append(f'CAST("{c}" AS TEXT) = ?')
        params.append(account)

    if not wheres:
        continue

    sql = f'SELECT * FROM "{t}" WHERE ' + " OR ".join(wheres) + " LIMIT 20"
    try:
        rows = list(cur.execute(sql, params))
    except Exception:
        continue

    if rows:
        print(f"\n[{t}] rows={len(rows)}")
        for row in rows:
            print(dict(row))

con.close()
PY
echo

echo "== 5. FILES DATASERVER =="
mapfile -t MATCHES < <(
    find "$RUNTIME/DataServer" -type f \
      \( -iname "${ACCOUNT}.dat" -o -iname "${CHARACTER}.dat" \) \
      -print 2>/dev/null | sort -u
)

if (( ${#MATCHES[@]} == 0 )); then
    echo "NO_MATCHING_DAT_FILES"
else
    for f in "${MATCHES[@]}"; do
        echo
        echo "--- $f ---"
        stat -c 'SIZE=%s MTIME=%y' "$f"
        sha256sum "$f"

        python3 - "$f" <<'PY'
import sys
p = sys.argv[1]
data = open(p, 'rb').read()
nz = sum(1 for b in data if b != 0)
print(f"NONZERO_BYTES={nz}")
print(f"ZERO_BYTES={len(data)-nz}")
print(f"ALL_ZERO={'YES' if nz == 0 else 'NO'}")
PY

        echo "-- strings --"
        strings -a -n 3 "$f" | head -n 120 || true
        echo "-- first 256 bytes --"
        xxd -l 256 "$f" || true
    done
fi
echo

echo "== 6. LOGS RECENTES RELACIONADOS =="
PATTERN="${ACCOUNT}|${CHARACTER}|R25_RECORD|RECORDDATA|SAVECLIENT|FAILRECORD|LOGOUT|Logout|logout|session|SESSION|inuse|INUSE"

mapfile -t LOGS < <(
    find "$RUNTIME" -maxdepth 2 -type f \
      \( -iname '*.log' -o -iname '*.txt' \) \
      -mmin -240 -print 2>/dev/null | sort -u
)

if (( ${#LOGS[@]} == 0 )); then
    echo "NO_RECENT_LOGS"
else
    for f in "${LOGS[@]}"; do
        if grep -Eaiq "$PATTERN" "$f" 2>/dev/null; then
            echo
            echo "--- $f ---"
            grep -Eai "$PATTERN" "$f" 2>/dev/null | tail -n 250 || true
        fi
    done
fi
echo

echo "== 7. CLASSIFICAÇÃO RÁPIDA =="
python3 - "$DB" "$ACCOUNT" <<'PY'
import sqlite3, sys

db, account = sys.argv[1], sys.argv[2]
con = sqlite3.connect(db)
cur = con.cursor()

try:
    row = cur.execute(
        'SELECT * FROM "AccountDB__TGameUser" WHERE lower(UserID)=lower(?) LIMIT 1',
        (account,)
    ).fetchone()
    cols = [x[1] for x in cur.execute('PRAGMA table_info("AccountDB__TGameUser")')]
except Exception as e:
    print(f"ACCOUNT_QUERY_ERROR={e}")
    sys.exit(0)

if not row:
    print("ACCOUNT_ROW=NOT_FOUND")
else:
    d = dict(zip(cols, row))
    for key in ("UserID", "InUse", "Server", "Guid", "GUID"):
        if key in d:
            print(f"{key}={d[key]!r}")

    inuse = None
    for key in ("InUse", "inuse", "INUSE"):
        if key in d:
            inuse = d[key]
            break

    if str(inuse) not in ("0", "None", ""):
        print("LIKELY_RELOGIN_BLOCKER=ACCOUNT_STILL_INUSE")
    else:
        print("LIKELY_RELOGIN_BLOCKER=NOT_INUSE_FLAG")

con.close()
PY

echo
echo "============================================================"
echo "INSPEÇÃO CONCLUÍDA"
echo "============================================================"
echo "OUTPUT=$OUT"
} | tee "$OUT"
