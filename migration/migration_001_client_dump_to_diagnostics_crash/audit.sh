#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="${1:-}"
EXPECTED_BASE="346137322c2635ce94b70aa849f23832511ba5ba"
[ -n "$SOURCE_DIR" ] || { echo "Uso: $0 /caminho/para/SourceCode"; exit 2; }
SOURCE_DIR="$(realpath "$SOURCE_DIR")"
GIT_ROOT="$(git -C "$SOURCE_DIR" rev-parse --show-toplevel 2>/dev/null)" || { echo "ERRO: Git não encontrado."; exit 1; }
GIT_ROOT="$(realpath "$GIT_ROOT")"
SOURCE_REL="${SOURCE_DIR#"$GIT_ROOT"/}"; [ "$SOURCE_REL" != "$SOURCE_DIR" ] || SOURCE_REL="."
BASE="$(git -C "$GIT_ROOT" rev-parse HEAD)"
[ "$BASE" = "$EXPECTED_BASE" ] || { echo "ERRO: HEAD mudou antes da aprovação. Esperado $EXPECTED_BASE, atual $BASE"; exit 1; }

check_after() {
    local rel="$1" expected="$2" path="$SOURCE_DIR/$rel" actual
    [ -f "$path" ] || { echo "ERRO: AFTER ausente: $rel"; return 1; }
    actual="$(sha256sum "$path" | awk '{print $1}')"
    [ "$actual" = "$expected" ] || { echo "ERRO: AFTER divergiu: $rel"; echo "Esperado: $expected"; echo "Atual   : $actual"; return 1; }
}

# Hashes do AFTER preparado.
check_after "SrcGame/src/diagnostics/crash/WinDump.cpp" "ee733699395a06c24127b21696d2c264cda94174f09309b4259151177a8804aa"
check_after "SrcGame/src/diagnostics/crash/WinDump.h"   "08f658860f2b8cfa6ca99c54bba18e9e01d9f2dc32600d837673eb50fd6df344"
check_after "SrcGame/src/Winmain.cpp"                   "$(sha256sum "$SCRIPT_DIR/after/SrcGame/src/Winmain.cpp" | awk '{print $1}')"
check_after "SrcGame/src/priston.vcxproj"               "$(sha256sum "$SCRIPT_DIR/after/SrcGame/src/priston.vcxproj" | awk '{print $1}')"

test ! -e "$SOURCE_DIR/SrcGame/src/Dump/WinDump.cpp"
test ! -e "$SOURCE_DIR/SrcGame/src/Dump/WinDump.h"
grep -q '#include "diagnostics/crash/WinDump.h"' "$SOURCE_DIR/SrcGame/src/Winmain.cpp"
if grep -RInE --exclude-dir=.git --exclude='*.bak' --exclude='*.BAK' 'Dump[/\\]WinDump|Dump/WinDump|Dump\\WinDump' "$SOURCE_DIR/SrcGame/src"; then
    echo "ERRO: referência ativa ao caminho legado encontrada."; exit 1
fi

git -C "$GIT_ROOT" diff --check -- \
    "$SOURCE_REL/SrcGame/src/Winmain.cpp" \
    "$SOURCE_REL/SrcGame/src/priston.vcxproj" \
    "$SOURCE_REL/SrcGame/src/Dump/WinDump.cpp" \
    "$SOURCE_REL/SrcGame/src/Dump/WinDump.h" \
    "$SOURCE_REL/SrcGame/src/diagnostics/crash/WinDump.cpp" \
    "$SOURCE_REL/SrcGame/src/diagnostics/crash/WinDump.h"

echo "== Migration 001 audit: OK =="
echo "Ficheiros do lote continuam exatamente no AFTER esperado."
echo "Outras modificações da working tree foram ignoradas por design."
echo "Se o runtime passou, execute stage.sh para preparar SOMENTE este lote."
