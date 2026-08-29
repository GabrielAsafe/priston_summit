# V0.4.5 Local Test Results

## Inherited V0.4.4 Physique audit

```text
SMD Model0.62 = 3148
RIGID raw      = 1953
STATIC raw     = 1195
MIXED raw      = 0
UNKNOWN audit  = 0
object headers = 7901
NO_MIXED_MODEL_062_IN_CANONICAL_CORPUS
```

## Object-local discovery on the user's canonical inventory

```text
status=OBJECT_LOCAL_ANIMATION_CANDIDATES_FOUND
discovery_gate=PASS
static_model062_files=1195
static_object_headers_audited=2714
object_local_candidate_files=432
object_local_candidate_objects=1723
plain_static_objects=991
```

Category counts:

```text
MAP_STAGE    353
EFFECT        63
MOB_MONSTER   10
WEAPON_ITEM    5
UNKNOWN         1
```

Representative weapon/item:

```text
image/Sinimage/Items/DropItem/itds103.smd
object=Ds103
serialized_animation_bytes=200
```

Representative effect:

```text
Effect/ObjAnimationData/Test/Test.smd
object=Cylinder01
serialized_animation_bytes=1512
```

Representative map:

```text
Field/castle/crystal_ani01.smd
object=crystal_ani01
serialized_animation_bytes=20052
```

`python3 -m compileall`: PASS.
Version smoke test: PASS (`0.4.5`).
