# Summit Tale Asset Tool V0.5.5 — Rigid LOD Runtime Name Semantics

V0.5.5 is a narrow hotfix on top of V0.5.4. It does not change TexLink material semantics or enable roundtrip/write-back.

## Bug fixed: RIGID INX LOD selection

`history_py/pt_smd_blender_import_v3_static_armature.py` previously compared INX model-group names directly against Blender object datablock names, case-sensitively.

That differs from the supplied client SourceCode. `smPAT3D::ChoiseRenderD3D()` compares every INX `szModelName[]` against every serialized SMD `NodeName` using `lstrcmpi()`, so matching is case-insensitive and all SMD chunks sharing the selected logical NodeName are rendered.

Bguardian proves both differences matter:

- INX default: `Bguardian_m`
- serialized SMD logical name: `BGuardian_m`
- three SMD chunks share the logical name `BGuardian_m`
- Blender makes duplicate datablock names unique as `BGuardian_m`, `BGuardian_m.001`, `BGuardian_m.002`

V0.5.5 now stores the serialized logical name in `pt_smd_node_name` and performs LOD selection using `casefold()` on that logical name. A genuinely missing INX node still fails closed.

The selected Blender meshes record:

```text
pt_inx_match_semantics=LSTRCMPI_SMD_NODENAME
```

## Corpus audit diagnostic fix

V0.5.4 counted every `source_semantics=FAIL` file as `htexture_mapping_fail_files`, even when its `hTexture` mapping was exact and the actual error was a `stage_index_error`.

V0.5.5 separates:

- `htexture_mapping_fail_files`
- `stage_index_fail_files`
- `source_semantics_fail_files`

and records error counts/samples per row. Semantic failures are always included in the generated semantic representative bundle, so the physical failing file can be inspected directly.

This is diagnostic only: V0.5.5 does **not** weaken or reinterpret `stage_index_errors`.

## Install

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
unzip -o asset_v0_5_5_rigid_lod_runtime_names.zip
./asset_v0_5_5_rigid_lod_runtime_names/install.sh \
  "/home/gabriel/Área de Trabalho/summit/design_work"
```

The installer backs up both the current `pt_asset_tool` directory and the historical V3 Blender importer before replacing them.

## First gate

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py doctor

python3 pt_asset.py validate texlinks \
  char/monster/Bguardian/Bguardian.smd
```

If Bguardian passes, regress with:

```bash
python3 pt_asset.py validate texlinks \
  char/monster/Morgon/Morgon.smd

python3 pt_asset.py validate texlinks \
  Field/Fall_Game/chandelier-ani01.smd
```

## Re-run semantic corpus audit

```bash
python3 pt_asset.py audit texlink-semantics \
  --output pt_build/multilayer_texlink_semantics_audit_v055.json \
  --bundle PT_MULTILAYER_TEXLINK_SEMANTIC_REPS_V055.zip
```

The new bundle will include any `source_semantics=FAIL` file (currently expected to expose the `Field/dungeon/dun-4-ani-33.smd` stage-index issue) instead of omitting it from profile-only representative selection.
