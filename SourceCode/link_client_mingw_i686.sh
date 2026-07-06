#!/usr/bin/env bash
set -u

OBJDIR="build_mingw_client_i686"
OUTDIR="dist_mingw_client_i686"
OUTEXE="$OUTDIR/SummitTaleClient.exe"
LOG="build_mingw_client_i686_link.log"

mkdir -p "$OUTDIR"
: > "$LOG"

echo "Objetos encontrados:"
find "$OBJDIR" -name "*.o" | sort > object_files_mingw.txt
wc -l object_files_mingw.txt

echo
echo "Iniciando linkagem..."

i686-w64-mingw32-g++ -mwindows \
  -o "$OUTEXE" \
  @object_files_mingw.txt \
  build_mingw_lua_i686/liblua.a \
   \
  -LDependency/DirectX9/Lib/x64 \
  -ld3d9 \
  -ld3dx9 \
  -lddraw \
  -ldinput8 \
  -ldxguid \
  -lwinmm \
  -lvfw32 \
  -ldsound \
  -lshlwapi \
  -lodbc32 \
  -limm32 \
  -liphlpapi \
  -lstrmiids \
  -lquartz \
  -lws2_32 \
  -lwininet \
  -lpsapi \
  -limagehlp \
  -lcomctl32 \
  -lole32 \
  -loleaut32 \
  -luuid \
  -lgdi32 \
  -luser32 \
  -lkernel32 \
  -lcomdlg32 \
  -ladvapi32 \
  2> "$LOG"

STATUS=$?

echo
if [ "$STATUS" -eq 0 ]; then
    echo "LINK OK:"
    ls -lh "$OUTEXE"
else
    echo "LINK FALHOU. Log:"
    grep -niE "undefined reference|cannot find|multiple definition|ld returned|collect2|error:" "$LOG" | tail -200 || true
fi

exit "$STATUS"
