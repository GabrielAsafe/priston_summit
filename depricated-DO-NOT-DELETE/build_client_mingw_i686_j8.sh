#!/usr/bin/env bash
set -u

################################################################################
# CONFIGURAÇÃO
################################################################################

BUILD_DIR="build_mingw_client_i686"
OBJ_LIST="object_files_client_mingw.txt"
CPP_LIST="cpp_files_client_i686.txt"
LOG="build_client_i686_compile_errors.log"

CXX="${CXX:-i686-w64-mingw32-g++}"
JOBS="${JOBS:-8}"

################################################################################
# FLAGS
################################################################################

CXXFLAGS=(
    -m32
    -std=gnu++14
    -fpermissive
    -w

    -DWIN32
    -D_WINDOWS
    -D_M_IX86
    -Di386
    -D__i386__

    # Build do cliente
    -D_CLIENT

    -I.
    -ISrcGame/src
    -ISrcGame/src/sinbaram
    -ISrcGame/src/Skills
    -ISrcGame/src/HoBaram/NewEffect
    -ISrcGame/src/HoBaram/NewEffect/Script
    -ISrcGame/src/HoBaram/NewEffect/Script/lua

    -IShared
    -IShared/Lua
    -IShared2

    -Imodules
    -Imodules/File


    "-IDependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Include"
)

################################################################################
# VALIDAÇÕES
################################################################################

if ! command -v "$CXX" >/dev/null 2>&1
then
    echo "Erro: compilador não encontrado: $CXX"
    exit 1
fi

if ! command -v parallel >/dev/null 2>&1
then
    echo "Erro: GNU parallel não encontrado"
    exit 1
fi

if [ ! -d "SrcGame/src" ]
then
    echo "Erro: diretório SrcGame/src não encontrado"
    exit 1
fi

################################################################################
# PREPARAÇÃO
################################################################################

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR/errors"

: > "$LOG"
: > "$OBJ_LIST"
: > "$CPP_LIST"

################################################################################
# LISTA DE FONTES DO CLIENTE
################################################################################

SEARCH_DIRS=()

for dir in \
    SrcGame/src \
    modules \
    Shared \
    Shared2
do
    if [ -d "$dir" ]
    then
        SEARCH_DIRS+=("$dir")
    fi
done

find "${SEARCH_DIRS[@]}" \
    -type f \
    \( \
        -iname "*.cpp" \
        -o -iname "*.cc" \
        -o -iname "*.cxx" \
        -o -iname "*.c" \
    \) \
    | grep -Eiv \
      'AntiCheat|GameGuard|XTrap|HackShield|Lua\.bak|\.bak|bak_|corrupt|/luac/|ltests\.c|/test/|/tests/' \
    | sort -u \
    > "$CPP_LIST"

################################################################################
# LUA
#
# Evita compilar novamente o núcleo Lua quando ele já é fornecido por uma
# biblioteca separada, como build_mingw_lua_i686.
################################################################################

sed -i \
    -E '\#/lua/(lua|luac|lapi|lauxlib|lbaselib|lcode|ldebug|ldo|ldump|lfunc|lgc|linit|liolib|llex|lmathlib|lmem|loadlib|lobject|lopcodes|loslib|lparser|lstate|lstring|lstrlib|ltable|ltablib|ltm|lundump|lvm|lzio)\.c$#Id' \
    "$CPP_LIST"

sort -u "$CPP_LIST" -o "$CPP_LIST"

TOTAL=$(wc -l < "$CPP_LIST")

echo
echo "Diretórios pesquisados:"
printf '  %s\n' "${SEARCH_DIRS[@]}"

echo
echo "Total de fontes do cliente: $TOTAL"
echo "Compilador: $CXX"
echo "Compilando com JOBS=$JOBS"
echo

if [ "$TOTAL" -eq 0 ]
then
    echo "Erro: nenhum ficheiro fonte foi encontrado"
    exit 1
fi

################################################################################
# EXPORTA CONFIGURAÇÃO PARA GNU PARALLEL
################################################################################

CXXFLAGS_SERIALIZED=$(declare -p CXXFLAGS)

export BUILD_DIR
export CXX
export CXXFLAGS_SERIALIZED

################################################################################
# COMPILA UM FICHEIRO
################################################################################

compile_one()
{
    eval "$CXXFLAGS_SERIALIZED"

    src="$1"

    # Preserva a árvore de diretórios para impedir colisões de nomes.
    obj="$BUILD_DIR/objects/${src%.*}.o"
    err="$BUILD_DIR/errors/${src%.*}.err"

    mkdir -p "$(dirname "$obj")"
    mkdir -p "$(dirname "$err")"

    EXTRA_FLAGS=()

    case "$src" in
        *.c)
            # O projeto legado inclui ficheiros .c que podem depender do modo C++.
            EXTRA_FLAGS+=(-x c++)
            ;;
    esac

    "$CXX" \
        "${CXXFLAGS[@]}" \
        "${EXTRA_FLAGS[@]}" \
        -c "$src" \
        -o "$obj" \
        >"$err" 2>&1

    status=$?

    if [ "$status" -eq 0 ]
    then
        rm -f "$err"
        printf 'OK   %s\n' "$src"
    else
        rm -f "$obj"
        printf 'FAIL %s\n' "$src"
    fi

    return 0
}

export -f compile_one

################################################################################
# COMPILAÇÃO PARALELA
################################################################################

parallel \
    --env compile_one \
    --env BUILD_DIR \
    --env CXX \
    --env CXXFLAGS_SERIALIZED \
    --jobs "$JOBS" \
    --eta \
    --line-buffer \
    compile_one \
    :::: "$CPP_LIST"

################################################################################
# CONSOLIDA ERROS
################################################################################

find "$BUILD_DIR/errors" \
    -type f \
    -name "*.err" \
    -print0 \
    | sort -z \
    | while IFS= read -r -d '' err
do
    src="${err#"$BUILD_DIR/errors/"}"
    src="${src%.err}"

    {
        echo
        echo "===== FAIL: $src ====="
        cat "$err"
    } >> "$LOG"
done

################################################################################
# LISTA DE OBJETOS
################################################################################

find "$BUILD_DIR/objects" \
    -type f \
    -name "*.o" \
    | sort \
    > "$OBJ_LIST"

OK=$(wc -l < "$OBJ_LIST")
FAIL=$((TOTAL - OK))

################################################################################
# RESULTADO
################################################################################

echo
echo "===================================="
echo "TOTAL : $TOTAL"
echo "OK    : $OK"
echo "FAIL  : $FAIL"
echo "===================================="

echo
echo "Lista de fontes:"
echo "  $CPP_LIST"

echo
echo "Lista de objetos:"
echo "  $OBJ_LIST"

if [ "$FAIL" -ne 0 ]
then
    echo
    echo "Log completo:"
    echo "  $LOG"

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
