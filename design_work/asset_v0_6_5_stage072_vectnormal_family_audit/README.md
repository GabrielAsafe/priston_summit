# Summit Tale Asset Tool V0.6.5 — Stage0.72 VectNormal family audit

V0.6.4 proved `StageArea`, `wAreaSize`, pointer layout and `StageMapRect` reconstruction across the physical 63-file Stage0.72 corpus. V0.6.5 audits the remaining geometry-write dependency: `smSTAGE_FACE::VectNormal`.

## Safety policy

This release **does not enable arbitrary Stage0.72 vertex writing**. It only proves/classifies face-normal generation families. Original client files are read-only. `VectNormal[3]` is treated as opaque/provenance state and is never normalized or rewritten.

The supplied `smStage3d.cpp` proves that `SetVertexShade()`:

1. reads each face vertex,
2. shifts serialized fixed-point `x/y/z` by `FLOATNS` (8),
3. calls `SetNormal(p0,p1,p2,&pn)`,
4. stores `pn.x/y/z` into `VectNormal[0..2]`.

The physical corpus proves a current family that is byte-exact with float32 cross normalization. A second legacy family differs only by floating evaluation order / one-LSB rounding, and therefore remains fail-closed for geometry writing until its exact implementation is located or proven.

## Install

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
unzip -o asset_v0_6_5_stage072_vectnormal_family_audit.zip
./asset_v0_6_5_stage072_vectnormal_family_audit/install.sh \
  "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py --version
python3 pt_asset.py doctor
```

## Physical normal audit

```bash
python3 pt_asset.py audit stage072-normals \
  --output pt_build/stage072_vectnormal_audit_v065.json \
  --bundle PT_STAGE072_VECTNORMAL_REPS_V065.zip
```

Expected on the same 63-file client corpus used by V0.6.4:

```text
status=PASS
current_exact_files=46
legacy_reciprocal_exact_files=7
legacy_float_variant_unresolved_files=10
max_legacy_component_delta=1
degenerate_faces=17716
degenerate_zero_normal_faces=17716
stage072_geometry_write_complete_candidate=false
```

`PASS` here means the audit successfully classified the physical data. It does **not** mean all normal families are writable yet.

## Locate the exact legacy `SetNormal`

Please also run this in the full supplied SourceCode tree and preserve the output verbatim:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"

rg -n --hidden --glob '!**/.git/**' \
  --glob '*.{c,cc,cpp,cxx,h,hpp}' \
  'SetNormal[[:space:]]*\\(' .

rg -n --hidden --glob '!**/.git/**' \
  --glob '*.{c,cc,cpp,cxx,h,hpp}' \
  '32767|0x7FFF|VectNormal' .
```

If `rg` is unavailable, use `grep -RIn` with the same terms.

The next geometry writer will be enabled only after the normal family for the target map is exact. StageArea and StageMapRect are already proven by V0.6.4.
