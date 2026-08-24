#!/usr/bin/env bash
set -euo pipefail

CXX="i686-w64-mingw32-g++"
OBJDIR="build_mingw_server_i686"
OUTDIR="dist_mingw_server_i686"
OUTPUT="$OUTDIR/Server.exe"
LOG="server_link_i686.log"

mkdir -p "$OUTDIR"

mapfile -d '' OBJECTS < <(
    find "$OBJDIR" -type f -iname '*.o' -print0 | sort -z
)

if [ "${#OBJECTS[@]}" -eq 0 ]; then
    echo "Erro: nenhum .o encontrado em $OBJDIR"
    exit 1
fi




echo "Objetos encontrados: ${#OBJECTS[@]}"
echo "Saída: $OUTPUT"
echo

rm -f "$OUTPUT" "$LOG"

"$CXX" \
    -m32 \
    -mconsole \
    -Wl,--large-address-aware \
    -o "$OUTPUT" \
    "${OBJECTS[@]}" \
    -Lbuild_mingw_lua_i686 \
    -Lbuild_mingw_lua \
    -LDependency/lib \
    -LDependency/Lib \
    -lws2_32 \
    -lwinmm \
    -lcomctl32 \
    -lcomdlg32 \
    -lgdi32 \
    -luser32 \
    -lkernel32 \
    -ladvapi32 \
    -lole32 \
    -loleaut32 \
    -luuid \
    -lversion \
    -lshlwapi \
    -lpsapi \
    -ldbghelp \
    -limm32 \
    -lddraw \
    -ldxguid \
    -ldinput8 \
    -lodbc32 \
   -lodbccp32 \
   -lwininet \
-lwininet \
-lstrmiids \
    2>"$LOG" || true

if [ -f "$OUTPUT" ]; then
    echo "LINK OK"
    file "$OUTPUT"
    ls -lh "$OUTPUT"
else
    echo "LINK FALHOU"
    echo
    grep -E \
        'undefined reference|multiple definition|cannot find|collect2:|ld.exe:' \
        "$LOG" |
        head -160
fi
