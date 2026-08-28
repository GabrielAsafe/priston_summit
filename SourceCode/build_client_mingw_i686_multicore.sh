#!/usr/bin/env bash
set -u
set -o pipefail

################################################################################
# CONFIGURAÇÃO
################################################################################

ROOT="$(pwd)"
BUILD_DIR="$ROOT/build_mingw_client_i686"
OBJ_DIR="$BUILD_DIR/objects"
ERR_DIR="$BUILD_DIR/errors"
DEP_DIR="$BUILD_DIR/deps"

CPP_LIST="$ROOT/cpp_files_client_i686.txt"
OBJ_LIST="$ROOT/object_files_client_mingw.txt"
LOG="$ROOT/build_client_i686_compile_errors.log"
SIGNATURE_FILE="$BUILD_DIR/compile_signature.sha256"

CXX="${CXX:-i686-w64-mingw32-g++}"

# Usa todos os processadores lógicos disponíveis.
# Pode ser sobrescrito: JOBS=16 ./build_client_mingw_i686_multicore_cached.sh
if command -v nproc >/dev/null 2>&1; then
    DEFAULT_JOBS="$(nproc)"
elif command -v getconf >/dev/null 2>&1; then
    DEFAULT_JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 8)"
else
    DEFAULT_JOBS=8
fi

JOBS="${JOBS:-$DEFAULT_JOBS}"

# CLEAN=1 força uma compilação totalmente limpa.
CLEAN="${CLEAN:-0}"

# Usa ccache automaticamente se estiver disponível.
if command -v ccache >/dev/null 2>&1; then
    USE_CCACHE=1
else
    USE_CCACHE=0
fi

# O make é usado apenas para consultar os .d e saber se algum header mudou.
# Se não existir, o script continua correto, mas não faz SKIP incremental;
# nesse caso o ccache ainda pode acelerar recompilações.
if command -v make >/dev/null 2>&1; then
    USE_MAKE_DEPS=1
else
    USE_MAKE_DEPS=0
fi

################################################################################
# FLAGS
################################################################################

CXXFLAGS=(
    -m32
    -std=gnu++17
    -fpermissive

    -Wno-write-strings
    -Wno-narrowing
    -Wno-unused-variable
    -Wno-unused-parameter
    -Wno-deprecated-declarations

    -DWIN32
    -D_WINDOWS
    -D_M_IX86
    -Di386
    -D__i386__
    -DNDEBUG
    -D_CLIENT

    -I.
    -IShared
    -IShared2

    -ISrcGame/src
    -ISrcGame/src/smLib3d
    -ISrcGame/src/sinbaram
    -ISrcGame/src/Skills
    -ISrcGame/src/Language
    -ISrcGame/src/Resources
    -ISrcGame/src/Sound
    -ISrcGame/src/HoBaram
    -ISrcGame/src/HoBaram/NewEffect
    -ISrcGame/src/HoBaram/NewEffect/math
    -ISrcGame/src/HoBaram/NewEffect/Script
    -ISrcGame/src/HoBaram/NewEffect/Script/lua
    -ISrcGame/src/Library/LuaSrc
    -ISrcGame/src/Library/LuaSrc/Lua
    -ISrcGame/src/TJBOY/clanmenu
    -ISrcGame/src/TJBOY/isaocheck
    -ISrcGame/src/TJBOY/park
    -ISrcGame/src/TJBOY/Ygy
    -ISrcGame/src/TJBOY/Ygy/memmap
    -ISrcGame/src/Server
    -ISrcGame/src/WinInt
    -ISrcGame/src/Security
    -ISrcGame/src/Settings

    -Imodules
    -Imodules/File
    -Imodules/MD5
    -Imodules/Helper

    -IDependency/DirectX9/Include
)

################################################################################
# VALIDAÇÕES
################################################################################

if ! command -v "$CXX" >/dev/null 2>&1; then
    echo "Erro: compilador não encontrado: $CXX"
    exit 1
fi

if ! command -v sha256sum >/dev/null 2>&1; then
    echo "Erro: sha256sum não encontrado"
    exit 1
fi

if ! command -v parallel >/dev/null 2>&1; then
    echo "Erro: GNU parallel não encontrado"
    exit 1
fi

if [ ! -d "$ROOT/SrcGame/src" ]; then
    echo "Erro: diretório SrcGame/src não encontrado"
    exit 1
fi

if ! [[ "$JOBS" =~ ^[1-9][0-9]*$ ]]; then
    echo "Erro: JOBS deve ser um inteiro positivo"
    exit 1
fi

################################################################################
# PREPARAÇÃO
################################################################################

if [ "$CLEAN" = "1" ]; then
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$OBJ_DIR" "$ERR_DIR" "$DEP_DIR"

# O cache incremental por .d só conhece timestamps de source/headers.
# Esta assinatura também invalida objetos quando mudam o script, o compilador
# ou as flags de compilação, evitando SKIP de .o produzidos por outro comando.
SCRIPT_PATH="$(readlink -f "${BASH_SOURCE[0]}")"
SCRIPT_HASH="$(sha256sum "$SCRIPT_PATH" | awk '{print $1}')"
COMPILER_PATH="$(command -v "$CXX")"
COMPILER_VERSION="$("$CXX" --version 2>/dev/null | head -n 1)"
BUILD_SIGNATURE="$(
    {
        printf 'script=%s\n' "$SCRIPT_HASH"
        printf 'compiler=%s\n' "$COMPILER_PATH"
        printf 'version=%s\n' "$COMPILER_VERSION"
        printf 'flags='
        printf '%q ' "${CXXFLAGS[@]}"
        printf '\n'
    } | sha256sum | awk '{print $1}'
)"
PREVIOUS_SIGNATURE=""
if [ -f "$SIGNATURE_FILE" ]; then
    PREVIOUS_SIGNATURE="$(cat "$SIGNATURE_FILE")"
fi

if [ "$PREVIOUS_SIGNATURE" != "$BUILD_SIGNATURE" ]; then
    echo "CACHE  : configuração de compilação mudou; invalidando objetos"
    rm -rf "$OBJ_DIR" "$ERR_DIR" "$DEP_DIR"
    mkdir -p "$OBJ_DIR" "$ERR_DIR" "$DEP_DIR"
fi
printf '%s\n' "$BUILD_SIGNATURE" > "$SIGNATURE_FILE"

: > "$CPP_LIST"
: > "$OBJ_LIST"
: > "$LOG"

################################################################################
# LISTA DE FONTES
################################################################################

{
    find SrcGame/src -type f \
        \( -iname "*.cpp" -o -iname "*.cc" -o -iname "*.cxx" -o -iname "*.c" \)

    if [ -d modules/MD5 ]; then
        find modules/MD5 -type f \
            \( -iname "*.cpp" -o -iname "*.cc" -o -iname "*.cxx" -o -iname "*.c" \)
    fi
} \
| grep -Eiv \
    'GameGuard|XTrap|HackShield|Lua\.bak|\.bak|bak_|corrupt|/luac/|ltests\.c|/test/|/tests/' \
| grep -Eiv \
    'HoSkyOld\.cpp|HoOpeningOFF\.cpp|FilterEffect\.cpp|CChatBox\.cpp|/Library/LuaSrc/Lua/|/Server/(ClientFuncPos|OnSever|PartySever|Svr_Damge|svrEventSpawn)\.cpp' \
| sort -u \
> "$CPP_LIST"

TOTAL="$(wc -l < "$CPP_LIST")"

if [ "$TOTAL" -eq 0 ]; then
    echo "Erro: nenhum ficheiro fonte foi encontrado"
    exit 1
fi

echo
echo "===================================="
echo "BUILD CLIENTE MINGW i686"
echo "===================================="
echo "Fontes : $TOTAL"
echo "JOBS   : $JOBS"
echo "CPU(s) : $DEFAULT_JOBS"
echo "CXX    : $CXX"
if [ "$USE_CCACHE" = "1" ]; then
    echo "CCACHE : ativo"
else
    echo "CCACHE : não encontrado"
fi
if [ "$USE_MAKE_DEPS" = "1" ]; then
    echo "CACHE  : incremental + headers (.d)"
else
    echo "CACHE  : ccache/compilação normal (make ausente)"
fi
echo "CLEAN  : $CLEAN"
echo "===================================="
echo

################################################################################
# EXPORTAÇÃO PARA GNU PARALLEL
################################################################################

CXXFLAGS_SERIALIZED="$(declare -p CXXFLAGS)"

export ROOT BUILD_DIR OBJ_DIR ERR_DIR DEP_DIR
export CXX CXXFLAGS_SERIALIZED USE_CCACHE USE_MAKE_DEPS

################################################################################
# COMPILAÇÃO UNITÁRIA
################################################################################

compile_one()
{
    eval "$CXXFLAGS_SERIALIZED"

    src="$1"
    rel="${src%.*}"
    obj="$OBJ_DIR/$rel.o"
    dep="$DEP_DIR/$rel.d"
    err="$ERR_DIR/$rel.err"

    mkdir -p "$(dirname "$obj")" "$(dirname "$dep")" "$(dirname "$err")"

    # O arquivo .d conhece todos os headers usados por este objeto.
    # make -q retorna 0 quando o objeto está atualizado em relação a todas
    # as dependências; se algo mudou, seguimos para a recompilação.
    if [ "$USE_MAKE_DEPS" = "1" ] && [ -f "$obj" ] && [ -f "$dep" ]; then
        if printf '%%.o:\n\t@:\n' | make -rR -s -q -f "$dep" -f - "$obj" >/dev/null 2>&1; then
            rm -f "$err"
            printf 'SKIP %s\n' "$src"
            return 0
        fi
    fi

    EXTRA_FLAGS=()
    case "$src" in
        *.c)
            EXTRA_FLAGS+=(-x c++)
            ;;
    esac

    if [ "$USE_CCACHE" = "1" ]; then
        COMPILER=(ccache "$CXX")
    else
        COMPILER=("$CXX")
    fi

    "${COMPILER[@]}" \
        "${CXXFLAGS[@]}" \
        "${EXTRA_FLAGS[@]}" \
        -MMD -MP \
        -MF "$dep" \
        -MQ "$obj" \
        -c "$src" \
        -o "$obj" \
        >"$err" 2>&1

    status=$?

    if [ "$status" -eq 0 ]; then
        rm -f "$err"
        printf 'OK   %s\n' "$src"
    else
        rm -f "$obj" "$dep"
        printf 'FAIL %s\n' "$src"
    fi

    # Não deixa um erro individual interromper os restantes jobs.
    return 0
}

export -f compile_one

################################################################################
# COMPILAÇÃO MULTICORE
################################################################################

parallel \
    --jobs "$JOBS" \
    --load 100% \
    --line-buffer \
    --eta \
    --halt never \
    --env compile_one \
    --env ROOT \
    --env BUILD_DIR \
    --env OBJ_DIR \
    --env ERR_DIR \
    --env DEP_DIR \
    --env CXX \
    --env CXXFLAGS_SERIALIZED \
    --env USE_CCACHE \
    --env USE_MAKE_DEPS \
    compile_one \
    :::: "$CPP_LIST"

################################################################################
# CONSOLIDAÇÃO
################################################################################

find "$ERR_DIR" \
    -type f \
    -name "*.err" \
    -print0 \
| sort -z \
| while IFS= read -r -d '' err; do
    src="${err#"$ERR_DIR/"}"
    src="${src%.err}"

    {
        echo
        echo "===== FAIL: $src ====="
        cat "$err"
    } >> "$LOG"
done

# Gera a lista apenas com objetos correspondentes às fontes atuais.
while IFS= read -r src; do
    rel="${src%.*}"
    obj="$OBJ_DIR/$rel.o"
    if [ -f "$obj" ]; then
        printf '%s\n' "$obj"
    fi
done < "$CPP_LIST" | sort > "$OBJ_LIST"

OK="$(wc -l < "$OBJ_LIST")"
FAIL=$((TOTAL - OK))

################################################################################
# RESULTADO
################################################################################

echo
echo "===================================="
echo "TOTAL : $TOTAL"
echo "OK    : $OK"
echo "FAIL  : $FAIL"
echo "JOBS  : $JOBS"
echo "===================================="
echo
echo "Lista CPP : $CPP_LIST"
echo "Lista OBJ : $OBJ_LIST"
echo "Log       : $LOG"

if [ "$FAIL" -ne 0 ]; then
    echo
    echo "Primeiros erros relevantes:"
    grep -Ei \
        "error:|fatal error:|undefined|not declared|no matching|cannot convert" \
        "$LOG" \
        | head -200 \
        || true

    exit 1
fi

echo
echo "COMPILAÇÃO DO CLIENTE CONCLUÍDA"
exit 0
