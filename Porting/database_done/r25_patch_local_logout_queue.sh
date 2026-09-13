#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="/home/gabriel/Área de Trabalho/summit/SourceCode"
FILE="$ROOT/Shared/server/legacy_runtime/OnSever_legacy_server.inl"
STAMP="$(date +%Y%m%d_%H%M%S)"
BACKUP="${FILE}.before_r25_local_logout_fix_${STAMP}"

die() {
    echo "[ERRO] $*" >&2
    exit 1
}

[[ -f "$FILE" ]] || die "Arquivo não encontrado: $FILE"

echo "============================================================"
echo "R25 - PATCH LOCAL LOGOUT QUEUE"
echo "============================================================"
echo
echo "Arquivo:"
echo "  $FILE"
echo
echo "Objetivo:"
echo "  remover o gate legado szGPCode[0] que impede contas"
echo "  AuthMode=local / free de enfileirar bSql_Logout()."
echo
echo "Este script NÃO compila e NÃO inicia nada."
echo

if grep -q 'R25_LOCAL_LOGOUT_QUEUE_FIX' "$FILE"; then
    echo "[OK] Patch já aplicado."
    exit 0
fi

python3 - "$FILE" "$BACKUP" <<'PY'
from pathlib import Path
import shutil
import sys

path = Path(sys.argv[1])
backup = Path(sys.argv[2])

text = path.read_text(encoding="utf-8", errors="surrogateescape")

old = """\
\t\tif(lpPlayInfo->szGPCode[0])
\t\t{

\t\t\tbSql_Logout(lpPlayInfo->getAccount(), lpPlayInfo->getName(), lpPlayInfo->lpsmSock->szIPAddr, lpPlayInfo->szGPCode,
\t\t\t\tlpPlayInfo->DisuseDay, lpPlayInfo->UsePeriod, lpPlayInfo->szGuid, lpPlayInfo->Bl_RNo);

\t\t}
"""

new = """\
\t\t// R25_LOCAL_LOGOUT_QUEUE_FIX:
\t\t// SQLite local-session ownership must not depend on legacy billing GPCode.
\t\t// bSql_Logout() queues BILLING_SQL_LOGOUT; its worker calls
\t\t// TrySQLiteLocalLogout() first and only falls back to the legacy provider
\t\t// when SQLite does not own the account.
\t\tbSql_Logout(lpPlayInfo->getAccount(), lpPlayInfo->getName(), lpPlayInfo->lpsmSock->szIPAddr, lpPlayInfo->szGPCode,
\t\t\tlpPlayInfo->DisuseDay, lpPlayInfo->UsePeriod, lpPlayInfo->szGuid, lpPlayInfo->Bl_RNo);
"""

count = text.count(old)
if count != 1:
    print(f"[ERRO] Bloco esperado encontrado {count} vezes; esperado exatamente 1.", file=sys.stderr)
    sys.exit(2)

shutil.copy2(path, backup)
path.write_text(text.replace(old, new, 1), encoding="utf-8", errors="surrogateescape")
PY

echo "[OK] Backup:"
echo "  $BACKUP"
echo

grep -q 'R25_LOCAL_LOGOUT_QUEUE_FIX' "$FILE" \
    || die "marker não encontrado depois do patch"

if grep -A12 -B2 'R25_LOCAL_LOGOUT_QUEUE_FIX' "$FILE" | grep -q 'if(lpPlayInfo->szGPCode\[0\])'; then
    die "gate szGPCode ainda existe no bloco corrigido"
fi

echo "== Diff =="
diff -u "$BACKUP" "$FILE" || true

echo
echo "== Verificação do bloco final =="
grep -n -A10 -B5 'R25_LOCAL_LOGOUT_QUEUE_FIX' "$FILE"

echo
echo "============================================================"
echo "PATCH SOURCE: PASS"
echo "============================================================"
echo
echo "Nenhum build/link foi executado."
echo "Revise o diff acima antes de compilar com server.sh."
