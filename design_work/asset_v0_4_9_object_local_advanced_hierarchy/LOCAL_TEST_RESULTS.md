# V0.4.9 Local Test Results

## Physical rare representatives

Using `PT_OBJECT_LOCAL_RARE_REPS.zip`:

### Animated child + parent

- `Field/AncientW/RotObj01.smd`: PASS parser, 5 objects, 4 resolved animated children.
- `Field/AncientW/RotObj02.smd`: PASS parser, 5 objects, 4 resolved animated children.
- `Field/AncientW/RotObj03.smd`: PASS parser, 5 objects, 4 resolved animated children.

### Deep hierarchy

- `Field/Fall_Game/chandelier-ani01.smd`: PASS parser, chain depth 4, 5 animated objects.
- `Field/dungeon/Dun-6a-ani-014.smd`: PASS parser, root animation plus two inherited descendants.
- `Field/dungeon/Dun-6a-ani-019.smd`: PASS parser, root animation plus two inherited descendants.

### Runtime unresolved parent behavior

- `Field/SeaA/sea_ani03.smd`: PASS parser; 24 unresolved `Group001` declarations become runtime roots; duplicate NodeName group retained diagnostically.
- `Field/SeaA/sea_ani06.smd`: PASS parser; 24 unresolved `Group006` declarations become runtime roots.
- `Field/SeaA/sea_ani07.smd`: PASS parser; 24 unresolved `Group098` declarations become runtime roots.

This matches the client `smPAT3D::LinkObject()` implementation: if no NodeName
match is found, `pParent` remains null and the object is included as a root in
`TmSort`.

### Legacy affine rotation blockers

Fail-closed with explicit classification:

- `dancesword.smd`: `TmPrevRot` cumulative mismatch ~0.0015354.
- `mg3_danceSd40_110_125.smd`: quaternion norm reaches ~1.02037639.
- `Mg3_Diastrophism24.smd`: `TmPrevRot` cumulative mismatch begins at ~0.0070113 and several objects contain significant non-orthonormal affine matrices.

These are not treated as ordinary quaternion rotations.

## Regression representatives

V0.4.7 representatives remain parseable:

- `bong.smd`: PASS, 3 independent animated objects.
- `At3_SplitJavelin40.smd`: PASS, resolved parent + static inherited child.
- `3ani-04.smd`: PASS, 2 independent animated objects.

## Blender orchestration simulation

Fake-Blender orchestration test passed for:

- `RotObj01.smd`: 4 animated children receive Delta Transform curves and remain parented to `ani-001`.
- `chandelier-ani01.smd`: all 5 objects receive Delta Transform curves through a depth-4 chain.
- `sea_ani03.smd`: duplicate serialized names map to Blender-style `Name`, `Name.001`, ... by source order.
- `sea_ani06.smd`: unresolved helpers produce independent runtime roots.

First keyed delta is identity for every animated object, proving the static bind
is not overwritten at preview frame 1.

## Automatic checks

- `python3 -m compileall`: PASS.
- no source/client file modified.
