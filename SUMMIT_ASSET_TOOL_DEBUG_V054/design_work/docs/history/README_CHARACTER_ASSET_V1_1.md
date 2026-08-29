# PT Character Asset V1.1

V1.1 closes the Character Asset abstraction.

## High-level commands

```text
inspect
sync-workspace
build
publish-build
status
deploy-plan
deploy
```

The artist no longer needs to know SMD/SMB/INX paths or internal LOD object names.

## Publish an already approved build

This does **not** run Blender again.

By build name:

```bash
python3 pt_character_asset_v1_1.py publish-build \
  --workspace ./blender_workspace_pikeman \
  --name pikeman_c01_character_asset_gate_v1 \
  --replace-published
```

Or by exact pipeline manifest:

```bash
python3 pt_character_asset_v1_1.py publish-build \
  --workspace ./blender_workspace_pikeman \
  --pipeline-manifest logs/characters/pikeman_c01_m5/all_lods/BUILD_pipeline_manifest.json \
  --replace-published
```

The command revalidates:

```text
pipeline_check=PASS
profile_id match
runtime_installed=false
SMD exists
SMD SHA matches manifest
SMD size matches manifest
```

Then delegates publication to the verified Bridge.

## Character status

```bash
python3 pt_character_asset_v1_1.py status \
  --workspace ./blender_workspace_pikeman
```

Important states:

```text
workspace_state=REGISTERED_SOURCE
workspace_state=CURRENT_PUBLISHED_BUILD
workspace_state=WORKSPACE_DRIFT

client_state=REGISTERED_SOURCE
client_state=CURRENT_PUBLISHED_BUILD
client_state=PREVIOUSLY_PUBLISHED_BUILD
client_state=CLIENT_DRIFT
```

Any drift causes the status gate to fail.

## Production rule

`--force-test` remains engineering-only. Production keeps `PT_REVIEW_REQUIRED`.
