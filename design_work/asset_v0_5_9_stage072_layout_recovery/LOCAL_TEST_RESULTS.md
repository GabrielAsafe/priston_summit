# V0.5.9 local test results

Test corpus: exact 63 Stage0.72 files extracted from the user's `PT_STAGE072_REPS_V058.zip`.

## Positive corpus gate

```text
physical files: 63
full_parse PASS: 63
full_parse FAIL: 0
stage072_physical_complete_candidate: true
```

### Area encoding

```text
DWORD32:                  59
WORD16:                     1
DWORD32_TRAILING_OPAQUE:    3
```

Opaque suffix total: 577068 bytes.

### TexLink depth

```text
depth 1: 16 files
depth 2: 44 files
depth 3:  3 files
```

Depth-3 files:

```text
Field/Mine/mine-1.smd
Field/Slab/Slab.smd
Field/endless/dun-7.smd
```

For every depth-3 face in those three representatives, stage 2 is structurally valid. Stage-2 UVs are distinct from stages 0/1 on the depth-3 faces.

### Inert zero-texture material links

```text
files: 11
faces: 5957
NULL hTexture: 4544
material slot-0 hTexture: 1413
```

## Negative fail-closed tests

1. Replaced the `hTexture` of a valid depth-1 zero-texture-material face with `0x12345678`.

Result:

```text
full_parse=FAIL
reason=zero-texture material uses unexpected TexLink handle
```

2. Replaced the first StageArea `slen` with a value larger than `wAreaSize` capacity.

Result:

```text
full_parse=FAIL
area=UNRESOLVED
```

No client file was modified.
