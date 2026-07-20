#!/usr/bin/env bash
set -u

BUILD_DIR="build_mingw_server_i686"
OBJ_LIST="object_files_server_mingw.txt"
CPP_LIST="cpp_files_server_i686.txt"

CXX=i686-w64-mingw32-g++

DX_INC="Dependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Include"

CXXFLAGS=(
  -m32
  -std=gnu++11
  -fpermissive
  -w
  -DWIN32
  -D_WINDOWS
  -D_M_IX86
  -Di386
  -D__i386__
  -D__in=
  -D__out=
  -D__in_opt=
  -D__out_opt=
  '-D__in_ecount(x)='
  '-D__out_ecount(x)='
  '-D__in_ecount_opt(x)='
  '-D__out_ecount_opt(x)='
  '-DSAL__in_ecount_opt(x)='
  '-D__in_bcount(x)='
  '-D__out_bcount(x)='
  '-D__in_bcount_opt(x)='
  '-D__out_bcount_opt(x)='
  '-D__deref_out_bcount(x)='
  '-D__deref_out_bcount_opt(x)='
  -D__inout=
  -D__inout_opt=

  -I.
  -ISrcServer/src
  -ISrcServer/src/sinbaram
  -ISrcServer/src/Skills
  -ISrcServer/src/HoBaram/NewEffect/Script
  -ISrcServer/src/HoBaram/NewEffect/Script/lua
  -ISrcGame/src
  -IShared
  -IShared2
  -Imodules
  -Imodules/File
  -I"Dependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Include"
)

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

find SrcServer/src modules \
  -type f \( -iname "*.cpp" -o -iname "*.CPP" -o -iname "*.c" \) \
  | grep -Eiv 'AntiCheat|Lua\.bak|\.bak|bak_|corrupt|/lua/lua\.c|/luac/|ltests\.c' \
  | sort > "$CPP_LIST"

cat >> "$CPP_LIST" <<'EOF'
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
EOF

sort -u "$CPP_LIST" -o "$CPP_LIST"

echo "Total CPP servidor:"
wc -l "$CPP_LIST"

: > build_server_i686_compile_errors.log

ok=0
fail=0
total=$(wc -l < "$CPP_LIST")
idx=0

while IFS= read -r src; do
  [ -z "$src" ] && continue
  idx=$((idx+1))

  obj="$BUILD_DIR/$(echo "$src" | sed 's#[/ ]#_#g; s#\.[^.]*$#.o#')"

  printf "\r%3d%% %d:%d %s" $((idx*100/total)) "$ok" "$fail" "$src"

  "$CXX" "${CXXFLAGS[@]}" -c "$src" -o "$obj" >> build_server_i686_compile_errors.log 2>&1

  if [ $? -eq 0 ]; then
    ok=$((ok+1))
  else
    fail=$((fail+1))
    rm -f "$obj"
  fi
done < "$CPP_LIST"

echo
find "$BUILD_DIR" -type f -name "*.o" | sort > "$OBJ_LIST"

echo "Objetos encontrados:"
wc -l "$OBJ_LIST"

echo "Winmain:"
find "$BUILD_DIR" -iname "*Winmain*.o"

echo "Packet:"
find "$BUILD_DIR" -iname "*Packet*.o"

if [ "$fail" -ne 0 ]; then
  echo "Alguns CPP falharam. Erros:"
  grep -Ei "error:|fatal:" build_server_i686_compile_errors.log | tail -200
fi
