#!/bin/sh
# Executar a partir de qualquer diretório; colocar este ficheiro em SourceCode/send_to_client.sh
set -eu

SOURCE_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
REPO_DIR=$(CDPATH= cd -- "$SOURCE_DIR/.." && pwd -P)
BUILD_DIR="$SOURCE_DIR/dist_mingw_client_i686"
CLIENT_DIR="$REPO_DIR/cliente"
BACKUP_DIR="$HOME/summit_auditorias/backups_deploy_cliente/$(date -u +%Y%m%dT%H%M%SZ)"
FILES='Game.exe libgcc_s_dw2-1.dll libstdc++-6.dll libwinpthread-1.dll'

if [ ! -d "$CLIENT_DIR" ]; then
    echo "ERRO: destino cliente inexistente: $CLIENT_DIR" >&2
    exit 1
fi
for f in $FILES; do
    if [ ! -s "$BUILD_DIR/$f" ]; then
        echo "ERRO: build ausente ou vazia: $BUILD_DIR/$f" >&2
        echo 'Compila o cliente antes de enviar. Nada foi alterado.' >&2
        exit 1
    fi
done

echo "SourceCode: $SOURCE_DIR"
echo "Build: $BUILD_DIR"
echo "Cliente: $CLIENT_DIR"
echo "Backup: $BACKUP_DIR"

if [ "${1:-}" != '--apply' ]; then
    echo 'PREVIEW=PASS. Para copiar, usa: sh SourceCode/send_to_client.sh --apply'
    for f in $FILES; do
        if [ -f "$CLIENT_DIR/$f" ]; then
            if cmp -s "$BUILD_DIR/$f" "$CLIENT_DIR/$f"; then
                echo "IDÊNTICO $f"
            else
                echo "DIFERENTE $f"
            fi
        else
            echo "NOVO $f"
        fi
    done
    exit 0
fi

mkdir -p "$BACKUP_DIR"
for f in $FILES; do
    if [ -f "$CLIENT_DIR/$f" ]; then
        cp -p -- "$CLIENT_DIR/$f" "$BACKUP_DIR/$f"
    fi
done

for f in $FILES; do
    tmp="$CLIENT_DIR/.${f}.summit-deploy-$$"
    if ! cp -- "$BUILD_DIR/$f" "$tmp"; then
        rm -f -- "$tmp"
        echo "ERRO ao copiar $f; backups em $BACKUP_DIR" >&2
        exit 1
    fi
    if ! cmp -s -- "$BUILD_DIR/$f" "$tmp"; then
        rm -f -- "$tmp"
        echo "ERRO de integridade antes de instalar $f" >&2
        exit 1
    fi
    mv -f -- "$tmp" "$CLIENT_DIR/$f"
    if ! cmp -s -- "$BUILD_DIR/$f" "$CLIENT_DIR/$f"; then
        echo "ERRO de integridade após instalar $f; backups em $BACKUP_DIR" >&2
        exit 1
    fi
    sha256sum "$CLIENT_DIR/$f"
done

echo 'CLIENTE_LOCAL_ATUALIZADO=PASS'
echo "BACKUP=$BACKUP_DIR"
