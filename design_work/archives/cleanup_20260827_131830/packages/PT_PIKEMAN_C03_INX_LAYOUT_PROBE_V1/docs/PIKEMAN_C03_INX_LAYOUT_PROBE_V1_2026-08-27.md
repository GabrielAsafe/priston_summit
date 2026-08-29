# Pikeman C03 INX Layout Probe V1

The full-client reference scan proved that C03 is not dead data.

External references exist in:

```text
C002.inx
C003.inx
C004.inx
C005.inx
tmbC03.smd
```

The previous selector audit only decoded `_MODELGROUP` at the known primary
offsets:

```text
0x0C0 HighModel
0x104 DefaultModel
0x148 LowModel
```

and therefore returned zero C03 matches.

This probe answers why.

It prints, for `C001..C005` and `M4Bip`:

```text
szModelFile
szMotionFile
szSubModelFile
known High/Default/Low groups
exact offsets of TbhC03/TbmC03/TblC03
```

Then it scans the whole file on 4-byte alignment for any structurally valid
68-byte `_MODELGROUP`-like record containing those names.

Run:

```bash
python3 pt_pikeman_c03_inx_layout_probe_v1.py \
  --client-root "/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente" \
  --report logs/pikeman_c03_inx_layout_probe_v1.json
```

Interpretation:

- If a `GROUP_CANDIDATE` appears, inspect its exact offset before assigning a
  semantic field name.
- If no group candidate appears but raw offsets exist, the C03 names belong to
  another field/structure and must be mapped from the source definition.
- If `C002..C005.szModelFile` points to a separate C03 model family, that is
  direct evidence for a separate equipped-armor model configuration.

Read-only. Do not deploy TmbC03 yet.
