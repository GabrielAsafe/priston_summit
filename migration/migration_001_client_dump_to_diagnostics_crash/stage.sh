#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="${1:-}"
PATCH="$SCRIPT_DIR/change.patch"
EXPECTED_BASE="346137322c2635ce94b70aa849f23832511ba5ba"
[ -n "$SOURCE_DIR" ] || { echo "Uso: $0 /caminho/para/SourceCode"; exit 2; }
SOURCE_DIR="$(realpath "$SOURCE_DIR")"
GIT_ROOT="$(git -C "$SOURCE_DIR" rev-parse --show-toplevel 2>/dev/null)" || { echo "ERRO: Git não encontrado."; exit 1; }
GIT_ROOT="$(realpath "$GIT_ROOT")"
SOURCE_REL="${SOURCE_DIR#"$GIT_ROOT"/}"; [ "$SOURCE_REL" != "$SOURCE_DIR" ] || SOURCE_REL="."
[ "$(git -C "$GIT_ROOT" rev-parse HEAD)" = "$EXPECTED_BASE" ] || { echo "ERRO: HEAD mudou; não preparar stage."; exit 1; }

# Confirma que o AFTER está realmente aplicado antes de mexer no index.
"$SCRIPT_DIR/audit.sh" "$SOURCE_DIR"

TOUCHED=(
    "$SOURCE_REL/SrcGame/src/Winmain.cpp"
    "$SOURCE_REL/SrcGame/src/priston.vcxproj"
    "$SOURCE_REL/SrcGame/src/Dump/WinDump.cpp"
    "$SOURCE_REL/SrcGame/src/Dump/WinDump.h"
    "$SOURCE_REL/SrcGame/src/diagnostics/crash/WinDump.cpp"
    "$SOURCE_REL/SrcGame/src/diagnostics/crash/WinDump.h"
)

# Não mistura com conteúdo previamente staged nesses mesmos ficheiros.
if ! git -C "$GIT_ROOT" diff --cached --quiet -- "${TOUCHED[@]}"; then
    echo "ERRO: já existe conteúdo staged em ficheiros deste lote."
    git -C "$GIT_ROOT" diff --cached --name-status -- "${TOUCHED[@]}"
    exit 1
fi

# Aplica o patch SOMENTE ao index. A working tree já contém o AFTER + eventuais
# bugfixes anteriores; esses bugfixes continuam unstaged.
git -C "$GIT_ROOT" apply --cached --check --directory="$SOURCE_REL" "$PATCH"
git -C "$GIT_ROOT" apply --cached --directory="$SOURCE_REL" "$PATCH"

echo "== STAGE ISOLADO DA MIGRATION 001 =="
git -C "$GIT_ROOT" diff --cached --check
git -C "$GIT_ROOT" diff --cached --stat
echo
echo "Conteúdo staged:"
git -C "$GIT_ROOT" diff --cached --name-status

echo
echo "IMPORTANTE: alterações antigas continuam na working tree e NÃO foram staged."
echo "Revise com:"
echo "  git -C \"$GIT_ROOT\" diff --cached"
echo "  git -C \"$GIT_ROOT\" diff -- SourceCode/SrcGame/src/Winmain.cpp"
echo "Após revisar, commit sugerido:"
echo "  git -C \"$GIT_ROOT\" commit -m \"migration: move client crash diagnostics\""
