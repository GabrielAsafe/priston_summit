# Pikeman SMD Material Audit V1

## Milestone

Both protected Pike textures are now runtime proven and restored:

```text
TmbC01.bmp = RUNTIME PROVEN
TmbC03.bmp = RUNTIME PROVEN
client restored to registered sources
```

Step 2A is closed.

## Step 2B

Audit the actual serialized `smMATERIAL_GROUP` in:

```text
TmbC01.smd
tmbC03.smd
```

The parser is read-only and uses the ABI already proven in the project:

```text
sizeof(smDFILE_HEADER)   = 556
sizeof(smMATERIAL_GROUP) = 88
sizeof(smMATERIAL)       = 320
```

Historic serialized pointers are exposed only diagnostically. Texture filenames
come exclusively from the serialized Name/NameA string block.

Run:

```bash
python3 pt_pikeman_smd_material_audit_v1.py \
  --smd TmbC01.smd \
  --smd tmbC03.smd \
  --report logs/pikeman_smd_material_audit_v1.json
```

Required final gate:

```text
material_region_end_check=PASS
material_audit_check=PASS
client_modified=False
```

Do not edit material bytes yet. First compare the real values for C01/C03/C06
materials and choose one scalar property for the first controlled runtime test.
