#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="/home/gabriel/Área de Trabalho/summit"
SOURCE="$ROOT/SourceCode"
RUNTIME="$ROOT/Servidor_R25_Wine"

ADMIN="$SOURCE/tools/sqlite_local_admin.py"
DB="$RUNTIME/Database/summit.sqlite3"

ACCOUNT="${ACCOUNT:-teste}"
PASSWORD="${PASSWORD:-123123}"
CHARACTER="${CHARACTER:-teste}"

STATE="/tmp/r25_fresh_account_test.env"

die() { echo "[ERRO] $*" >&2; exit 1; }
ok()  { echo "[OK] $*"; }

echo "============================================================"
echo "R25 - NOVA CONTA LIMPA PARA TESTE CONTROLADO"
echo "============================================================"
echo

[[ -f "$ADMIN" ]] || die "sqlite_local_admin.py não encontrado: $ADMIN"
[[ -f "$DB" ]] || die "Banco SQLite não encontrado: $DB"

if [[ -e "$RUNTIME/Database.dll" ]]; then
    die "Database.dll está ATIVA no runtime. Não criar teste nesse estado."
fi

[[ -f "$RUNTIME/Database.dll.R25_DISABLED" ]] \
    || die "Database.dll.R25_DISABLED ausente"

[[ -f "$RUNTIME/sqlite3.dll" ]] \
    || die "sqlite3.dll ausente"

echo "== 1. Política atual =="
python3 "$ADMIN" --db "$DB" status
echo

echo "== 2. Criando conta nova =="
python3 "$ADMIN" \
    --db "$DB" \
    create-account "$ACCOUNT" \
    --password "$PASSWORD" \
    --auth local

ok "conta criada"
echo

echo "== 3. Verificando conta =="
python3 "$ADMIN" --db "$DB" show "$ACCOUNT"
echo

echo "== 4. Verificando ausência de estado antigo em DataServer =="

mapfile -t stale < <(
    find "$RUNTIME/DataServer" -type f \
      \( -iname "${ACCOUNT}.dat" -o -iname "${CHARACTER}.dat" \) \
      -print 2>/dev/null || true
)

if (( ${#stale[@]} > 0 )); then
    echo "[ERRO] Foram encontrados artefatos inesperados:"
    printf '  %s\n' "${stale[@]}"
    exit 2
fi

ok "nenhum userinfo/userdata antigo para esta conta/personagem"

cat >"$STATE" <<EOF
ACCOUNT='$ACCOUNT'
CHARACTER='$CHARACTER'
EOF
chmod 600 "$STATE"

echo
echo "============================================================"
echo "CONTA DE TESTE CRIADA: PASS"
echo "============================================================"
echo
echo "Use no cliente:"
echo
echo "  LOGIN    : $ACCOUNT"
echo "  SENHA    : $PASSWORD"
echo
echo "Ao criar personagem, use EXATAMENTE:"
echo
echo "  PERSONAGEM: $CHARACTER"
echo
echo "Estado do teste salvo em:"
echo "  $STATE"
echo
echo "Não reutilize TestR25/teste123 neste teste."
