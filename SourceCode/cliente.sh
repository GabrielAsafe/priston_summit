#!/usr/bin/env bash
set -euo pipefail

################################################################################
# SummitTale — build, link e envio do cliente MinGW i686
################################################################################

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

BUILD_SCRIPT="${BUILD_SCRIPT:-build_client_mingw_i686_multicore.sh}"
LINK_SCRIPT="${LINK_SCRIPT:-link_client_mingw_i686.sh}"
DIST_DIR="${DIST_DIR:-dist_mingw_client_i686}"
CLIENT_EXE="${CLIENT_EXE:-Game.exe}"

# Pode ser alterado sem editar o script:
# DEST='utilizador@host:C:/caminho' PORT=2223 ./cliente.sh
DEST="${DEST:-server@127.0.0.1:C:/Users/server/Desktop/cli}"


PORT="${PORT:-2223}"

MINGW_BIN="${MINGW_BIN:-/usr/i686-w64-mingw32/sys-root/mingw/bin}"

RUNTIME_DLLS=(
    libgcc_s_dw2-1.dll
    libstdc++-6.dll
    libwinpthread-1.dll
)

log_section()
{
    echo
    echo "===================================="
    echo "$1"
    echo "===================================="
}

require_file()
{
    if [ ! -f "$1" ]; then
        echo "Erro: ficheiro não encontrado: $1" >&2
        exit 1
    fi
}

require_command()
{
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "Erro: comando não encontrado: $1" >&2
        exit 1
    fi
}

require_file "$BUILD_SCRIPT"
require_file "$LINK_SCRIPT"
require_command bash
require_command scp

log_section "1/4 — COMPILAÇÃO DO CLIENTE"
bash "$BUILD_SCRIPT"

log_section "2/4 — LINK DO CLIENTE"
bash "$LINK_SCRIPT"

OUTPUT="$DIST_DIR/$CLIENT_EXE"
require_file "$OUTPUT"

log_section "3/4 — DLLs DE RUNTIME"
mkdir -p "$DIST_DIR"

FILES_TO_SEND=("$OUTPUT")

for dll in "${RUNTIME_DLLS[@]}"; do
    source_path="$MINGW_BIN/$dll"
    target_path="$DIST_DIR/$dll"

    if [ -f "$source_path" ]; then
        cp -f "$source_path" "$target_path"
        FILES_TO_SEND+=("$target_path")
        echo "OK   $dll"
    elif [ -f "$target_path" ]; then
        FILES_TO_SEND+=("$target_path")
        echo "OK   $dll (já existente em $DIST_DIR)"
    else
        echo "Aviso: DLL não encontrada: $source_path" >&2
    fi
done

log_section "4/4 — ENVIO DO CLIENTE"
echo "Destino : $DEST"
echo "Porta   : $PORT"
echo "Ficheiros:"
printf '  %s\n' "${FILES_TO_SEND[@]}"
echo

scp -P "$PORT" "${FILES_TO_SEND[@]}" "$DEST"

log_section "CLIENTE CONCLUÍDO"
echo "Executável: $OUTPUT"
echo "Enviado para: $DEST"
