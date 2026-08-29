#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
SRC="$ROOT/blender/pikeman_c01_m4/artist/C002_TbmC03_artist_master_g5_v1.blend"
DST="$ROOT/blender/pikeman_c01_m4/artist/C002_TbmC03_artist_master_g6_all_lods_v1.blend"

if [[ ! -f "$SRC" ]]; then
  echo "prepare_check=FAIL: missing G5 artist master: $SRC" >&2
  exit 2
fi
if [[ -e "$DST" ]]; then
  echo "prepare_check=FAIL: destination already exists: $DST" >&2
  exit 3
fi

mkdir -p "$(dirname "$DST")"
cp -a "$SRC" "$DST"

SRC_SHA="$(sha256sum "$SRC" | awk '{print $1}')"
DST_SHA="$(sha256sum "$DST" | awk '{print $1}')"
[[ "$SRC_SHA" == "$DST_SHA" ]] || {
  echo "prepare_check=FAIL: copy SHA mismatch" >&2
  exit 4
}

echo "PT_PIKEMAN_MANUAL_ALL_LOD_PREPARE_G6_V1"
echo "source=$SRC"
echo "destination=$DST"
echo "source_sha256=$SRC_SHA"
echo "destination_sha256=$DST_SHA"
echo "client_modified=False"
echo "prepare_check=PASS"
