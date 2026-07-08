#!/usr/bin/env bash
set -u

ROOT="$(pwd)"
BUILD_CLIENT="build_mingw_client_i686"
BUILD_SERVER="build_mingw_server_i686"
DIST="dist_mingw_all_i686"

CXX=i686-w64-mingw32-g++
CC=i686-w64-mingw32-gcc
AR=i686-w64-mingw32-ar

CXXFLAGS="-m32 -std=gnu++98 -fpermissive -w -DWIN32 -D_WINDOWS -D_M_IX86 -Di386 -D__i386__"
CXXFLAGS="$CXXFLAGS -I. -ISrcGame/src -IDependency/DirectXSDK/Microsoft\ DirectX\ SDK\ \(June\ 2010\)/Include"

LDFLAGS="-m32 -mwindows"
LIBS="-lws2_32 -lwinmm -lgdi32 -lole32 -loleaut32 -luuid -ldinput8 -lddraw -ldsound -ldxguid -ld3d9 -ld3dx9"

rm -rf "$BUILD_CLIENT" "$BUILD_SERVER" "$DIST"
mkdir -p "$BUILD_CLIENT" "$BUILD_SERVER" "$DIST"

find . \
  -type f \( -iname "*.cpp" -o -iname "*.cxx" -o -iname "*.cc" -o -iname "*.c" -o -iname "*.CPP" \) \
  | sed 's#^\./##' \
  | sort > cpp_files_all_i686.txt

grep -Ei '(^|/)(SrcGame|modules)/|\.CPP$' cpp_files_all_i686.txt \
  | grep -Eiv '(^|/)(build_|dist_|lixo_|Dependency/DirectXSDK|Dependency/DXSDK|Backup|backup)' \
  | grep -Eiv 'AntiCheat' \
  > cpp_files_client_i686.txt

grep -Eiv '(^|/)(build_|dist_|lixo_|Dependency/DirectXSDK|Dependency/DXSDK|Backup|backup)' cpp_files_all_i686.txt \
  | grep -Eiv 'SrcGame/src/Winmain\.cpp|AntiCheat' \
  > cpp_files_server_i686.txt

echo "CLIENT CPP:"
wc -l cpp_files_client_i686.txt

echo "SERVER CPP:"
wc -l cpp_files_server_i686.txt

compile_list() {
  local list="$1"
  local outdir="$2"
  local fail=0

  while IFS= read -r src; do
    [ -z "$src" ] && continue

    obj="$outdir/$(echo "$src" | sed 's#[/ ]#_#g; s#\.[^.]*$#.o#')"

    echo "[CXX] $src"
    $CXX $CXXFLAGS -c "$src" -o "$obj" >> "${outdir}_compile.log" 2>&1
    rc=$?

    if [ $rc -ne 0 ]; then
      echo "FAIL: $src"
      fail=1
    fi
  done < "$list"

  return $fail
}

: > "${BUILD_CLIENT}_compile.log"
: > "${BUILD_SERVER}_compile.log"

compile_list cpp_files_client_i686.txt "$BUILD_CLIENT"
CLIENT_RC=$?

compile_list cpp_files_server_i686.txt "$BUILD_SERVER"
SERVER_RC=$?

echo "CLIENT OBJ:"
find "$BUILD_CLIENT" -type f -name "*.o" | wc -l

echo "SERVER OBJ:"
find "$BUILD_SERVER" -type f -name "*.o" | wc -l

find "$BUILD_CLIENT" -iname "*Winmain*.o"

if [ $CLIENT_RC -ne 0 ]; then
  echo "CLIENT COMPILE FALHOU"
  grep -Ei "error:|undefined reference|multiple definition|cannot find|ld returned|collect2" "${BUILD_CLIENT}_compile.log" | tail -200
  exit 1
fi

if [ $SERVER_RC -ne 0 ]; then
  echo "SERVER COMPILE FALHOU"
  grep -Ei "error:|undefined reference|multiple definition|cannot find|ld returned|collect2" "${BUILD_SERVER}_compile.log" | tail -200
  exit 1
fi

echo "LINK CLIENT..."
$CXX $LDFLAGS \
  $(find "$BUILD_CLIENT" -type f -name "*.o" | sort) \
  -o "$DIST/SummitTaleClient.exe" \
  -L"Dependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Lib/x86" \
  $LIBS \
  > "${BUILD_CLIENT}_link.log" 2>&1

if [ $? -ne 0 ]; then
  echo "LINK CLIENT FALHOU"
  grep -Ei "error:|undefined reference|multiple definition|cannot find|ld returned|collect2" "${BUILD_CLIENT}_link.log" | tail -200
  exit 1
fi

echo "LINK SERVER..."
$CXX -m32 \
  $(find "$BUILD_SERVER" -type f -name "*.o" | sort) \
  -o "$DIST/SummitTaleServer.exe" \
  -lws2_32 -lwinmm -lgdi32 \
  > "${BUILD_SERVER}_link.log" 2>&1

if [ $? -ne 0 ]; then
  echo "LINK SERVER FALHOU"
  grep -Ei "error:|undefined reference|multiple definition|cannot find|ld returned|collect2" "${BUILD_SERVER}_link.log" | tail -300
  exit 1
fi

cp -v /usr/i686-w64-mingw32/sys-root/mingw/bin/libgcc_s_dw2-1.dll "$DIST/"
cp -v /usr/i686-w64-mingw32/sys-root/mingw/bin/libwinpthread-1.dll "$DIST/"
cp -v /usr/i686-w64-mingw32/sys-root/mingw/bin/libstdc++-6.dll "$DIST/"

cabextract -F d3dx9_43.dll \
  "Dependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Redist/Jun2010_d3dx9_43_x86.cab" \
  -d "$DIST"

file "$DIST/SummitTaleClient.exe"
file "$DIST/SummitTaleServer.exe"

echo "OK CLIENT + SERVER"
