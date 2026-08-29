# PT SMD Diffuse Render Source Audit V1

Read-only source audit.

Purpose:

```text
smMATERIAL::Diffuse
→ exact smFCOLOR definition
→ import/write path
→ runtime read/use path
→ character vertex/pixel color path
```

This gate exists because the ABI proves:

```text
smMATERIAL::Diffuse offset = 132
sizeof(smFCOLOR) = 12
```

but the exact member definition and a runtime renderer use have not yet been
closed as production facts.

Run:

```bash
python3 pt_smd_diffuse_render_source_audit_v1.py \
  --source-root "/home/gabriel/Área de Trabalho/summit" \
  --report logs/pt_smd_diffuse_render_source_audit_v1.json \
  > logs/pt_smd_diffuse_render_source_audit_v1.txt
```

The default scanner skips Dependency/vendor/build trees to avoid DirectX SDK
noise. It does not modify source or client files.

Interpretation gate:

1. `smFCOLOR` declaration must identify exact members/types.
2. `Diffuse` member accesses are conservatively classified as:
   - `WRITE_LHS`
   - `READ_OR_USE`
3. A Blender-editable Diffuse control is allowed only if source evidence links
   `smMATERIAL::Diffuse` to the character runtime color path.
4. If all material accesses are writes/copies only, Diffuse remains preserved
   metadata and no runtime patch is created.
