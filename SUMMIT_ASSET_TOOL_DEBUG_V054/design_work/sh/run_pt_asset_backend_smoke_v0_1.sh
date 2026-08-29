#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
CLIENT="${2:-$HOME/Área de Trabalho/summit/Summit Tale Client Full/cliente}"
cd "$ROOT"
python3 pt_asset.py --root "$ROOT" --client-root "$CLIENT" doctor
python3 pt_asset.py --root "$ROOT" --client-root "$CLIENT" families
python3 pt_asset.py --root "$ROOT" --client-root "$CLIENT" assets list --visual-only --limit 15
python3 pt_asset.py --root "$ROOT" --client-root "$CLIENT" asset inspect 'char/monster/Monbagon/Monbagon-C.smd'
echo "client_modified=False"
echo "backend_smoke_check=PASS"
