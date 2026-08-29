# Pikeman Real Geometry — Vertex Gate V1

Target:

```text
profile: pikeman_c01_m4
SMD:     TmbC01.smd
SMB:     m4.smb
INX:     C002.inx
object:  TbmC03
```

This is deliberately NOT a free-topology gate yet.

It changes only the bind-space vertex positions of `TbmC03`:

```text
X/Y scale = 1.20
around the source bind-space centroid
Z unchanged
```

Everything else remains source bytes:

```text
vertex count = 74
face count   = 122
texlinks     = 122
topology     = source unchanged
UV           = source unchanged
materials    = source unchanged
Physique names = source unchanged
non-target object bytes = unchanged
file size    = unchanged
```

The exporter requires one rigid skeleton group per Blender vertex and computes:

```text
edited bind world
→ inverse(m4 bone.Tm)
→ quantized SMD bone-local XYZ
```

It then forward-reconstructs each patched vertex and rejects fit error > 0.01
Blender units.

## 1. Create the controlled artist blend

```bash
flatpak run --filesystem=home org.blender.Blender \
  --background blender/pikeman_c01_m4/armor_family/C002_pikeman_armor_preview.blend \
  --python-exit-code 1 \
  --python pt_pikeman_geometry_gate_edit_v1.py -- \
  blender/pikeman_c01_m4/geometry/C002_TbmC03_vertex_gate_v1.blend
```

Required:

```text
object=TbmC03
vertices=74
faces=122
scale_xy=1.2
moved_vertices>0
topology_changed=False
uv_changed=False
material_changed=False
physique_changed=False
geometry_gate_edit_check=PASS
```

## 2. Build direct SMD from the Blender geometry

```bash
flatpak run --filesystem=home org.blender.Blender \
  --background blender/pikeman_c01_m4/geometry/C002_TbmC03_vertex_gate_v1.blend \
  --python-exit-code 1 \
  --python pt_pikeman_geometry_vertex_build_v1.py -- \
  TmbC01.smd \
  m4.smb \
  pt_build/characters/pikeman_c01_m4/geometry/TmbC01_TbmC03_vertex_gate_v1.smd \
  logs/characters/pikeman_c01_m4/geometry/TmbC01_TbmC03_vertex_gate_v1_build_manifest.json
```

Required:

```text
source_counts=74/122/122
output_counts=74/122/122
moved_vertices=74 (normally; centroid-coincident vertices may reduce this)
outside_target_vertex_xyz_changed_byte_count=0
topology_serialized_from_source_unchanged=True
uv_serialized_from_source_unchanged=True
materials_serialized_from_source_unchanged=True
file_size_preserved=True
runtime_installed=False
build_check=PASS
```

Do NOT deploy before independent V3 reimport.

## 3. Independent V3 reimport

```bash
flatpak run --filesystem=home org.blender.Blender \
  --background \
  --python-exit-code 1 \
  --python history_py/pt_smd_blender_import_v3_static_armature.py -- \
  --smd pt_build/characters/pikeman_c01_m4/geometry/TmbC01_TbmC03_vertex_gate_v1.smd \
  --skeleton m4.smb \
  --model-info C002.inx \
  --lod default \
  --client-root "/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente" \
  --out blender/pikeman_c01_m4/geometry/C002_TbmC03_vertex_gate_v1_reimport.blend \
  --no-pack
```

Structural expectations remain:

```text
objects=9
materials=2
skeleton objects=58
bind vertices=955
bind bones used=32
visible_objects=['TbmC03','TamC01']
static_armature_import_check=PASS
```

Only after PASS should the artifact be published/deployed through the managed
workspace bridge and Restore V4.
