# PT All-LODs Character Core V1

Adds `--all-lods` to the multi-object character pipeline.

For `pikeman_c01_m5`, this selects:

```text
HIGH:    MbhC01 MshC01 MlhC01
DEFAULT: MbmC01 MsmC01 MlmC01
LOW:     MblC01 MslC01 MllC01
```

All nine blocks are rebuilt from the same pristine SMD in one final file.
The remaining six C03 blocks stay byte-identical.

The final SMD is independently reimported three times:

```text
high
default
low
```

## Gate command

```bash
python3 pt_character_multi_object_lod_pipeline_v2.py \
  --artist blender/MfbC01_artist_free_mesh_v1.blend \
  --all-lods \
  --name MfbC01_all_lods_artist_gate_v1 \
  --force-test
```

`--force-test` is engineering-only. Production still stops with
`PT_REVIEW_REQUIRED` when rigid-skin ambiguities remain.
