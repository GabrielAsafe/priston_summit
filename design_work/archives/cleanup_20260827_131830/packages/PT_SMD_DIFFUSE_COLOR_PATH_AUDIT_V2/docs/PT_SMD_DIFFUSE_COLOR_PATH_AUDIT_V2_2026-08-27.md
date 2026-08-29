# PT SMD Diffuse Color Path Audit V2

The V1 audit proved:

```text
smFCOLOR = { float r, g, b; }

ASE MATERIAL_DIFFUSE
→ aseMaterial.Diffuse
→ smMaterial.Diffuse

runtime read:
SetBlinkColor(
    sColors,
    smRendMaterial[MatNum].Diffuse,
    smRendMaterial[MatNum].WindMeshBottom
)
```

V2 is read-only and closes the remaining link:

```text
Diffuse
→ SetBlinkColor implementation
→ sColors mutation
→ render vertex/D3D diffuse color
```

Run:

```bash
python3 pt_smd_diffuse_color_path_audit_v2.py \
  --source-root "/home/gabriel/Área de Trabalho/summit" \
  --report logs/pt_smd_diffuse_color_path_audit_v2.json \
  > logs/pt_smd_diffuse_color_path_audit_v2.txt

cat logs/pt_smd_diffuse_color_path_audit_v2.txt
```

No SMD or client file is modified.
