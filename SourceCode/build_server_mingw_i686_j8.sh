#!/usr/bin/env bash
set -u
set -o pipefail

################################################################################
# CONFIGURAÇÃO
################################################################################

ROOT="$(pwd)"
BUILD_DIR="$ROOT/build_mingw_server_i686"
OBJ_DIR="$BUILD_DIR/objects"
ERR_DIR="$BUILD_DIR/errors"
DEP_DIR="$BUILD_DIR/deps"

OBJ_LIST="$ROOT/object_files_server_mingw.txt"
CPP_LIST="$ROOT/cpp_files_server_i686.txt"
LOG="$ROOT/build_server_i686_compile_errors.log"
SIGNATURE_FILE="$BUILD_DIR/compile_signature.sha256"

CXX="${CXX:-i686-w64-mingw32-g++}"
JOBS="${JOBS:-8}"
CLEAN="${CLEAN:-0}"

if command -v ccache >/dev/null 2>&1; then
    USE_CCACHE=1
else
    USE_CCACHE=0
fi

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
    -std=gnu++14
    -fpermissive
    -w

    -DWIN32
    -D_WINDOWS
    -D_M_IX86
    -Di386
    -D__i386__
    -D_SERVER
    -I.
    -ISrcServer/src
    -ISrcServer/src/sinbaram
    -ISrcServer/src/Skills
    -ISrcServer/src/HoBaram/NewEffect
    -ISrcServer/src/HoBaram/NewEffect/Script
    -ISrcServer/src/HoBaram/NewEffect/Script/lua

    -ISrcGame/src

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

if [ ! -d "$ROOT/SrcServer/src" ]; then
    echo "Erro: diretório SrcServer/src não encontrado"
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

: > "$LOG"
: > "$OBJ_LIST"

################################################################################
# LISTA CPP SERVIDOR
################################################################################

find SrcServer/src modules \
    -type f \
    \( \
        -iname "*.cpp" \
        -o -iname "*.CPP" \
        -o -iname "*.c" \
    \) \
    | grep -Eiv \
      'AntiCheat|Lua\.bak|\.bak|bak_|corrupt|/lua/lua\.c|/luac/|ltests\.c' \
    | sort \
    > "$CPP_LIST"

cat >> "$CPP_LIST" <<'LISTEOF'
SrcServer/src/Winmain.cpp
SrcServer/src/TJBOY/Ygy/Packet/Packet.cpp
SrcServer/src/TJBOY/Ygy/Process/Process.cpp
SrcServer/src/TJBOY/Ygy/Util.cpp
SrcServer/src/TJBOY/Ygy/memmap/memmap.cpp
SrcServer/src/sinbaram/sinInvenTory.cpp
SrcServer/src/sinbaram/sinSkill.cpp
SrcServer/src/sinbaram/sinSkill_Info.cpp
SrcServer/src/sinbaram/SkillFunction/Morayion.cpp
SrcServer/src/sinbaram/SkillFunction/Tempskron.cpp
LISTEOF

sort -u "$CPP_LIST" -o "$CPP_LIST"
TOTAL="$(wc -l < "$CPP_LIST")"

echo
echo "===================================="
echo "BUILD SERVIDOR MINGW i686"
echo "===================================="
echo "Fontes : $TOTAL"
echo "JOBS   : $JOBS"
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
# EXPORTA FLAGS PARA GNU PARALLEL
################################################################################

CXXFLAGS_SERIALIZED="$(declare -p CXXFLAGS)"

export ROOT BUILD_DIR OBJ_DIR ERR_DIR DEP_DIR LOG
export CXX CXXFLAGS_SERIALIZED USE_CCACHE USE_MAKE_DEPS

################################################################################
# COMPILA UM CPP
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

    return 0
}

export -f compile_one

################################################################################
# PARALELO
################################################################################

parallel \
    --jobs "$JOBS" \
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
    echo "Erros:"
    grep -Ei "error:|fatal error:|undefined|not declared|no matching|cannot convert" "$LOG" | tail -200 || true
    exit 1
fi

echo
echo "COMPILAÇÃO DO SERVIDOR CONCLUÍDA"
exit 0
