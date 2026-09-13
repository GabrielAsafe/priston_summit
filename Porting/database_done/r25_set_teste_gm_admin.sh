#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="/home/gabriel/Área de Trabalho/summit"
SOURCE="$ROOT/SourceCode"
RUNTIME="$ROOT/Servidor_R25_Wine"

ADMIN="$SOURCE/tools/sqlite_local_admin.py"
DB="$RUNTIME/Database/summit.sqlite3"
ACCOUNT="${ACCOUNT:-teste}"

die() {
    echo "[ERRO] $*" >&2
    exit 1
}

echo "============================================================"
echo "R25 - CADASTRAR CONTA COMO ADMIN GM"
echo "============================================================"
echo
echo "Conta: $ACCOUNT"
echo "Banco: $DB"
echo

[[ -f "$ADMIN" ]] || die "sqlite_local_admin.py não encontrado: $ADMIN"
[[ -f "$DB" ]] || die "Banco SQLite não encontrado: $DB"

echo "== 1. Estado atual =="
python3 "$ADMIN" --db "$DB" show "$ACCOUNT"
echo

echo "== 2. Cadastrando security=4 (administrator) =="
echo "Digite a senha GM quando solicitado."
echo "Use a senha que o servidor deve validar para esta conta GM."
echo

python3 "$ADMIN" \
    --db "$DB" \
    set-gm "$ACCOUNT" \
    --security 4

echo
echo "== 3. Estado final =="
python3 "$ADMIN" --db "$DB" show "$ACCOUNT"

echo
echo "== 4. Status global =="
python3 "$ADMIN" --db "$DB" status

echo
echo "============================================================"
echo "GM ADMIN: PASS"
echo "============================================================"
echo
echo "Se o servidor já estiver rodando, faça logout/login da conta."
echo "Se o GameMasters for carregado apenas no startup desta build,"
echo "reinicie o servidor antes de testar os comandos GM."
