#!/usr/bin/env sh
set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
CANONICAL_SCRIPT="$SCRIPT_DIR/tools/content_port/consolidate_canonical_content.py"

cd "$SCRIPT_DIR"

CLEAN=1 bash cliente.sh
CLEAN=1 bash server.sh

if [ ! -f "$CANONICAL_SCRIPT" ]; then
    echo "Erro: consolidador canonical nao encontrado: $CANONICAL_SCRIPT" >&2
    exit 1
fi

printf '\n====================================\n'
printf 'CANONICAL -> PACKS -> STANDALONE\n'
printf '====================================\n'
python3 "$CANONICAL_SCRIPT" --repo-root "$REPO_ROOT"

echo
echo "BUILD + CANONICAL + DEPLOY concluido."
echo "Canonical source   : $SCRIPT_DIR/Content/canonical"
echo "Cliente standalone : $REPO_ROOT/cliente"
echo "Servidor standalone: $REPO_ROOT/Servidor"
