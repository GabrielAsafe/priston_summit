# V0.6.6 local test results

## Source proof incorporated

- `SrcGame/src/smLib3d/smMap3d.cpp::SetNormal` and `SrcServer/...` are identical.
- `SetNormal`: float cross components, `sqrt(double(float sum-of-squares))`, cast length to float, divide/multiply by 32767, truncate to int.
- `FLOATNS == 8`.
- `SetVertexShade` uses `Vertex.x/y/z >> 8` and stores only `VectNormal[0..2]`.

## Compile / CLI

- `python3 -m compileall`: PASS.
- package version: `0.6.6`.
- `validate stage072-geometry-roundtrip --help`: PASS.

## Pure physical writer tests

Using shipped Stage0.72 files from the 63-file corpus, with a +64-world-unit X witness:

- `Field/dungeon/dun-1.smd`: PASS; one incident face; StageArea core changed; file size changed from 4,539,876 to 4,539,888; rebuilt StageArea proof PASS; rebuilt normal family current-exact.
- `Field/endless/dun-7.smd`: PASS; one incident face; StageArea core changed; 3-stage TexLink data preserved; file size 5,857,569 -> 5,857,577.
- `Field/Quest/quest_IV.smd`: PASS; StageArea core changed; file size 966,265 -> 966,273.
- `Field/Room/office.smd`: PASS; StageArea and StageMapRect changed; file size 475,345 -> 475,381.

For all four, strict comparator PASS:

- one vertex X coordinate only;
- only incident `VectNormal[0..2]` authorized;
- `VectNormal[3]` exact;
- materials/TexLinks/lights exact;
- rebuilt StageArea physical proof PASS;
- output remains in the current exact normal family.

## Negative gates

Expected fail-closed before writing:

- `Field/Lost/lost.smd` — unresolved legacy 1-LSB family.
- `cSelect/select.smd` — unresolved legacy family / opaque StageArea suffix.
- `game/maps/chrselect/scene.smd` — reciprocal legacy family.
- `Field/AncientW/ancientW.smd` — reciprocal legacy family.

A deliberate collateral byte mutation in a rebuilt `dun-1.smd` is rejected by the strict comparator.

## Blender

The actual persisted Blender witness/writer/reimport gates must run on the user's configured Blender installation. The local environment tests only the pure binary writer/comparator and compiles the Blender scripts.
