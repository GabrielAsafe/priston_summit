# V0.6.3 local test results

Environment does not provide the user's Blender/Flatpak runtime, so Blender GUI/background execution remains a user-side gate. Backend/writer logic was exercised against physical Stage0.72 representatives from `PT_STAGE072_REPS_V060.zip`.

## Compile / install

- `python3 -m compileall`: PASS
- install into isolated design_work clone: PASS
- `python3 pt_asset.py --version`: `0.6.3`
- CLI exposes `validate stage072-roundtrip`: PASS

## Physical strict-writer tests

All tests applied one real TexLink UV float witness and reparsed the rebuilt file:

| File | TexLink depth | StageArea | Opaque suffix | Result |
| --- | ---: | --- | ---: | --- |
| `Field/dungeon/dun-1.smd` | 2 | DWORD32 | 0 | PASS |
| `Field/endless/dun-7.smd` | 3 | DWORD32 | 0 | PASS |
| `Field/Lost/lost.smd` | 1 | WORD16 | 0 | PASS |
| `cSelect/select.smd` | 2 | DWORD32_TRAILING_OPAQUE | 124344 | PASS |

For all four:

- rebuilt physical parser: PASS
- Stage header byte-identical: PASS
- materials byte-identical: PASS
- geometry byte-identical: PASS
- face payload byte-identical: PASS
- lights byte-identical: PASS
- hTexture/NextTex byte-identical: PASS
- StageArea + suffix byte-identical: PASS
- non-witness bytes byte-identical: PASS

The depth-3 representative selects the deepest renderable stage, therefore its backend witness targets stage 2.

## Negative tests

- changing one byte in the opaque suffix after the valid witness: REJECTED
- expected source SHA mismatch: REJECTED

## Safety policy

Geometry writes are intentionally refused in V0.6.3. StageArea is a spatial face index and its rebuild/update semantics have not yet been certified for moved geometry. Transport of source geometry into Blender remains supported and automatically checked; serialization of moved geometry is deferred.
