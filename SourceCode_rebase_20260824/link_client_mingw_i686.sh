#!/usr/bin/env bash
set -u
set -o pipefail

ROOT="$(pwd)"
CXX="${CXX:-i686-w64-mingw32-g++}"
BUILD_DIR="$ROOT/build_mingw_client_i686"
OBJ_DIR="$BUILD_DIR/objects"
OBJ_LIST="$ROOT/object_files_client_mingw.txt"
OUT_DIR="$ROOT/dist_mingw_client_i686"
OUTPUT="$OUT_DIR/Game.exe"
LOG="$ROOT/client_link_i686.log"
RSP="$BUILD_DIR/client_objects.rsp"
MISSING="$ROOT/client_link_missing_i686.txt"
RELEASE="${RELEASE:-0}"

if ! command -v "$CXX" >/dev/null 2>&1; then
    echo "Erro: compilador não encontrado: $CXX"
    exit 1
fi

if [ ! -d "$OBJ_DIR" ]; then
    echo "Erro: diretório de objetos não encontrado: $OBJ_DIR"
    exit 1
fi

mkdir -p "$OUT_DIR" "$BUILD_DIR"

if [ ! -s "$OBJ_LIST" ]; then
    find "$OBJ_DIR" -type f -name '*.o' | sort > "$OBJ_LIST"
fi

TOTAL="$(wc -l < "$OBJ_LIST")"
if [ "$TOTAL" -eq 0 ]; then
    echo "Erro: nenhum objeto encontrado em $OBJ_DIR"
    exit 1
fi

: > "$RSP"
while IFS= read -r obj; do
    [ -n "$obj" ] || continue
    if [ ! -f "$obj" ]; then
        echo "Erro: objeto listado não existe: $obj"
        exit 1
    fi
    printf '"%s"\n' "$obj" >> "$RSP"
done < "$OBJ_LIST"

LDFLAGS=(
    -m32
    -mwindows
    -Wl,--no-keep-memory
    -Wl,--gc-sections
    -Wl,--subsystem,windows
)

if [ "$RELEASE" = "1" ]; then
    LDFLAGS+=(-s)
fi

LIB_DIRS=(
    -L"$ROOT/build_mingw_lua_i686"
    -L"$ROOT/build_mingw_lua"
    -L"$ROOT/Dependency/lib"
    -L"$ROOT/Dependency/Lib"
#    -L"$ROOT/Dependency/DirectX9/Lib/x86"
#    -L"$ROOT/Dependency/DirectX9/Lib"
#    -L"$ROOT/Dependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Lib/x86"
)

LIBS=(
    -Wl,--start-group
    -llua
    -ld3d9
    -ld3dx9_43
    -ldsound
    -ldxguid
    -ldinput8
    -lddraw
    -lstrmiids
    -lvfw32
    -lws2_32
    -lwininet
    -liphlpapi
    -lcomctl32
    -lcomdlg32
    -lgdi32
    -luser32
    -lkernel32
    -ladvapi32
    -lshell32
    -lshlwapi
    -limm32
    -lwinmm
    -lversion
    -lpsapi
    -ldbghelp
    -limagehlp
    -lrpcrt4
    -lole32
    -loleaut32
    -luuid
    -lodbc32
    -lodbccp32
    -Wl,--end-group
)

rm -f "$OUTPUT" "$LOG" "$MISSING"

echo
echo "===================================="
echo "LINK CLIENTE MINGW i686"
echo "===================================="
echo "Objetos : $TOTAL"
echo "Saída   : $OUTPUT"
echo "Release : $RELEASE"
echo "===================================="
echo

"$CXX" \
    "${LDFLAGS[@]}" \
    -o "$OUTPUT" \
    "@$RSP" \
    "${LIB_DIRS[@]}" \
    "${LIBS[@]}" \
    >"$LOG" 2>&1

STATUS=$?

if [ "$STATUS" -eq 0 ] && [ -f "$OUTPUT" ]; then
    echo "LINK OK"
    echo
    file "$OUTPUT"
    ls -lh "$OUTPUT"

    echo
    echo "Dependências DLL declaradas:"
    if command -v i686-w64-mingw32-objdump >/dev/null 2>&1; then
        i686-w64-mingw32-objdump -p "$OUTPUT" 2>/dev/null \
            | grep 'DLL Name:' \
            | sort -u \
            || true
    fi

    exit 0
fi

echo "LINK FALHOU"
echo "Status: $STATUS"
echo "Log: $LOG"
echo

grep -E \
    'undefined reference|multiple definition|cannot find|file format not recognized|collect2:|ld.exe:|ld:' \
    "$LOG" \
    > "$MISSING" \
    || true

echo "Primeiros erros relevantes:"
head -200 "$MISSING" || true

echo
echo "Relatório: $MISSING"
exit 1
