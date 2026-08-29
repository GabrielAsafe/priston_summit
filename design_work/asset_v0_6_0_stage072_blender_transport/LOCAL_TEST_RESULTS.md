# V0.6.0 local tests

Environment without Blender runtime; tests below cover the physical/parser half and package wiring.

## Physical representatives supplied by user

Input: `PT_STAGE072_REPS_V059.zip` — 33 physical representatives.

- 33/33 `parse_stage072(..., include_payload=True)` PASS.
- payload vertex count == serialized `nVertex` for 33/33.
- payload face count == serialized `nFace` for 33/33.
- payload TexLink count == serialized `nTexLink` for 33/33.
- payload light count == serialized `nLight` for 33/33.
- maximum chain depth in payload agrees with physical audit.

Observed texture stage operations across the 33 representative files:

```text
stage 0: DEFAULT(0)       1,582,457 face uses
stage 1: DEFAULT(0)         638,229 face uses
stage 1: D3DTOP_ADD(7)        7,899 face uses
stage 2: DEFAULT(0)           6,609 face uses
```

No unsupported non-stage0 combiner was observed in the representative bundle.

Representative mini-audit using the V0.6.0 parser:

```text
status=PASS
full_parse=33/33
depths={'1': 6, '2': 24, '3': 3}
area={'DWORD32': 29, 'DWORD32_TRAILING_OPAQUE': 3, 'WORD16': 1}
```

## Python/package checks

- `python3 -m compileall` PASS.
- package version `0.6.0` PASS.
- CLI parse for `validate stage072-preview` PASS.
- new doctor-required scripts are present.

## Blender-dependent checks

Not executable in this environment. The user-side gate deliberately uses two Blender processes:

1. import + save `.blend`;
2. reopen `.blend` + recompare geometry, face material IDs, UV stages, vertex colors, light metadata and material stage metadata against the original SMD.
