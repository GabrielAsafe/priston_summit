#!/usr/bin/env bash
set -u

################################################################################
# CONFIGURAÇÃO
################################################################################

BUILD_DIR="build_mingw_server_i686"
OBJ_LIST="object_files_server_mingw.txt"
CPP_LIST="cpp_files_server_i686.txt"
LOG="build_server_i686_compile_errors.log"

CXX=i686-w64-mingw32-g++
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

    # -include Shared/compat_sal.h

    "-IDependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Include"
)


################################################################################
# PREPARAÇÃO
################################################################################

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

: > "$LOG"


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


TOTAL=$(wc -l < "$CPP_LIST")


echo
echo "Total CPP servidor: $TOTAL"
echo "Compilando com JOBS=$JOBS"
echo


################################################################################
# EXPORTA FLAGS PARA GNU PARALLEL
################################################################################

CXXFLAGS_SERIALIZED=$(declare -p CXXFLAGS)

export BUILD_DIR
export LOG
export CXX
export CXXFLAGS_SERIALIZED


################################################################################
# COMPILA UM CPP
################################################################################

compile_one()
{
    eval "$CXXFLAGS_SERIALIZED"

    src="$1"

    obj="$BUILD_DIR/${src//\//_}"
    obj="${obj%.*}.o"

    err="$obj.err"

    EXTRA_FLAGS=()

#    case "$src" in
#        SrcServer/src/Sound/Dxwav.cpp)
#            EXTRA_FLAGS+=(
    # -include Shared/compat_sal.h
#            )
#            ;;
#    esac

    "$CXX" \
        "${CXXFLAGS[@]}" \
        "${EXTRA_FLAGS[@]}" \
        -c "$src" \
        -o "$obj" \
        >"$err" 2>&1

    if [ $? -eq 0 ]
    then
        rm -f "$err"
        echo "OK   $src"
    else
        echo "FAIL $src"

        {
            echo
            echo "===== FAIL: $src ====="
            cat "$err"
        } >> "$LOG"

        rm -f "$obj"
    fi
}
################################################################################
# PARALELO
################################################################################
export -f compile_one

if command -v parallel >/dev/null 2>&1
then

parallel \
    --env compile_one \
    --env BUILD_DIR \
    --env LOG \
    --env CXX \
    --env CXXFLAGS_SERIALIZED \
    -j"$JOBS" \
    --eta \
    compile_one \
    :::: "$CPP_LIST"
else

    echo "GNU parallel não encontrado"
    exit 1

fi


################################################################################
# RESULTADO
################################################################################

find "$BUILD_DIR" \
    -type f \
    -name "*.o" \
    | sort > "$OBJ_LIST"


OK=$(wc -l < "$OBJ_LIST")
FAIL=$((TOTAL-OK))


echo
echo "===================================="
echo "TOTAL : $TOTAL"
echo "OK    : $OK"
echo "FAIL  : $FAIL"
echo "===================================="


if [ "$FAIL" -ne 0 ]
then
    echo
    echo "Erros:"
    grep -Ei "error:|fatal:" "$LOG" | tail -200 || true
fi
