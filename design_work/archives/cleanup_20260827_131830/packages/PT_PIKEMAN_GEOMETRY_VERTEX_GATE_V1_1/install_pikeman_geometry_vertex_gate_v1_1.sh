#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PKG="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
mkdir -p "$ROOT/docs"
install -m 0644 "$PKG/pt_pikeman_geometry_gate_edit_v1_1.py" "$ROOT/pt_pikeman_geometry_gate_edit_v1_1.py"
install -m 0644 "$PKG/pt_pikeman_geometry_vertex_build_v1_1.py" "$ROOT/pt_pikeman_geometry_vertex_build_v1_1.py"
install -m 0644 "$PKG/pt_pikeman_geometry_reimport_compare_v1_1.py" "$ROOT/pt_pikeman_geometry_reimport_compare_v1_1.py"
install -m 0644 "$PKG/docs/PT_PIKEMAN_GEOMETRY_VERTEX_GATE_V1_1_2026-08-27.md" "$ROOT/docs/PT_PIKEMAN_GEOMETRY_VERTEX_GATE_V1_1_2026-08-27.md"
echo "PT_PIKEMAN_GEOMETRY_VERTEX_GATE_V1_1_INSTALL"
echo "ROOT=$ROOT"
echo "installed=4"
echo "client_modified=False"
echo "install_check=PASS"
