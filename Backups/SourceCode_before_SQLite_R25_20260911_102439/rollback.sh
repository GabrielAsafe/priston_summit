#!/usr/bin/env bash
set -Eeuo pipefail

TARGET_SRC=/home/gabriel/Área\ de\ Trabalho/summit/SourceCode
BACKUP_FILES=/home/gabriel/Área\ de\ Trabalho/summit/Backups/SourceCode_before_SQLite_R25_20260911_102439/files
NEW_FILES=/home/gabriel/Área\ de\ Trabalho/summit/Backups/SourceCode_before_SQLite_R25_20260911_102439/new_files.txt

echo "Rollback SQLite R25"
echo "Destino: $TARGET_SRC"
echo

if [[ ! -d "$BACKUP_FILES" ]]; then
    echo "Backup não encontrado: $BACKUP_FILES" >&2
    exit 1
fi

# Remove somente ficheiros que não existiam antes.
if [[ -f "$NEW_FILES" ]]; then
    while IFS= read -r rel; do
        [[ -n "$rel" ]] || continue
        rm -f -- "$TARGET_SRC/$rel"
    done < "$NEW_FILES"
fi

# Restaura todos os ficheiros originais preservando estrutura.
while IFS= read -r -d '' src; do
    rel="${src#"$BACKUP_FILES/"}"
    dst="$TARGET_SRC/$rel"
    mkdir -p -- "$(dirname "$dst")"
    cp -a -- "$src" "$dst"
done < <(find "$BACKUP_FILES" -type f -print0)

# Remove diretórios vazios criados pela integração, sem tocar em conteúdo alheio.
find "$TARGET_SRC/SrcServer/src/systems/persistence/database/sqlite" -depth -type d -empty -delete 2>/dev/null || true
find "$TARGET_SRC/Database/migrations" -depth -type d -empty -delete 2>/dev/null || true
find "$TARGET_SRC/tools" -depth -type d -empty -delete 2>/dev/null || true

echo
echo "Rollback concluído."
