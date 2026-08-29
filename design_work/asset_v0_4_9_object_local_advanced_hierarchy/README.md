# Summit Tale Asset Tool V0.4.9 — Object-Local Advanced Hierarchy

Base: V0.4.8.

## Goal

Extend static Model0.62 object-local animation preview to hierarchy cases proven by
physical client representatives and the original client SourceCode.

## New behavior

- Animated child objects are now supported.
- Deep NodeParent chains are supported.
- Blender animation uses Delta Transforms over the pristine/static imported bind:
  - `delta_location`
  - `delta_rotation_quaternion`
  - `delta_scale`
- This preserves the imported bind/placement while applying each object's
  FIRST_KEY-normalized local animation on top of it.
- Runtime NodeParent resolution now mirrors `smPAT3D::LinkObject`:
  - first case-insensitive NodeName match in serialized order;
  - missing NodeParent helper leaves `pParent == null`, therefore the object is a root;
  - duplicate NodeNames are diagnostic, not automatically rejected.
- Blender object mapping is by serialized object order and handles Blender duplicate
  names (`Name`, `Name.001`, ...).

## Source semantics

Original client code proves:

`TmResult = qmat * pParent->TmResult`

and `smPAT3D::LinkObject()` evaluates parent objects before descendants.

## Intentional fail-closed subtype

The following physical representatives are not silently normalized:

- `Effect/AssaEffect/Skill3Mesh/dancesword.smd`
- `Effect/AssaEffect/Skill3Mesh/mg3_danceSd40_110_125.smd`
- `Effect/AssaEffect/Skill3Mesh/Mg3_Diastrophism24.smd`

They contain legacy non-unit quaternion / accumulated `TmPrevRot` affine drift.
The runtime preserves this matrix behavior; converting it to a pure normalized
Blender quaternion would erase scale/shear. They are reported as:

`LEGACY_AFFINE_ROTATION_UNSUPPORTED`

A later patch must preview this subtype with affine-matrix-preserving semantics.

## Still not timing-certified

Preview stays at 30 fps and remains:

`PREVIEW_NOT_RUNTIME_SPEED_CERTIFIED`

No client file is modified.
