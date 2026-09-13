#!/usr/bin/env bash
set -Eeuo pipefail

# Integração segura Summit SQLite R25 -> SourceCode
#
# Uso normal:
#   chmod +x integrar_r25_no_source.sh
#   ./integrar_r25_no_source.sh
#
# Dry-run:
#   ./integrar_r25_no_source.sh --dry-run
#
# Overrides opcionais:
#   R25_SRC="/caminho/SourceCode_SQLite_WIP" \
#   TARGET_SRC="/caminho/SourceCode" \
#   ./integrar_r25_no_source.sh
#
# O script:
#   1. valida a origem R25;
#   2. cria backup timestampado de tudo que for sobrescrito;
#   3. registra os ficheiros novos para rollback;
#   4. cria toda a estrutura de diretórios necessária;
#   5. copia runtime C++/headers, migrations 001..032 e tools operacionais;
#   6. integra SQLiteConnection.cpp na lista de compilação SOMENTE se necessário;
#   7. verifica SHA-256 de todos os ficheiros copiados;
#   8. gera rollback.sh automático no diretório de backup.
#
# IMPORTANTE:
#   - NÃO adiciona sqlite3.lib / -lsqlite3: o R25 usa LoadLibraryA("sqlite3.dll").
#   - NÃO remove nem altera Database.dll.
#   - NÃO limpa build dirs.
#   - sqlite3.dll x86 continua sendo requisito de runtime e não vem deste patch.

DRY_RUN=0
if [[ "${1:-}" == "--dry-run" ]]; then
    DRY_RUN=1
elif [[ $# -gt 0 ]]; then
    echo "Uso: $0 [--dry-run]" >&2
    exit 2
fi

DEFAULT_R25_SRC="/home/gabriel/Área de Trabalho/summit/Porting/database/summit_sqlite_wip_r25_2026-09-11/r25_checkpoint/current/SourceCode_SQLite_WIP"
DEFAULT_TARGET="/home/gabriel/Área de Trabalho/summit/SourceCode"
DEFAULT_BACKUP_BASE="/home/gabriel/Área de Trabalho/summit/Backups"

R25_SRC="${R25_SRC:-$DEFAULT_R25_SRC}"
TARGET_SRC="${TARGET_SRC:-$DEFAULT_TARGET}"
BACKUP_BASE="${BACKUP_BASE:-$DEFAULT_BACKUP_BASE}"

timestamp="$(date +%Y%m%d_%H%M%S)"
BACKUP_DIR="$BACKUP_BASE/SourceCode_before_SQLite_R25_$timestamp"
BACKUP_FILES="$BACKUP_DIR/files"
MANIFEST="$BACKUP_DIR/integration_manifest.tsv"
NEW_FILES="$BACKUP_DIR/new_files.txt"
BUILD_SNAPSHOT="$BACKUP_DIR/build_metadata"

say()  { printf '\n==> %s\n' "$*"; }
info() { printf '    %s\n' "$*"; }
die()  { printf '\nERRO: %s\n' "$*" >&2; exit 1; }

run() {
    if (( DRY_RUN )); then
        printf 'DRY-RUN: '
        printf '%q ' "$@"
        printf '\n'
    else
        "$@"
    fi
}

sha() {
    sha256sum -- "$1" | awk '{print $1}'
}

# Ficheiros C++/headers que compõem a integração runtime R25.
RUNTIME_FILES=(
    "Shared/persistence/database/Database_sqlite_server.h"
    "Shared/persistence/database/Database_sqlite_server.inl"
    "Shared/server/legacy_runtime/OnSever_legacy_server.inl"

    "SrcServer/src/systems/persistence/database/core/Database.cpp"
    "SrcServer/src/systems/persistence/database/core/Database.h"
    "SrcServer/src/systems/persistence/database/odbc/SQL.cpp"
    "SrcServer/src/systems/persistence/database/odbc/SQL.h"

    "SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp"
    "SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.h"
    "SrcServer/src/systems/persistence/database/sqlite/SqliteQueryCompat.h"
    "SrcServer/src/systems/persistence/database/sqlite/SqliteQueryCompat.inl"

    "SrcServer/src/core/server/legacy_runtime/DllServer.cpp"
    "SrcServer/src/core/server/legacy_runtime/DllServer.h"
    "SrcServer/src/core/server/legacy_runtime/PartySever.cpp"
    "SrcServer/src/core/server/legacy_runtime/onserver.h"

    "SrcServer/src/systems/admin/game_masters/GameMasters.cpp"
)

TOOLS_FILES=(
    "tools/sqlite_local_admin.py"
    "tools/run_controlled_validation.py"
    "tools/windows_sqlite_preflight.ps1"
)

# Build metadata: é copiado para o backup por segurança.
# O script só altera cpp_files_server_i686.txt quando detecta que a build depende
# de uma lista estática e SQLiteConnection.cpp ainda não consta nela.
BUILD_METADATA=(
    "build_server_mingw_i686_j8.sh"
    "link_server_mingw_i686.sh"
    "cpp_files_server_i686.txt"
    "object_files_server_mingw.txt"
    "server_deploy_extra.txt"
)

SQLITE_CPP_REL="SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp"

say "Validando caminhos"
[[ -d "$R25_SRC" ]] || die "Origem R25 não encontrada: $R25_SRC"
[[ -d "$TARGET_SRC" ]] || die "SourceCode destino não encontrado: $TARGET_SRC"
[[ "$R25_SRC" != "$TARGET_SRC" ]] || die "Origem e destino não podem ser iguais."

info "Origem : $R25_SRC"
info "Destino: $TARGET_SRC"

say "Validando ficheiros R25 antes de tocar no SourceCode"
missing=0

for rel in "${RUNTIME_FILES[@]}" "${TOOLS_FILES[@]}"; do
    if [[ ! -f "$R25_SRC/$rel" ]]; then
        printf 'FALTA: %s\n' "$R25_SRC/$rel" >&2
        missing=1
    fi
done

MIG_DIR="$R25_SRC/Database/migrations"
[[ -d "$MIG_DIR" ]] || {
    printf 'FALTA diretório: %s\n' "$MIG_DIR" >&2
    missing=1
}

for i in $(seq 1 32); do
    n="$(printf '%03d' "$i")"
    if ! compgen -G "$MIG_DIR/${n}_*.sql" >/dev/null; then
        printf 'FALTA migration %s_*.sql em %s\n' "$n" "$MIG_DIR" >&2
        missing=1
    fi
done

(( missing == 0 )) || die "Origem R25 incompleta. Nenhuma alteração foi feita."

migration_count="$(find "$MIG_DIR" -maxdepth 1 -type f -name '[0-9][0-9][0-9]_*.sql' | wc -l)"
[[ "$migration_count" -eq 32 ]] || die "Esperava 32 migrations SQL; encontrei $migration_count."

info "Runtime: ${#RUNTIME_FILES[@]} ficheiros"
info "Tools  : ${#TOOLS_FILES[@]} ficheiros"
info "SQL    : $migration_count migrations"

ALL_COPY_FILES=("${RUNTIME_FILES[@]}" "${TOOLS_FILES[@]}")
while IFS= read -r -d '' f; do
    rel="${f#"$R25_SRC/"}"
    ALL_COPY_FILES+=("$rel")
done < <(find "$MIG_DIR" -maxdepth 1 -type f \( -name '[0-9][0-9][0-9]_*.sql' -o -name 'README.md' \) -print0 | sort -z)

say "Preparando backup"
if (( DRY_RUN )); then
    info "Backup seria criado em: $BACKUP_DIR"
else
    mkdir -p -- "$BACKUP_FILES" "$BUILD_SNAPSHOT"
    : > "$NEW_FILES"
    printf 'action\trelative_path\tsha256_before\tsha256_after\n' > "$MANIFEST"
fi

backup_destination_file() {
    local rel="$1"
    local dst="$TARGET_SRC/$rel"

    if [[ -e "$dst" ]]; then
        if (( ! DRY_RUN )); then
            mkdir -p -- "$BACKUP_FILES/$(dirname "$rel")"
            cp -a -- "$dst" "$BACKUP_FILES/$rel"
        fi
    else
        if (( ! DRY_RUN )); then
            printf '%s\n' "$rel" >> "$NEW_FILES"
        fi
    fi
}

# Backup de tudo que será copiado/sobrescrito.
for rel in "${ALL_COPY_FILES[@]}"; do
    backup_destination_file "$rel"
done

# Snapshot adicional dos ficheiros de build/link para diagnóstico/rollback manual.
for rel in "${BUILD_METADATA[@]}"; do
    if [[ -e "$TARGET_SRC/$rel" ]]; then
        if (( DRY_RUN )); then
            info "Backup build metadata: $rel"
        else
            cp -a -- "$TARGET_SRC/$rel" "$BUILD_SNAPSHOT/$rel"
        fi
    fi
done

say "Copiando runtime R25, migrations e tools"
for rel in "${ALL_COPY_FILES[@]}"; do
    src="$R25_SRC/$rel"
    dst="$TARGET_SRC/$rel"

    if (( DRY_RUN )); then
        printf 'DRY-RUN: copiar %q -> %q\n' "$src" "$dst"
        continue
    fi

    before=""
    action="NEW"
    if [[ -e "$dst" ]]; then
        before="$(sha "$dst")"
        action="REPLACE"
    fi

    mkdir -p -- "$(dirname "$dst")"
    cp -a -- "$src" "$dst"

    after="$(sha "$dst")"
    source_hash="$(sha "$src")"
    [[ "$after" == "$source_hash" ]] || die "SHA-256 divergente após copiar $rel"

    printf '%s\t%s\t%s\t%s\n' "$action" "$rel" "$before" "$after" >> "$MANIFEST"
done

say "Verificando integração da unidade SQLiteConnection.cpp na build"

BUILD_SCRIPT="$TARGET_SRC/build_server_mingw_i686_j8.sh"
CPP_LIST="$TARGET_SRC/cpp_files_server_i686.txt"
build_mode="desconhecido"

if [[ -f "$BUILD_SCRIPT" ]]; then
    # Se a build varre SrcServer/src por *.cpp, o novo ficheiro já será descoberto.
    if grep -Eq 'find[[:space:]].*SrcServer(/src)?|SrcServer/src.*-name.*(\*\.cpp|\\\.cpp)' "$BUILD_SCRIPT"; then
        build_mode="auto-discovery"
        info "build_server_mingw_i686_j8.sh parece descobrir .cpp automaticamente; nenhum patch necessário."
    elif grep -q 'cpp_files_server_i686.txt' "$BUILD_SCRIPT" && [[ -f "$CPP_LIST" ]]; then
        build_mode="cpp-list"
        if grep -Fq 'SQLiteConnection.cpp' "$CPP_LIST"; then
            info "SQLiteConnection.cpp já consta em cpp_files_server_i686.txt."
        else
            info "Build usa cpp_files_server_i686.txt e SQLiteConnection.cpp não consta. Vou adicioná-lo."

            # Esse ficheiro pode já ter sido salvo no snapshot de build, mas como ele
            # será efetivamente alterado, também entra no backup restaurável normal.
            if (( ! DRY_RUN )); then
                if [[ ! -e "$BACKUP_FILES/cpp_files_server_i686.txt" ]]; then
                    cp -a -- "$CPP_LIST" "$BACKUP_FILES/cpp_files_server_i686.txt"
                fi

                sample="$(grep -m1 -v '^[[:space:]]*$' "$CPP_LIST" || true)"
                if [[ "$sample" == /* ]]; then
                    printf '%s\n' "$TARGET_SRC/$SQLITE_CPP_REL" >> "$CPP_LIST"
                elif [[ "$sample" == ./* ]]; then
                    printf './%s\n' "$SQLITE_CPP_REL" >> "$CPP_LIST"
                else
                    printf '%s\n' "$SQLITE_CPP_REL" >> "$CPP_LIST"
                fi

                printf 'BUILD_EDIT\t%s\t%s\t%s\n' \
                    "cpp_files_server_i686.txt" \
                    "$(sha "$BACKUP_FILES/cpp_files_server_i686.txt")" \
                    "$(sha "$CPP_LIST")" >> "$MANIFEST"
            else
                info "DRY-RUN: adicionaria $SQLITE_CPP_REL em cpp_files_server_i686.txt"
            fi
        fi
    else
        info "Não consegui provar que o script descobre .cpp automaticamente."
        info "Não vou editar build_server_mingw_i686_j8.sh às cegas."
        info "Confirma depois se ele compila: $SQLITE_CPP_REL"
    fi
else
    info "build_server_mingw_i686_j8.sh não encontrado; código foi integrado, mas a build deve ser revisada."
fi

say "Verificando linker"
LINK_SCRIPT="$TARGET_SRC/link_server_mingw_i686.sh"
if [[ -f "$LINK_SCRIPT" ]]; then
    if grep -Eq -- '(-lsqlite3|sqlite3\.(a|lib))' "$LINK_SCRIPT"; then
        info "AVISO: encontrei link estático/import lib de sqlite3 no linker."
        info "O R25 não precisa disso porque usa LoadLibraryA/GetProcAddress."
        info "Não removi automaticamente para não afetar outra parte do teu projeto."
    else
        info "OK: não é necessário adicionar -lsqlite3/sqlite3.lib."
    fi
else
    info "link_server_mingw_i686.sh não encontrado."
fi

if (( ! DRY_RUN )); then
    say "Gerando rollback automático"

    # Deduplica new_files preservando conteúdo simples.
    sort -u -o "$NEW_FILES" "$NEW_FILES"

    cat > "$BACKUP_DIR/rollback.sh" <<ROLLBACK
#!/usr/bin/env bash
set -Eeuo pipefail

TARGET_SRC=$(printf '%q' "$TARGET_SRC")
BACKUP_FILES=$(printf '%q' "$BACKUP_FILES")
NEW_FILES=$(printf '%q' "$NEW_FILES")

echo "Rollback SQLite R25"
echo "Destino: \$TARGET_SRC"
echo

if [[ ! -d "\$BACKUP_FILES" ]]; then
    echo "Backup não encontrado: \$BACKUP_FILES" >&2
    exit 1
fi

# Remove somente ficheiros que não existiam antes.
if [[ -f "\$NEW_FILES" ]]; then
    while IFS= read -r rel; do
        [[ -n "\$rel" ]] || continue
        rm -f -- "\$TARGET_SRC/\$rel"
    done < "\$NEW_FILES"
fi

# Restaura todos os ficheiros originais preservando estrutura.
while IFS= read -r -d '' src; do
    rel="\${src#"\$BACKUP_FILES/"}"
    dst="\$TARGET_SRC/\$rel"
    mkdir -p -- "\$(dirname "\$dst")"
    cp -a -- "\$src" "\$dst"
done < <(find "\$BACKUP_FILES" -type f -print0)

# Remove diretórios vazios criados pela integração, sem tocar em conteúdo alheio.
find "\$TARGET_SRC/SrcServer/src/systems/persistence/database/sqlite" -depth -type d -empty -delete 2>/dev/null || true
find "\$TARGET_SRC/Database/migrations" -depth -type d -empty -delete 2>/dev/null || true
find "\$TARGET_SRC/tools" -depth -type d -empty -delete 2>/dev/null || true

echo
echo "Rollback concluído."
ROLLBACK
    chmod +x "$BACKUP_DIR/rollback.sh"

    # Estado pós-cópia.
    {
        echo "R25 source integration"
        echo "timestamp=$timestamp"
        echo "source=$R25_SRC"
        echo "target=$TARGET_SRC"
        echo "backup=$BACKUP_DIR"
        echo "build_detection=$build_mode"
        echo "migrations=$migration_count"
        echo "sqlite_cpp=$SQLITE_CPP_REL"
    } > "$BACKUP_DIR/README_INTEGRATION.txt"

    say "Verificação final"
    errors=0

    for rel in "${ALL_COPY_FILES[@]}"; do
        if [[ ! -f "$TARGET_SRC/$rel" ]]; then
            echo "FALTA NO DESTINO: $rel" >&2
            errors=$((errors + 1))
            continue
        fi
        if [[ "$(sha "$R25_SRC/$rel")" != "$(sha "$TARGET_SRC/$rel")" ]]; then
            echo "SHA DIFERENTE: $rel" >&2
            errors=$((errors + 1))
        fi
    done

    current_migrations="$(find "$TARGET_SRC/Database/migrations" -maxdepth 1 -type f -name '[0-9][0-9][0-9]_*.sql' | wc -l)"
    if [[ "$current_migrations" -lt 32 ]]; then
        echo "Migrations insuficientes no destino: $current_migrations" >&2
        errors=$((errors + 1))
    fi

    (( errors == 0 )) || die "Integração copiada, mas a verificação final encontrou $errors erro(s). Use o rollback."

    info "Todos os ficheiros R25 copiados têm SHA-256 idêntico à origem."
    info "Migrations encontradas no destino: $current_migrations"
fi

say "Integração concluída"
if (( DRY_RUN )); then
    info "Nenhum ficheiro foi alterado (--dry-run)."
else
    info "Backup       : $BACKUP_DIR"
    info "Manifest     : $MANIFEST"
    info "Ficheiros novos: $NEW_FILES"
    info "Rollback     : $BACKUP_DIR/rollback.sh"
fi

cat <<'EOF'

Próximos passos recomendados:

  cd "/home/gabriel/Área de Trabalho/summit/SourceCode"

  # 1) Confirmar que a nova unidade entra na compilação:
  grep -n "SQLiteConnection.cpp" cpp_files_server_i686.txt 2>/dev/null || true

  # 2) Compilar:
  ./build_server_mingw_i686_j8.sh

  # 3) Se a compilação passar, linkar:
  ./link_server_mingw_i686.sh

Runtime:
  - NÃO é preciso -lsqlite3/sqlite3.lib.
  - É preciso sqlite3.dll x86 ao lado do Server.exe (ou disponível no PATH).
  - Database/migrations/001..032 deve acompanhar o diretório de execução.

Se precisares desfazer:
  executa o rollback.sh mostrado acima.

EOF
