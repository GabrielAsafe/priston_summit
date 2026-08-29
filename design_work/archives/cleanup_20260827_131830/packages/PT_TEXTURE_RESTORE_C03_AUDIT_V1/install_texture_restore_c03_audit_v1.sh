#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

mkdir -p "$ROOT/history_py" "$ROOT/docs"

for f in \
  summit_roundtrip_restore_v3.py \
  pt_character_texture_v1_1.py \
  pt_character_asset_v1_3.py
do
  if [[ -f "$ROOT/$f" ]]; then
    cp -a "$ROOT/$f" "$ROOT/history_py/$f" 2>/dev/null || true
  fi
done

install -m 0755 "$PKG/summit_roundtrip_restore_v4.py" \
  "$ROOT/summit_roundtrip_restore_v4.py"
install -m 0755 "$PKG/pt_character_texture_v1_2.py" \
  "$ROOT/pt_character_texture_v1_2.py"
install -m 0755 "$PKG/pt_character_asset_v1_4.py" \
  "$ROOT/pt_character_asset_v1_4.py"
install -m 0755 "$PKG/pt_pikeman_c03_inx_selector_audit_v1.py" \
  "$ROOT/pt_pikeman_c03_inx_selector_audit_v1.py"
install -m 0644 "$PKG/docs/RESTORE_V4_AND_C03_AUDIT_2026-08-27.md" \
  "$ROOT/docs/RESTORE_V4_AND_C03_AUDIT_2026-08-27.md"

echo "PT_TEXTURE_RESTORE_C03_AUDIT_V1_INSTALL"
echo "ROOT=$ROOT"
echo "installed=5"
echo "client_modified=False"
echo "install_check=PASS"
