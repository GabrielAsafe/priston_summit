# Pikeman C004/C005 Model Resolution Audit V1

C004 is currently structurally inconsistent with `tmbC03.smd`:

```text
C004 DEFAULT requests:
TbmC03 TamC06 TsmC03 TlmC06

tmbC03.smd contains:
TbmC03 TamC06 TsmC06 TlmC06

missing:
TsmC03
```

The same C03-shoulder mismatch exists in HIGH/LOW:

```text
TshC03 missing
TsmC03 missing
TslC03 missing
```

Do not change the importer to accept missing nodes.

C004 has empty `szSubModelFile`, so the normal second-pattern route is not
available to explain the missing objects.

Run:

```bash
python3 pt_pikeman_c004_model_resolution_audit_v1.py \
  --client-root "/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"
```

The scanner reads C004/C005 directly and searches every `.smd` in the client for
their selected NodeNames.

Key outcomes:

1. Another SMD contains all C004 nodes:
   follow that model/file relationship before changing the Blender catalog.

2. C03 shoulder nodes exist only in a separate partial SMD:
   investigate whether another engine path merges models despite empty
   `szSubModelFile`.

3. No SMD contains `TshC03/TsmC03/TslC03`:
   C004 is stale/inconsistent data in the current client build.

C005 is audited at the same time and may be fully coherent even if C004 is not.
