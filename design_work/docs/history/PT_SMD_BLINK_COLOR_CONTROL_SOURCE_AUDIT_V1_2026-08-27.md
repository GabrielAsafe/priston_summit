# PT SMD Blink Color Control Source Audit V1

Previous source audits have now proven the full color math:

```text
Diffuse.r/g/b
→ int(channel * 255)
→ triangular time modulation in SetBlinkColor
→ sColors RGB

SetBlinkColorVertex:
BlinkColor = sColors
BlinkColor += rv->bCol

if <=255:
    output Color = BlinkColor
    Specular = rv->bSpe

if >255:
    output Color = 255
    overflow is added to Specular
```

But the effect is conditional:

```text
UseState & sMATS_SCRIPT_BLINK_COLOR
```

and its speed/step is:

```text
WindMeshBottom
→ BlinkStep
→ BlinkTime = 1 << BlinkStep
```

The real Pike materials currently have:

```text
UseState=0
WindMeshBottom=0
```

so changing only `Diffuse` cannot produce this effect.

This audit closes the exact flag value and real BlinkStep assignments before a
composite Blender authoring gate is created.

Run:

```bash
python3 pt_smd_blink_color_control_source_audit_v1.py \
  --source-root "/home/gabriel/Área de Trabalho/summit" \
  --report logs/pt_smd_blink_color_control_source_audit_v1.json \
  > logs/pt_smd_blink_color_control_source_audit_v1.txt

cat logs/pt_smd_blink_color_control_source_audit_v1.txt
```

Read-only: no source or client changes.
