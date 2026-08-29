# V0.4.6 local test results

## Physical representative parse

All four user-supplied physical SMDs parse with exact object-block length equality.

| Representative | Serialized channels | Moving channels | Counts (rot/pos/scale) | Max engine frame | Result |
|---|---|---|---:|---:|---|
| Test.smd | ROT | ROT | 18/0/0 | 16000 | PASS |
| Web.smd | POS,SCALE | POS,SCALE | 0/11/11 | 16000 | PASS |
| crystal_ani01.smd | ROT,POS | ROT,POS | 201/198/0 | 32000 | PASS |
| itds103.smd | ROT,POS | none | 2/2/0 | 16000 | HOLD detected |

## Rotation semantics

For every rotation key the parser verifies:

- first key: `TmPrevRot[0] ~= Matrix(raw_quaternion[0])`
- subsequent key: `TmPrevRot[i] ~= TmPrevRot[i-1] @ Matrix(raw_quaternion[i])`

Maximum observed matrix-component error:

- Test.smd: ~1.942256e-7
- crystal_ani01.smd: ~6.881102e-8
- itds103.smd: 0

A deliberately corrupted TmPrevRot matrix is rejected fail-closed.

## Preview orchestration

With a deterministic fake Blender harness:

- Test.smd ROT preview orchestration: PASS
- Web.smd POS+SCALE preview orchestration: PASS, frame range 1..101
- crystal_ani01.smd ROT+POS preview orchestration: PASS, frame range 1..201
- itds103.smd HOLD: rejected before Blender as intended
- all summaries: `client_modified=false`

The real Blender visual gate is intentionally left for the client machine.

## Static roundtrip preservation

The strict witness comparator was exercised on physical Test.smd with one vertex XYZ edit:

- animation key counts source/output: 18/0/0 -> 18/0/0
- non-XYZ changed bytes: 0
- `object_local_animation_preserved_byte_identical=true`
- comparator: PASS

Full roundtrip orchestration with the fake Blender harness also returned:

- `static_roundtrip=PASS`
- `rebuilt_capability=PASS`
- `object_local_animation_preserved=PASS`
- `client_modified=false`

## Corpus boundary

V0.4.5 inventory audit re-evaluated at full candidate output:

- object-local candidate files: 432
- single-object: 218
- multi-object: 214

V0.4.6 does not infer multi-object hierarchy semantics from filenames or inventory metadata.
