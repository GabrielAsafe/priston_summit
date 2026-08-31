#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="${1:-}"
PATCH="$SCRIPT_DIR/change.patch"
[ -n "$SOURCE_DIR" ] || { echo "Uso: $0 /caminho/para/SourceCode"; exit 2; }
SOURCE_DIR="$(realpath "$SOURCE_DIR")"
GIT_ROOT="$(git -C "$SOURCE_DIR" rev-parse --show-toplevel 2>/dev/null)" || { echo "ERRO: Git não encontrado."; exit 1; }
GIT_ROOT="$(realpath "$GIT_ROOT")"
SOURCE_REL="${SOURCE_DIR#"$GIT_ROOT"/}"; [ "$SOURCE_REL" != "$SOURCE_DIR" ] || SOURCE_REL="."

echo "== Rollback Migration 001 =="
# Se stage.sh já foi executado, remove apenas este patch do index primeiro.
if git -C "$GIT_ROOT" apply --cached -R --check --directory="$SOURCE_REL" "$PATCH" >/dev/null 2>&1; then
    git -C "$GIT_ROOT" apply --cached -R --directory="$SOURCE_REL" "$PATCH"
    echo "Stage da Migration 001 removido do index."
fi
# Reverte apenas o delta da migração na working tree; alterações alheias permanecem.
git -C "$GIT_ROOT" apply -R --check --directory="$SOURCE_REL" "$PATCH"
git -C "$GIT_ROOT" apply -R --directory="$SOURCE_REL" "$PATCH"
echo "Rollback do lote concluído. Outros trabalhos foram preservados."
