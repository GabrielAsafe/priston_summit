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

CPP_LIST="$ROOT/cpp_files_client_i686.txt"
OBJ_LIST="$ROOT/object_files_client_mingw.txt"
LOG="$ROOT/build_client_i686_compile_errors.log"

CXX="${CXX:-i686-w64-mingw32-g++}"

# Usa todos os processadores lógicos disponíveis.
# Pode ser sobrescrito: JOBS=16 ./build_client_mingw_i686_multicore.sh
if command -v nproc >/dev/null 2>&1; then
    DEFAULT_JOBS="$(nproc)"
elif command -v getconf >/dev/null 2>&1; then
    DEFAULT_JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 8)"
else
    DEFAULT_JOBS=8
fi

JOBS="${JOBS:-$DEFAULT_JOBS}"

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

    # Usa os headers DirectX compatíveis já separados para o cliente.
    -IDependency/DirectX9/Include
)

################################################################################
# VALIDAÇÕES
################################################################################

if ! command -v "$CXX" >/dev/null 2>&1; then
    echo "Erro: compilador não encontrado: $CXX"
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

rm -rf "$BUILD_DIR"
mkdir -p "$OBJ_DIR" "$ERR_DIR"

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
echo "===================================="
echo

################################################################################
# EXPORTAÇÃO PARA GNU PARALLEL
################################################################################

CXXFLAGS_SERIALIZED="$(declare -p CXXFLAGS)"

export ROOT
export BUILD_DIR
export OBJ_DIR
export ERR_DIR
export CXX
export CXXFLAGS_SERIALIZED

################################################################################
# COMPILAÇÃO UNITÁRIA
################################################################################

compile_one()
{
    eval "$CXXFLAGS_SERIALIZED"

    src="$1"

    rel="${src%.*}"
    obj="$OBJ_DIR/$rel.o"
    err="$ERR_DIR/$rel.err"

    mkdir -p "$(dirname "$obj")" "$(dirname "$err")"

    EXTRA_FLAGS=()

    case "$src" in
        *.c)
            # Mantém compatibilidade com fontes C legadas que dependem de C++.
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

    if [ "$status" -eq 0 ]; then
        rm -f "$err"
        printf 'OK   %s\n' "$src"
    else
        rm -f "$obj"
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
    --noswap \
    --line-buffer \
    --eta \
    --halt never \
    --env compile_one \
    --env ROOT \
    --env BUILD_DIR \
    --env OBJ_DIR \
    --env ERR_DIR \
    --env CXX \
    --env CXXFLAGS_SERIALIZED \
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

find "$OBJ_DIR" \
    -type f \
    -name "*.o" \
    | sort \
    > "$OBJ_LIST"

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
