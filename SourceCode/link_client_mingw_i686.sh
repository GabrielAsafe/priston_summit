#!/usr/bin/env bash
set -u
set -o pipefail

ROOT="$(pwd)"
CXX="${CXX:-i686-w64-mingw32-g++}"
WINDRES="${WINDRES:-i686-w64-mingw32-windres}"
BUILD_DIR="$ROOT/build_mingw_client_i686"
OBJ_DIR="$BUILD_DIR/objects"
OBJ_LIST="$ROOT/object_files_client_mingw.txt"
OUT_DIR="$ROOT/dist_mingw_client_i686"
OUTPUT="$OUT_DIR/Game.exe"
LOG="$ROOT/client_link_i686.log"
RSP="$BUILD_DIR/client_objects.rsp"
MISSING="$ROOT/client_link_missing_i686.txt"
RELEASE="${RELEASE:-0}"
CLIENT_RESOURCE_DIR="$ROOT/SrcGame/src/platform/windows/resources"
CLIENT_RESOURCE_RC="${CLIENT_RESOURCE_DIR}/Script1.rc"
CLIENT_RESOURCE_OBJ="$BUILD_DIR/client_resources.o"

if ! command -v "$CXX" >/dev/null 2>&1; then
    echo "Erro: compilador não encontrado: $CXX"
    exit 1
fi

if ! command -v "$WINDRES" >/dev/null 2>&1; then
    echo "Erro: windres não encontrado: $WINDRES"
    exit 1
fi

if [ ! -d "$OBJ_DIR" ]; then
    echo "Erro: diretório de objetos não encontrado: $OBJ_DIR"
    echo "Execute primeiro o build do cliente."
    exit 1
fi

if [ ! -s "$OBJ_LIST" ]; then
    echo "Erro: lista de objetos não encontrada ou vazia: $OBJ_LIST"
    echo "Execute primeiro o build do cliente para gerar a lista correta."
    exit 1
fi

mkdir -p "$OUT_DIR" "$BUILD_DIR"

# Gera response file EXCLUSIVAMENTE a partir da lista produzida pelo compilador.
# Não usa 'find', evitando objetos antigos/duplicados que possam ter ficado no build.
: > "$RSP"
TOTAL=0
MISSING_OBJECTS=0

declare -A SEEN_OBJECTS=()
while IFS= read -r obj || [ -n "$obj" ]; do
    [ -n "$obj" ] || continue

    # Aceita tanto caminhos absolutos quanto relativos.
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
    echo "Rode novamente o build do cliente antes de linkar."
    exit 1
fi

if [ "$TOTAL" -eq 0 ]; then
    echo "Erro: nenhum objeto válido encontrado na lista: $OBJ_LIST"
    exit 1
fi

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

if [ ! -f "${CLIENT_RESOURCE_RC}" ]; then
    echo "Erro: resource script não encontrado: ${CLIENT_RESOURCE_RC}"
    exit 1
fi
if [ ! -f "${CLIENT_RESOURCE_DIR}/resource.h" ]; then
    echo "Erro: resource.h não encontrado: ${CLIENT_RESOURCE_DIR}/resource.h"
    exit 1
fi
rm -f "${CLIENT_RESOURCE_OBJ}"
(
    cd "${CLIENT_RESOURCE_DIR}" || exit 1
    "$WINDRES" \
        -I "${CLIENT_RESOURCE_DIR}" \
        -I "$ROOT/SrcGame/src" \
        --input-format=rc \
        --output-format=coff \
        -i "Script1.rc" \
        -o "${CLIENT_RESOURCE_OBJ}"
) || {
    echo "Erro: falha ao compilar resources client"
    exit 1
}
if [ ! -s "${CLIENT_RESOURCE_OBJ}" ]; then
    echo "Erro: resource object vazio/ausente: ${CLIENT_RESOURCE_OBJ}"
    exit 1
fi

rm -f "$OUTPUT" "$LOG" "$MISSING"

echo
echo "===================================="
echo "LINK CLIENTE MINGW i686"
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
    "${CLIENT_RESOURCE_OBJ}" \
    "${LIB_DIRS[@]}" \
    "${LIBS[@]}" \
    >"$LOG" 2>&1

STATUS=$?

if [ "$STATUS" -eq 0 ] && [ -f "$OUTPUT" ]; then
    echo "LINK OK"
    echo
    file "$OUTPUT"
    ls -lh "$OUTPUT"

    if command -v i686-w64-mingw32-objdump >/dev/null 2>&1; then
        if i686-w64-mingw32-objdump -h "$OUTPUT" 2>/dev/null | grep -q "\.rsrc"; then
            echo "CLIENT_PE_RESOURCE_SECTION=PASS"
        else
            echo "CLIENT_PE_RESOURCE_SECTION=FAIL"
            exit 1
        fi
    fi

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
