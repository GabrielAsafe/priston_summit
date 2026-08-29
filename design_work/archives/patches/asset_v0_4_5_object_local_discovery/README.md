# Summit Tale Asset Tool V0.4.5 — Object-Local Animation Discovery

Base: V0.4.4 (includes the certified no-mixed-Physique corpus gate).

## New audit

```bash
python3 pt_asset.py audit object-local --limit 50
```

The inventory survey retained object block length plus vertex/face/TexLink counts. For no-Physique Model0.62, the known serialized layout gives:

```text
residual = object_length - (2236 + 24*nVertex + 36*nFace + 32*nTexLink)
         = 84*nRot + 16*nPos + 16*nScale
```

Therefore `residual > 0` proves serialized object-local transform arrays exist even before reading the physical SMD again.

## Real corpus discovery

- static Model0.62 files: 1195
- static object headers audited: 2714
- files with object-local arrays: 432
- objects with object-local arrays: 1723
- plain static objects: 991

By category:

- MAP_STAGE: 353 files
- EFFECT: 63 files
- MOB_MONSTER: 10 files
- WEAPON_ITEM: 5 files
- UNKNOWN: 1 file

Deterministic representatives include:

- `image/Sinimage/Items/DropItem/itds103.smd`
- `Effect/ObjAnimationData/Test/Test.smd`
- `Field/castle/crystal_ani01.smd`
- `Effect/ObjAnimationData/MonsterWeb/Web.smd`

## Current boundary

The inventory does not retain individual `nRot/nPos/nScale` values. Exact animation decoding, Blender key creation and writer preservation therefore require the physical representative SMDs. V0.4.5 is a discovery/audit patch, not yet object-local authoring certification.
