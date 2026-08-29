# Pikeman C03 Reference Audit V1

## Current proven state

Restore V4 is now proven in the real client:

```text
workspace source + client previous managed build
→ candidate_count=1
→ verified backup
→ verified write
→ client REGISTERED_SOURCE
```

`TmbC03.bmp` technical texture pipeline:

```text
inspect PASS
prepare PASS
noop byte-identical PASS
Blender binding PASS
visible red test BMP creation PASS
runtime deploy NOT YET PERFORMED
```

INX selector audit:

```text
INX scanned = 2046
valid model-info = 2046
TbhC03/TbmC03/TblC03 selector matches = 0
```

Therefore no known INX High/Default/Low group selects these C03 nodes.

## Next gate

Search the entire client bytewise for:

```text
TbhC03
TbmC03
TblC03
TmbC03.bmp
```

Run:

```bash
python3 pt_pikeman_c03_reference_audit_v1.py \
  --client-root "/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente" \
  --report logs/pikeman_c03_reference_audit_v1.json
```

Interpretation:

### `node_reference_status=SMD_EMBEDDED_ONLY`

No external file references the C03 NodeNames. Combined with zero INX model-group
matches, this is strong evidence that the nodes are embedded but unselected in
the current client configuration. Do not require a runtime texture gate for C03
as part of the normal Pike asset workflow until a real selector/use case is
identified.

### `node_reference_status=EXTERNAL_REFERENCES_FOUND`

Follow exactly the listed files/call chain before deciding the role.

The scanner is read-only.
