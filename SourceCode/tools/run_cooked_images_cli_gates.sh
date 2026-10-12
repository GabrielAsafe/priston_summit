#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
python3 -m py_compile tools/cook_textures_dds.py tools/test_cooked_image_contract_v1.py
python3 tools/test_cooked_image_contract_v1.py
if command -v "${CXX:-g++}" >/dev/null 2>&1; then
  temp_bin="$(mktemp "${TMPDIR:-/tmp}/summit-cooked-identity.XXXXXX")"
  trap 'rm -f "$temp_bin"' EXIT
  "${CXX:-g++}" -std=c++11 -Wall -Wextra -Werror -I. tools/tests/cooked_texture_identity_smoke.cpp -o "$temp_bin"
  "$temp_bin"
else
  echo "[WARN] C++ compiler unavailable; C++ contract not validated" >&2
  exit 2
fi
python3 - <<'PY'
from pathlib import Path
cpp = Path('Shared/rendering/legacy_3d/material/smTexture_legacy_client.inl').read_text()
assert 'summit::rendering::cooked::BuildRelativePath(logicalPath, profile)' in cpp
assert 'summit::rendering::cooked::Material' in cpp
assert 'summit::rendering::cooked::Dib' in cpp
assert 'DIB_COOKED_DDS_HIT' in cpp
assert 'DIB_SOURCE_FALLBACK' in cpp
assert 'SummitLoadDibSourcePathWithInfo(Filename, pSourceWidth, pSourceHeight)' in cpp
assert 'imageInfo.ImageFileFormat != D3DXIFF_DDS' in cpp
assert 'relative.replace(dot, std::string::npos, ".dds")' not in cpp
print('SUMMIT_COOKED_RUNTIME_STRUCTURE=PASS')
PY
printf 'SUMMIT_COOKED_CLI_GATES=PASS\n'
