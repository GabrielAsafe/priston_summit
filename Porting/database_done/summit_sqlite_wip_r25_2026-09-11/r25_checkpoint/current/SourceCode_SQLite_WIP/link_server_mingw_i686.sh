#!/usr/bin/env bash
set -u
set -o pipefail

ROOT="$(pwd)"
CXX="${CXX:-i686-w64-mingw32-g++}"
BUILD_DIR="$ROOT/build_mingw_server_i686"
OBJ_DIR="$BUILD_DIR/objects"
OBJ_LIST="$ROOT/object_files_server_mingw.txt"
OUT_DIR="$ROOT/dist_mingw_server_i686"
OUTPUT="$OUT_DIR/Server.exe"
LOG="$ROOT/server_link_i686.log"
RSP="$BUILD_DIR/server_objects.rsp"
MISSING="$ROOT/server_link_missing_i686.txt"
RELEASE="${RELEASE:-0}"

if ! command -v "$CXX" >/dev/null 2>&1; then
    echo "Erro: compilador não encontrado: $CXX"
    exit 1
fi

if [ ! -d "$OBJ_DIR" ]; then
    echo "Erro: diretório de objetos não encontrado: $OBJ_DIR"
    echo "Execute primeiro o build do servidor."
    exit 1
fi

if [ ! -s "$OBJ_LIST" ]; then
    echo "Erro: lista de objetos não encontrada ou vazia: $OBJ_LIST"
    echo "Execute primeiro o build do servidor para gerar a lista correta."
    exit 1
fi

mkdir -p "$OUT_DIR" "$BUILD_DIR"

# Usa SOMENTE os objetos listados pelo script de compilação.
# Isso impede que .o antigos (do layout anterior) entrem novamente no link.
: > "$RSP"
TOTAL=0
MISSING_OBJECTS=0

declare -A SEEN_OBJECTS=()
while IFS= read -r obj || [ -n "$obj" ]; do
    [ -n "$obj" ] || continue

    if [[ "$obj" != /* ]]; then
        obj="$ROOT/$obj"
    fi

    if [ -n "${SEEN_OBJECTS[$obj]+x}" ]; then
        echo "Aviso: objeto duplicado ignorado na lista: $obj"
        continue
    fi
    SEEN_OBJECTS["$obj"]=1

    if [ ! -f "$obj" ]; then
        echo "Erro: objeto listado não existe: $obj"
        MISSING_OBJECTS=$((MISSING_OBJECTS + 1))
        continue
    fi

    printf '"%s"\n' "$obj" >> "$RSP"
    TOTAL=$((TOTAL + 1))
done < "$OBJ_LIST"

if [ "$MISSING_OBJECTS" -ne 0 ]; then
    echo "Erro: $MISSING_OBJECTS objeto(s) da lista estão ausentes."
    echo "Rode novamente o build do servidor antes de linkar."
    exit 1
fi

if [ "$TOTAL" -eq 0 ]; then
    echo "Erro: nenhum objeto válido encontrado na lista: $OBJ_LIST"
    exit 1
fi

LDFLAGS=(
    -m32
    -mconsole
    -Wl,--large-address-aware
)

if [ "$RELEASE" = "1" ]; then
    LDFLAGS+=(-s)
fi

LIB_DIRS=(
    -L"$ROOT/build_mingw_lua_i686"
    -L"$ROOT/build_mingw_lua"
    -L"$ROOT/Dependency/lib"
    -L"$ROOT/Dependency/Lib"
)

LIBS=(
    -lws2_32
    -lwinmm
    -lcomctl32
    -lcomdlg32
    -lgdi32
    -luser32
    -lkernel32
    -ladvapi32
    -lole32
    -loleaut32
    -luuid
    -lversion
    -lshlwapi
    -lpsapi
    -ldbghelp
    -limm32
    -lddraw
    -ldxguid
    -ldinput8
    -lodbc32
    -lodbccp32
    -lwininet
    -lstrmiids
)

rm -f "$OUTPUT" "$LOG" "$MISSING"

echo
echo "===================================="
echo "LINK SERVIDOR MINGW i686"
echo "===================================="
echo "Objetos : $TOTAL"
echo "Lista   : $OBJ_LIST"
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
