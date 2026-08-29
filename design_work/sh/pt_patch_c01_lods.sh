#!/usr/bin/env bash
set -euo pipefail

# Conservative 3-LOD wrapper around the already-proven
# pt_blender_patch_smd_vertices_v1.py.
#
# It patches, in order:
#   MshC01 = HIGH torso
#   MsmC01 = DEFAULT torso
#   MslC01 = LOW torso
#
# Unchanged objects are harmless: their stage will report changed_vertices=0.
# Topology changes are still rejected by the V1 patcher.

BLEND="${1:-MfbC01_direct_smd_v3_static_armature_testv2.blend}"
OUT="${2:-MfbC01_testv2_lods.smd}"

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

PATCHER="$ROOT/pt_blender_patch_smd_vertices_v1.py"
ORIGINAL="$ROOT/MfbC01.smd"
SKELETON="$ROOT/m5.smb"

[[ -f "$BLEND" ]] || { echo "ERROR: blend not found: $BLEND" >&2; exit 1; }
[[ -f "$PATCHER" ]] || { echo "ERROR: patcher not found: $PATCHER" >&2; exit 1; }
[[ -f "$ORIGINAL" ]] || { echo "ERROR: original SMD not found: $ORIGINAL" >&2; exit 1; }
[[ -f "$SKELETON" ]] || { echo "ERROR: skeleton not found: $SKELETON" >&2; exit 1; }

TMPDIR_LOCAL="$(mktemp -d "$ROOT/.pt_lod_patch.XXXXXX")"
trap 'rm -rf -- "$TMPDIR_LOCAL"' EXIT

STEP1="$TMPDIR_LOCAL/01_high.smd"
STEP2="$TMPDIR_LOCAL/02_default.smd"

echo "=== HIGH / MshC01 ==="
flatpak run --filesystem=home org.blender.Blender \
  --background "$BLEND" \
  --python "$PATCHER" -- \
  --smd "$ORIGINAL" \
  --skeleton "$SKELETON" \
  --object MshC01 \
  --out "$STEP1" \
  --report "$ROOT/MfbC01_testv2_MshC01_report.json"

echo
echo "=== DEFAULT / MsmC01 ==="
flatpak run --filesystem=home org.blender.Blender \
  --background "$BLEND" \
  --python "$PATCHER" -- \
  --smd "$STEP1" \
  --skeleton "$SKELETON" \
  --object MsmC01 \
  --out "$STEP2" \
  --report "$ROOT/MfbC01_testv2_MsmC01_report.json"

echo
echo "=== LOW / MslC01 ==="
flatpak run --filesystem=home org.blender.Blender \
  --background "$BLEND" \
  --python "$PATCHER" -- \
  --smd "$STEP2" \
  --skeleton "$SKELETON" \
  --object MslC01 \
  --out "$ROOT/$OUT" \
  --report "$ROOT/MfbC01_testv2_MslC01_report.json"

echo
echo "=== FINAL ==="
printf 'output=%s\n' "$ROOT/$OUT"
printf 'size=%s\n' "$(stat -c %s "$ROOT/$OUT")"
printf 'sha256=%s\n' "$(sha256sum -- "$ROOT/$OUT" | awk '{print $1}')"
printf 'original_sha256=%s\n' "$(sha256sum -- "$ORIGINAL" | awk '{print $1}')"

if [[ "$(stat -c %s "$ROOT/$OUT")" != "$(stat -c %s "$ORIGINAL")" ]]; then
  echo "FINAL_CHECK=FAIL: size changed" >&2
  exit 1
fi

if cmp -s -- "$ROOT/$OUT" "$ORIGINAL"; then
  echo "FINAL_CHECK=FAIL: final SMD is byte-identical to original; no LOD edit was exported" >&2
  exit 1
fi

echo "FINAL_CHECK=PASS"
