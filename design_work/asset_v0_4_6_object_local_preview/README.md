# Summit Tale Asset Tool V0.4.6 — Object-Local Animation Preview V1

Base: V0.4.5. No source/client file is modified.

## What this adds

- Exact Model0.62 object-local parser for serialized `rot`, `pos`, `scale`, and `TmPrevRot` tracks.
- Rotation semantic guard: raw quaternion keys are verified as incremental local deltas and `TmPrevRot` as the accumulated absolute rotation source-of-truth.
- `asset object-local <asset>` source inspection command.
- `validate object-local <asset>` Blender preview gate for flat/single-object static Model0.62.
- First-key-normalized preview: the imported mesh keeps its static placement; only local motion relative to its first key is shown.
- Preview samples every 160 engine-frame units and keeps runtime speed explicitly uncertified.
- Strict static witness roundtrip now permits object-local assets only when their animation arrays remain byte-identical.

## V1 safety boundary

V1 intentionally refuses:

- Physique objects;
- `TmFrameCnt > 0` segmented object-local tracks;
- NodeParent hierarchy;
- multi-object object-local visual certification;
- HOLD-only tracks as visual animation representatives.

The corpus audit found 432 static Model0.62 files with object-local arrays: 218 single-object and 214 multi-object. V0.4.6 directly targets the proven single-object subset. Multi-object/hierarchy stays fail-closed until a physical representative is analyzed.

## Representative source semantics proven

- `Effect/ObjAnimationData/Test/Test.smd`: rotation-only, 18 rot keys.
- `Effect/ObjAnimationData/MonsterWeb/Web.smd`: position+scale, 11+11 keys.
- `Field/castle/crystal_ani01.smd`: rotation+position, 201+198 keys.
- `image/Sinimage/Items/DropItem/itds103.smd`: serialized rot+pos but HOLD; correctly rejected as a moving visual representative.

## Timing

Preview FPS is a Blender inspection convenience only. V0.4.6 reports:

`PREVIEW_NOT_RUNTIME_SPEED_CERTIFIED`
