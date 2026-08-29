# SMD Material Render Source Audit V1

The binary ABI is already proven:

```text
smMATERIAL size = 320
SelfIllum offset = 148
Transparency offset = 144
BlendType offset = 116
TwoSide offset = 124
```

But binary position alone is not enough to choose a safe runtime edit.

This audit finds:
- exact source declarations/types;
- enum/constant values;
- renderer branches using these fields;
- Direct3D render-state calls affected by them.

Run:

```bash
python3 pt_smd_material_render_source_audit_v1.py \
  --source-root "/home/gabriel/Área de Trabalho/summit" \
  --report logs/pt_smd_material_render_source_audit_v1.json
```

Do not patch SMD material bytes until this source audit establishes the
semantics/value domain of the chosen field.
