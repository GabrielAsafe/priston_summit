#!/usr/bin/env bash
set -euo pipefail

ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
PRISTINE_SHA="9c5b137bc637f485419cafae6864c4a37f77835d32210810295c1b58c746443c"
CLIENT="$ROOT/../Summit Tale Client Full/cliente"
CLIENT_SMD="$CLIENT/char/tmABCD/TmbC01.smd"
ARTIST_DIR="$ROOT/blender/pikeman_c01_m4/artist"
FRESH="$ARTIST_DIR/C001_C002_pristine_auto_uv_g7_v1.blend"
EDITED="$ARTIST_DIR/C001_C002_pristine_auto_uv_g7_v1_edited.blend"
G7_MANIFEST="$ROOT/logs/characters/pikeman_c01_m4/geometry/character_asset_g7_v1/TmbC01_character_asset_g7_v1_build_manifest.json"
STAMP="$(date +%Y%m%d_%H%M%S)"
RESET_ARCHIVE="$ARTIST_DIR/_reset_archive_$STAMP"

cd "$ROOT"

echo "PT_PIKEMAN_RESET_PRISTINE_FULL_ASSET_ONE_COMMAND_V1_1"
echo "root=$ROOT"
echo "client=$CLIENT"
echo "mode=FULL_RESET_AUTO_UV_G7"

need() {
  [[ -e "$1" ]] || {
    echo "RESET_FAIL: missing required file: $1" >&2
    exit 2
  }
  echo "FILE_OK=$1"
}

for f in \
  "$ROOT/TmbC01.smd" \
  "$ROOT/m4.smb" \
  "$ROOT/C001.inx" \
  "$ROOT/C002.inx" \
  "$ROOT/M4Bip.inx" \
  "$ROOT/profiles/pt_character_profile_pikeman_c01_m4_v1.json" \
  "$ROOT/history_py/pt_smd_blender_import_v3_static_armature.py" \
  "$ROOT/pt_pikeman_character_asset_gate_g7_v1.py" \
  "$ROOT/pt_pikeman_character_asset_inspect_g7_v1.py" \
  "$ROOT/pt_character_multi_object_lod_pipeline_v2.py" \
  "$ROOT/pt_blender_to_smd_free_mesh_v4_multi.py" \
  "$ROOT/pt_free_mesh_validate_multi_v1.py" \
  "$ROOT/pt_free_mesh_validate_v2.py" \
  "$ROOT/pt_rigid_skin_normalize_copy_v2.py" \
  "$ROOT/pt_pikeman_smd_material_audit_v1.py" \
  "$ROOT/summit_workspace_asset_bridge_v1_2.py" \
  "$ROOT/summit_roundtrip_restore_v4.py" \
  "$ROOT/logs/characters/pikeman_c01_m4/materials/TmbC01_selfillum_restore_source_v1_build_manifest.json"
do
  need "$f"
done

ROOT_SHA="$(sha256sum "$ROOT/TmbC01.smd" | awk '{print $1}')"
[[ "$ROOT_SHA" == "$PRISTINE_SHA" ]] || {
  echo "RESET_FAIL: root TmbC01.smd is not pristine: $ROOT_SHA" >&2
  exit 3
}

echo
echo "=== 1. ARCHIVE/REMOVE ACTIVE USER EDITS ==="
mkdir -p "$RESET_ARCHIVE"

shopt -s nullglob
for p in \
  "$ARTIST_DIR"/C002_TbmC03_artist_master_g5_v1.blend* \
  "$ARTIST_DIR"/C002_TbmC03_artist_master_g6_all_lods_v1.blend* \
  "$ARTIST_DIR"/C001_C002_character_asset_master_g7_v1.blend* \
  "$ARTIST_DIR"/C001_C002_character_asset_g7_auto_witness_v1.blend* \
  "$ARTIST_DIR"/C001_character_asset_g7_v1_reimport_*.blend \
  "$ARTIST_DIR"/C002_character_asset_g7_v1_reimport_*.blend
do
  [[ -e "$p" ]] || continue
  echo "ARCHIVE_ACTIVE_EDIT=$p"
  mv "$p" "$RESET_ARCHIVE/"
done
shopt -u nullglob

if [[ -d "$ARTIST_DIR/_recovery_g6" ]]; then
  echo "ARCHIVE_ACTIVE_EDIT=$ARTIST_DIR/_recovery_g6"
  mv "$ARTIST_DIR/_recovery_g6" "$RESET_ARCHIVE/"
fi

# Remove any prior fresh-auto files from the active path, preserving them first.
for p in "$FRESH" "$EDITED"; do
  if [[ -e "$p" ]]; then
    echo "ARCHIVE_ACTIVE_EDIT=$p"
    mv "$p" "$RESET_ARCHIVE/"
  fi
done

echo "active_edit_archive=$RESET_ARCHIVE"

echo
echo "=== 2. ARCHIVE OLD G7 GENERATED OUTPUTS ==="
BUILD_ARCH="$ROOT/pt_build/characters/pikeman_c01_m4/_reset_archive_$STAMP"
LOG_ARCH="$ROOT/logs/characters/pikeman_c01_m4/_reset_archive_$STAMP"
BLEND_ARCH="$ROOT/blender/_reset_archive_$STAMP"
mkdir -p "$BUILD_ARCH" "$LOG_ARCH" "$BLEND_ARCH"

move_if_exists() {
  local src="$1"
  local dst="$2"
  if [[ -e "$src" ]]; then
    echo "ARCHIVE_GENERATED=$src"
    mv "$src" "$dst/"
  fi
}

move_if_exists \
  "$ROOT/pt_build/characters/pikeman_c01_m4/geometry/character_asset_g7_v1" \
  "$BUILD_ARCH"
move_if_exists \
  "$ROOT/pt_build/characters/pikeman_c01_m4/multi_TbhC03_TbmC03_TblC03_TblC01_TbmC01_TbhC01_TahC01_TamC01_TalC01" \
  "$BUILD_ARCH"

move_if_exists \
  "$ROOT/logs/characters/pikeman_c01_m4/geometry/character_asset_g7_v1" \
  "$LOG_ARCH"
move_if_exists \
  "$ROOT/logs/characters/pikeman_c01_m4/multi_TbhC03_TbmC03_TblC03_TblC01_TbmC01_TbhC01_TahC01_TamC01_TalC01" \
  "$LOG_ARCH"

move_if_exists \
  "$ROOT/blender/_pipeline_work/TmbC01_character_asset_g7_v1" \
  "$BLEND_ARCH"
move_if_exists \
  "$ROOT/blender/TmbC01_character_asset_g7_v1_game_normalized.blend" \
  "$BLEND_ARCH"
move_if_exists \
  "$ROOT/blender/TmbC01_character_asset_g7_v1_reimport.blend" \
  "$BLEND_ARCH"

echo
echo "=== 3. RESTORE CLIENT TO PRISTINE ==="
python3 "$ROOT/summit_workspace_asset_bridge_v1_2.py" publish-file \
  "$ROOT/blender_workspace_pikeman_real" \
  --build-manifest "$ROOT/logs/characters/pikeman_c01_m4/materials/TmbC01_selfillum_restore_source_v1_build_manifest.json" \
  --target char/tmABCD/TmbC01.smd \
  --replace-published

python3 "$ROOT/summit_roundtrip_restore_v4.py" \
  "$ROOT/blender_workspace_pikeman_real" \
  --target char/tmABCD/TmbC01.smd

python3 "$ROOT/summit_roundtrip_restore_v4.py" \
  "$ROOT/blender_workspace_pikeman_real" \
  --target char/tmABCD/TmbC01.smd \
  --apply

CLIENT_SHA="$(sha256sum "$CLIENT_SMD" | awk '{print $1}')"
echo "client_pristine_sha256=$CLIENT_SHA"
[[ "$CLIENT_SHA" == "$PRISTINE_SHA" ]] || {
  echo "RESET_FAIL: client did not restore to pristine" >&2
  exit 4
}
echo "client_pristine_check=PASS"

echo
echo "=== 4. FRESH IMPORT FROM PRISTINE SMD ==="
mkdir -p "$ARTIST_DIR"
flatpak run --filesystem=home org.blender.Blender \
  --background \
  --python-exit-code 1 \
  --python "$ROOT/history_py/pt_smd_blender_import_v3_static_armature.py" \
  -- \
  --smd "$ROOT/TmbC01.smd" \
  --skeleton "$ROOT/m4.smb" \
  --model-info "$ROOT/C002.inx" \
  --lod default \
  --client-root "$CLIENT" \
  --out "$FRESH" \
  --no-pack

need "$FRESH"

echo
echo "=== 5. AUTOMATIC SEAM-SAFE WITNESS: UV ONLY ==="
mkdir -p "$ROOT/sh"
TMP_PY="$(mktemp "$ROOT/sh/.pt_pikeman_auto_uv_g7_XXXXXX.py")"
trap 'rm -f "$TMP_PY"' EXIT

cat > "$TMP_PY" <<'PY'
import bpy, hashlib, json, pathlib, sys

TARGETS = (
    "TbhC03","TbmC03","TblC03",
    "TblC01","TbmC01","TbhC01",
    "TahC01","TamC01","TalC01",
)

def sha256(path):
    h=hashlib.sha256()
    with open(path,"rb") as f:
        for b in iter(lambda:f.read(1024*1024),b""):
            h.update(b)
    return h.hexdigest()

argv=sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
if len(argv)!=2:
    raise SystemExit("usage: -- <output.blend> <report.json>")
out=pathlib.Path(argv[0]).expanduser().resolve()
report=pathlib.Path(argv[1]).expanduser().resolve()

rows=[]
for name in TARGETS:
    obj=bpy.data.objects.get(name)
    if obj is None or obj.type!="MESH":
        raise RuntimeError(f"missing mesh: {name}")
    me=obj.data

    uv_names=[u.name for u in me.uv_layers]
    if uv_names != ["SMD_UV"]:
        raise RuntimeError(f"{name}: unexpected UV schema {uv_names}")
    layer=me.uv_layers.active
    if layer is None or layer.name!="SMD_UV":
        raise RuntimeError(f"{name}: SMD_UV is not active")

    pos_before=[tuple(float(c) for c in v.co) for v in me.vertices]
    topo_before=[(tuple(int(i) for i in p.vertices), int(p.material_index)) for p in me.polygons]
    weights_before=[
        sorted((int(g.group),float(g.weight)) for g in v.groups if g.weight>1e-12)
        for v in me.vertices
    ]
    groups_before=[g.name for g in obj.vertex_groups]

    changed=0
    max_delta=0.0
    for d in layer.data:
        ou,ov=float(d.uv[0]),float(d.uv[1])
        # Same affine mapping on every mesh. Geometry/skin/seams remain pristine.
        nu=ou*0.90+0.05
        nv=ov*0.90+0.05
        d.uv[0]=nu
        d.uv[1]=nv
        delta=((nu-ou)**2+(nv-ov)**2)**0.5
        if delta>1e-12:
            changed += 1
            max_delta=max(max_delta,delta)

    if [tuple(float(c) for c in v.co) for v in me.vertices] != pos_before:
        raise RuntimeError(f"{name}: vertex positions changed")
    if [(tuple(int(i) for i in p.vertices), int(p.material_index)) for p in me.polygons] != topo_before:
        raise RuntimeError(f"{name}: topology/material changed")
    if [g.name for g in obj.vertex_groups] != groups_before:
        raise RuntimeError(f"{name}: vertex-group schema changed")
    weights_after=[
        sorted((int(g.group),float(g.weight)) for g in v.groups if g.weight>1e-12)
        for v in me.vertices
    ]
    if weights_after != weights_before:
        raise RuntimeError(f"{name}: weights changed")
    if changed == 0:
        raise RuntimeError(f"{name}: UV witness made no change")

    rows.append({
        "object":name,
        "vertices":len(me.vertices),
        "faces":len(me.polygons),
        "uv_loops":len(layer.data),
        "changed_uv_loops":changed,
        "max_uv_delta":max_delta,
        "positions_changed":False,
        "topology_changed":False,
        "weights_changed":False,
        "uv_changed":True,
    })

out.parent.mkdir(parents=True,exist_ok=True)
bpy.ops.wm.save_as_mainfile(filepath=str(out))
data={
    "tool":"PT_PIKEMAN_PRISTINE_AUTO_UV_G7_V1",
    "provenance":"AUTOMATED_UV_AFFINE_WITNESS_FROM_PRISTINE",
    "targets":rows,
    "target_count":len(rows),
    "geometry_positions_pristine":True,
    "topology_pristine":True,
    "weights_pristine":True,
    "client_modified":False,
    "output":str(out),
    "output_sha256":sha256(out),
    "check":"PASS",
}
report.parent.mkdir(parents=True,exist_ok=True)
report.write_text(json.dumps(data,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")

print("PT_PIKEMAN_PRISTINE_AUTO_UV_G7_V1")
for row in rows:
    print(
      f"OBJECT {row['object']}: V={row['vertices']} F={row['faces']} "
      f"uv_loops={row['uv_loops']} changed_uv_loops={row['changed_uv_loops']} "
      f"max_uv_delta={row['max_uv_delta']:.9f} "
      "positions_changed=False topology_changed=False weights_changed=False uv_changed=True"
    )
print("target_count=9")
print("geometry_positions_pristine=True")
print("topology_pristine=True")
print("weights_pristine=True")
print(f"output={out}")
print(f"output_sha256={data['output_sha256']}")
print(f"report={report}")
print("client_modified=False")
print("auto_uv_witness_check=PASS")
PY

UV_REPORT="$ROOT/logs/characters/pikeman_c01_m4/geometry/pristine_auto_uv_g7_v1/uv_witness.json"
mkdir -p "$(dirname "$UV_REPORT")"

flatpak run --filesystem=home org.blender.Blender \
  --background "$FRESH" \
  --python-exit-code 1 \
  --python "$TMP_PY" \
  -- "$EDITED" "$UV_REPORT"

need "$EDITED"

echo
echo "=== 6. STRICT G7 INSPECT ==="
python3 "$ROOT/pt_pikeman_character_asset_gate_g7_v1.py" inspect \
  --root "$ROOT" \
  --artist "$EDITED"

echo
echo "=== 7. STRICT G7 BUILD: 9 OBJECTS / C001+C002 / ALL LODS ==="
python3 "$ROOT/pt_pikeman_character_asset_gate_g7_v1.py" build \
  --root "$ROOT" \
  --artist "$EDITED"

need "$G7_MANIFEST"

echo
echo "=== 8. FIX MANIFEST PROVENANCE (NO FALSE MANUAL CLAIM) ==="
python3 - "$G7_MANIFEST" <<'PY'
import json, pathlib, sys
p=pathlib.Path(sys.argv[1])
d=json.loads(p.read_text(encoding="utf-8"))
d["all_targets_manually_changed"]=False
d["authoring_provenance"]="AUTOMATED_UV_AFFINE_WITNESS_FROM_PRISTINE"
d["geometry_positions_pristine_before_writer_roundtrip"]=True
d["topology_pristine_before_writer_roundtrip"]=True
d["weights_pristine_before_writer_roundtrip"]=True
d["runtime_installed"]=False
p.write_text(json.dumps(d,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
print("manifest_provenance_patch=PASS")
print(f"manifest={p}")
PY

ARTIFACT_SHA="$(python3 - "$G7_MANIFEST" <<'PY'
import json,sys
print(json.load(open(sys.argv[1],encoding="utf-8"))["artifact_sha256"])
PY
)"
ARTIFACT_SIZE="$(python3 - "$G7_MANIFEST" <<'PY'
import json,sys
print(json.load(open(sys.argv[1],encoding="utf-8"))["artifact_size"])
PY
)"

echo "artifact_sha256=$ARTIFACT_SHA"
echo "artifact_size=$ARTIFACT_SIZE"

echo
echo "=== 9. PUBLISH + VERIFIED DEPLOY WITH BACKUP ==="
python3 "$ROOT/summit_workspace_asset_bridge_v1_2.py" publish-file \
  "$ROOT/blender_workspace_pikeman_real" \
  --build-manifest "$G7_MANIFEST" \
  --target char/tmABCD/TmbC01.smd \
  --replace-published

python3 "$ROOT/summit_roundtrip_restore_v4.py" \
  "$ROOT/blender_workspace_pikeman_real" \
  --target char/tmABCD/TmbC01.smd

python3 "$ROOT/summit_roundtrip_restore_v4.py" \
  "$ROOT/blender_workspace_pikeman_real" \
  --target char/tmABCD/TmbC01.smd \
  --apply

FINAL_SHA="$(sha256sum "$CLIENT_SMD" | awk '{print $1}')"
echo "client_final_sha256=$FINAL_SHA"
[[ "$FINAL_SHA" == "$ARTIFACT_SHA" ]] || {
  echo "RESET_FAIL: deployed client SHA != build artifact SHA" >&2
  exit 5
}

echo
echo "============================================================"
echo "PIKEMAN PRISTINE FULL-ASSET AUTO TEST = READY FOR RUNTIME"
echo "source_pristine_sha256=$PRISTINE_SHA"
echo "deployed_sha256=$FINAL_SHA"
echo "geometry_witness=UV_ONLY"
echo "positions_pristine=True"
echo "topology_pristine=True"
echo "weights_pristine=True"
echo "C001_high_default_low=STRUCTURAL_PASS"
echo "C002_high_default_low=STRUCTURAL_PASS"
echo "client_backup=CREATED_BY_ROUNDTRIP_V4"
echo "runtime_installed=True"
echo "flatpak_temp_helper_location=HOME_VISIBLE\none_command_check=PASS"
echo "============================================================"
