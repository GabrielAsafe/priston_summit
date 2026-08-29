#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
[[ -d "$ROOT" ]] || { echo "ROOT_NOT_FOUND=$ROOT" >&2; exit 2; }
python3 - "$ROOT" "$PKG/core_requirements.json" <<'PYVERIFY'
import hashlib,json,pathlib,sys
root=pathlib.Path(sys.argv[1]).resolve(); req=json.loads(pathlib.Path(sys.argv[2]).read_text())
errors=[]
for row in req["files"]:
    p=root/row["path"]
    if not p.is_file():
        errors.append(f"MISSING {row['path']}"); continue
    h=hashlib.sha256(p.read_bytes()).hexdigest()
    if h != row["sha256"]:
        errors.append(f"SHA_MISMATCH {row['path']} expected={row['sha256']} actual={h}")
if errors:
    print("core_requirements_check=FAIL")
    print("\n".join(errors))
    raise SystemExit(3)
print("core_requirements_check=PASS")
PYVERIFY
mkdir -p "$ROOT/pt_asset_tool" "$ROOT/certifications" "$ROOT/pt_build/pt_asset_tool" "$ROOT/logs/pt_asset_tool"
cp -a "$PKG/pt_asset_tool/." "$ROOT/pt_asset_tool/"
install -m 0755 "$PKG/pt_asset.py" "$ROOT/pt_asset.py"
mkdir -p "$ROOT/sh"
install -m 0755 "$PKG/sh/run_pt_asset_backend_smoke_v0_1.sh" "$ROOT/sh/run_pt_asset_backend_smoke_v0_1.sh"
install -m 0644 "$PKG/README.md" "$ROOT/docs/PT_ASSET_TOOL_BACKEND_V0_1.md"
echo "PT_ASSET_TOOL_BACKEND_V0_1_INSTALL"
echo "root=$ROOT"
echo "client_modified=False"
echo "install_check=PASS"
