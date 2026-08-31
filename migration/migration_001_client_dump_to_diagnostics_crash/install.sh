#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="${1:-}"
PATCH="$SCRIPT_DIR/change.patch"
EXPECTED_BASE="346137322c2635ce94b70aa849f23832511ba5ba"

usage() {
    echo "Uso: $0 /caminho/para/SourceCode"
    exit 2
}

[ -n "$SOURCE_DIR" ] || usage
SOURCE_DIR="$(realpath "$SOURCE_DIR")"

fail() {
    status=$?
    echo
    echo "MIGRATION 001: FALHOU (status $status)"
    echo "Nenhuma limpeza automática de outros trabalhos será feita."
    echo "Se o patch chegou a ser aplicado, rollback somente deste lote:"
    echo "  $SCRIPT_DIR/rollback.sh \"$SOURCE_DIR\""
    exit "$status"
}
trap fail ERR

echo "== Migration 001: client Dump -> diagnostics/crash =="
echo "Source : $SOURCE_DIR"
echo "Base   : $EXPECTED_BASE"

[ -d "$SOURCE_DIR" ] || { echo "ERRO: SourceCode não encontrado: $SOURCE_DIR"; exit 1; }

GIT_ROOT="$(git -C "$SOURCE_DIR" rev-parse --show-toplevel 2>/dev/null)" || {
    echo "ERRO: não encontrei um repositório Git em SourceCode nem nos diretórios pais."
    exit 1
}
GIT_ROOT="$(realpath "$GIT_ROOT")"
case "$SOURCE_DIR/" in
    "$GIT_ROOT/"*) ;;
    *) echo "ERRO: SourceCode não pertence ao repositório Git detectado: $GIT_ROOT"; exit 1 ;;
esac
SOURCE_REL="${SOURCE_DIR#"$GIT_ROOT"/}"
[ "$SOURCE_REL" != "$SOURCE_DIR" ] || SOURCE_REL="."

echo "Git    : $GIT_ROOT"
echo "Subdir : $SOURCE_REL"

ACTUAL_BASE="$(git -C "$GIT_ROOT" rev-parse HEAD)"
if [ "$ACTUAL_BASE" != "$EXPECTED_BASE" ]; then
    echo "ERRO: commit-base inesperado."
    echo "Esperado: $EXPECTED_BASE"
    echo "Atual   : $ACTUAL_BASE"
    exit 1
fi

# A working tree pode estar suja por bugfixes. O que NÃO pode existir é alteração
# já staged nos ficheiros deste lote, porque stage.sh precisa preservar a separação.
TOUCHED=(
    "$SOURCE_REL/SrcGame/src/Winmain.cpp"
    "$SOURCE_REL/SrcGame/src/priston.vcxproj"
    "$SOURCE_REL/SrcGame/src/Dump/WinDump.cpp"
    "$SOURCE_REL/SrcGame/src/Dump/WinDump.h"
    "$SOURCE_REL/SrcGame/src/diagnostics/crash/WinDump.cpp"
    "$SOURCE_REL/SrcGame/src/diagnostics/crash/WinDump.h"
)
if ! git -C "$GIT_ROOT" diff --cached --quiet -- "${TOUCHED[@]}"; then
    echo "ERRO: há alterações já staged em ficheiros tocados pela Migration 001."
    echo "Desfaça apenas o stage desses ficheiros antes de continuar; não descarte a working tree."
    git -C "$GIT_ROOT" diff --cached --name-status -- "${TOUCHED[@]}"
    exit 1
fi

(cd "$SCRIPT_DIR" && sha256sum -c change.patch.sha256)

check_before() {
    local rel="$1" expected="$2" path="$SOURCE_DIR/$rel" actual
    [ -f "$path" ] || { echo "ERRO: BEFORE ausente: $rel"; return 1; }
    actual="$(sha256sum "$path" | awk '{print $1}')"
    if [ "$actual" != "$expected" ]; then
        echo "ERRO: conteúdo atual diverge do BEFORE usado para preparar a migração:"
        echo "  $rel"
        echo "Esperado: $expected"
        echo "Atual   : $actual"
        echo "A migração não será aplicada por cima de conteúdo desconhecido."
        return 1
    fi
}

# Valida EXCLUSIVAMENTE os quatro ficheiros deste lote. Outros ficheiros podem
# estar modificados e são intencionalmente ignorados.
check_before "SrcGame/src/Dump/WinDump.cpp" "ee733699395a06c24127b21696d2c264cda94174f09309b4259151177a8804aa"
check_before "SrcGame/src/Dump/WinDump.h"   "08f658860f2b8cfa6ca99c54bba18e9e01d9f2dc32600d837673eb50fd6df344"
check_before "SrcGame/src/Winmain.cpp"       "9e20e10736c7b1df0781f69a5a62341f62be9e7477fb2f269339b33e1fbf03e1"
check_before "SrcGame/src/priston.vcxproj"   "d97772075c722b03505a92a12727ec67130dcb85b99a70a4db87c81e6422bdec"

# Os destinos não devem existir antes desta migração.
[ ! -e "$SOURCE_DIR/SrcGame/src/diagnostics/crash/WinDump.cpp" ] || { echo "ERRO: destino WinDump.cpp já existe."; exit 1; }
[ ! -e "$SOURCE_DIR/SrcGame/src/diagnostics/crash/WinDump.h" ] || { echo "ERRO: destino WinDump.h já existe."; exit 1; }

echo "Preflight: ficheiros deste lote correspondem exatamente ao BEFORE."
echo "Working tree externa ao lote: permitida."

git -C "$GIT_ROOT" apply --check --directory="$SOURCE_REL" "$PATCH"
git -C "$GIT_ROOT" apply --directory="$SOURCE_REL" "$PATCH"

# diff --check limitado ao patch: o repositório pode conter whitespace legado em
# outros trabalhos em andamento.
git -C "$GIT_ROOT" diff --check -- \
    "$SOURCE_REL/SrcGame/src/Winmain.cpp" \
    "$SOURCE_REL/SrcGame/src/priston.vcxproj" \
    "$SOURCE_REL/SrcGame/src/Dump/WinDump.cpp" \
    "$SOURCE_REL/SrcGame/src/Dump/WinDump.h" \
    "$SOURCE_REL/SrcGame/src/diagnostics/crash/WinDump.cpp" \
    "$SOURCE_REL/SrcGame/src/diagnostics/crash/WinDump.h"

# Auditoria estrutural.
test ! -e "$SOURCE_DIR/SrcGame/src/Dump/WinDump.cpp"
test ! -e "$SOURCE_DIR/SrcGame/src/Dump/WinDump.h"
test -f "$SOURCE_DIR/SrcGame/src/diagnostics/crash/WinDump.cpp"
test -f "$SOURCE_DIR/SrcGame/src/diagnostics/crash/WinDump.h"

if grep -RInE --exclude-dir=.git --exclude='*.bak' --exclude='*.BAK' \
    'Dump[/\\]WinDump|Dump/WinDump|Dump\\WinDump' \
    "$SOURCE_DIR/SrcGame/src"; then
    echo "ERRO: ainda existem referências ativas ao caminho legado Dump/WinDump."
    exit 1
fi

echo
echo "== Estado dos ficheiros deste lote =="
git -C "$GIT_ROOT" status --short -- "${TOUCHED[@]}"

echo
echo "== Build + link cliente =="
(cd "$SOURCE_DIR" && sh cliente.sh)

echo
echo "== Build + link servidor =="
(cd "$SOURCE_DIR" && sh server.sh)

echo
echo "MIGRATION 001: BUILD/LINK CLIENTE + SERVIDOR OK"
echo "Próximo passo: teste de runtime. NÃO COMMITAR antes do runtime passar."
echo "Depois do teste:"
echo "  $SCRIPT_DIR/audit.sh \"$SOURCE_DIR\""
echo "Depois da aprovação, para stage isolado:"
echo "  $SCRIPT_DIR/stage.sh \"$SOURCE_DIR\""
trap - ERR
