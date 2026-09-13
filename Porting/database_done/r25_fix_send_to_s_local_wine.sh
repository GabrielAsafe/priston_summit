#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="/home/gabriel/Área de Trabalho/summit"
SOURCE="$ROOT/SourceCode"
TARGET="$SOURCE/send_to_s.sh"
BACKUP="$TARGET.before_local_wine_$(date +%Y%m%d_%H%M%S)"

[[ -f "$TARGET" ]] || {
    echo "[ERRO] não encontrei: $TARGET" >&2
    exit 1
}

cp -av "$TARGET" "$BACKUP"

cat > "$TARGET" <<'EOF'
#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="/home/gabriel/Área de Trabalho/summit"
SOURCE="$ROOT/SourceCode"
DIST="$SOURCE/dist_mingw_server_i686"
DEST="$ROOT/Servidor_R25_Wine"

die() {
    echo "[ERRO] $*" >&2
    exit 1
}

ok() {
    echo "[OK] $*"
}

echo "============================================================"
echo "SUMMIT - DEPLOY SERVER LOCAL LINUX/WINE"
echo "============================================================"
echo
echo "Origem : $DIST"
echo "Destino: $DEST"
echo

[[ -d "$DIST" ]] || die "dist não encontrado: $DIST"
[[ -d "$DEST" ]] || die "runtime não encontrado: $DEST"
[[ -f "$DIST/Server.exe" ]] || die "Server.exe não encontrado em $DIST"

if pgrep -af '[S]erver\.exe' >/dev/null 2>&1; then
    echo "[ERRO] Há Server.exe em execução."
    echo "Pare o servidor antes do deploy para não confundir binário antigo em memória com binário novo."
    pgrep -af '[S]erver\.exe' || true
    exit 2
fi

echo "== 1. Atualizando DLLs MinGW no dist =="

for dll in \
    libgcc_s_dw2-1.dll \
    libstdc++-6.dll \
    libwinpthread-1.dll
do
    src="/usr/i686-w64-mingw32/sys-root/mingw/bin/$dll"

    if [[ -f "$src" ]]; then
        cp -av "$src" "$DIST/$dll"
    else
        echo "[AVISO] não encontrado no toolchain: $src"
    fi
done

echo
echo "== 2. Backup do Server.exe atual =="

if [[ -f "$DEST/Server.exe" ]]; then
    backup="$DEST/Server.exe.before_send_to_s_$(date +%Y%m%d_%H%M%S)"
    cp -av "$DEST/Server.exe" "$backup"
    sha256sum "$backup"
fi

echo
echo "== 3. Deploy do servidor =="

cp -av "$DIST/Server.exe" "$DEST/Server.exe"

for dll in \
    libgcc_s_dw2-1.dll \
    libstdc++-6.dll \
    libwinpthread-1.dll
do
    if [[ -f "$DIST/$dll" ]]; then
        cp -av "$DIST/$dll" "$DEST/$dll"
    fi
done

# sqlite3.dll faz parte do runtime SQLite atual. Atualiza somente se o build/dist tiver uma cópia.
if [[ -f "$DIST/sqlite3.dll" ]]; then
    cp -av "$DIST/sqlite3.dll" "$DEST/sqlite3.dll"
fi

echo
echo "== 4. hotuk.ini =="

HOTUK="$ROOT/Servidor/Configs/hotuk.ini"
HOTUK_DEST="$DEST/Configs/hotuk.ini"

if [[ -f "$HOTUK" ]]; then
    mkdir -p "$DEST/Configs"
    cp -av "$HOTUK" "$HOTUK_DEST"
    ok "hotuk.ini"
else
    echo "[AVISO] hotuk.ini original não encontrado: $HOTUK"
fi

echo
echo "== 5. Guardas SQLite/local-only =="

[[ ! -e "$DEST/Database.dll" ]] \
    || die "Database.dll ATIVA encontrada no runtime"

[[ -f "$DEST/Database.dll.R25_DISABLED" ]] \
    || echo "[AVISO] Database.dll.R25_DISABLED ausente"

[[ -f "$DEST/sqlite3.dll" ]] \
    || die "sqlite3.dll ausente após deploy"

echo
echo "== 6. Verificação final =="

src_sha="$(sha256sum "$DIST/Server.exe" | awk '{print $1}')"
dst_sha="$(sha256sum "$DEST/Server.exe" | awk '{print $1}')"

echo "DIST   : $src_sha"
echo "RUNTIME: $dst_sha"

[[ "$src_sha" == "$dst_sha" ]] \
    || die "SHA256 do Server.exe diverge entre dist e runtime"

file "$DEST/Server.exe"
file "$DEST/sqlite3.dll"

ok "Server.exe copiado para runtime atual"
ok "SHA256 confirmado"

echo
echo "============================================================"
echo "SEND_TO_S LOCAL: PASS"
echo "============================================================"
echo
echo "Nenhum processo foi iniciado."
EOF

chmod +x "$TARGET"

echo
echo "============================================================"
echo "PATCH SEND_TO_S: PASS"
echo "============================================================"
echo
echo "Backup:"
echo "  $BACKUP"
echo
echo "Novo destino:"
echo "  $ROOT/Servidor_R25_Wine"
echo
echo "Conteúdo novo:"
sed -n '1,240p' "$TARGET"
