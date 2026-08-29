# Summit Tale Asset Tool V0.4.7 — Multi-Object Object-Local Preview V2

V0.4.7 extends the V0.4.6 object-local animation parser/preview from single-object SMDs to the multi-object cases proven by the supplied client samples and SourceCode.

## Proven runtime semantics

The supplied SourceCode shows:

- `smPAT3D::LinkObject()` resolves `NodeParent` and builds `TmSort` parent-first.
- `smPAT3D::TmAnimation()` evaluates objects in `TmSort` order.
- `smOBJ3D::TmAnimation()` composes a child as `qmat * pParent->TmResult`.
- a child without local transform keys reconstructs its bind-local transform from `Tm * pParent->TmInvert` before parent composition.

The parser now reads `Tm`, `TmInvert`, `NodeParent`, validates case-insensitive parent resolution and cycles, and emits the explicit hierarchy graph.

## Client representatives

- `Effect/ObjAnimationData/PikeWind/bong.smd`: 3 independent animated roots, ROT+POS.
- `Field/forest/3ani-04.smd`: 2 independent animated roots, ROT.
- `Effect/AssaEffect/Skill3Mesh/At3_SplitJavelin40.smd`: animated root `wt108` plus static child `bip SJ01`; the child inherits the parent transform.

## Preview V2 behavior

- all independent animated roots are baked in one preview;
- `NodeParent` is recreated in Blender while preserving imported world placement;
- static/HOLD children inherit their parent animation;
- rotation continues to use accumulated `TmPrevRot` semantics;
- preview timing remains `PREVIEW_NOT_RUNTIME_SPEED_CERTIFIED`.

## Fail-closed boundary

V0.4.7 intentionally refuses an **animated child that also has its own object-local curves**. The engine composition is visible in SourceCode, but no physical supplied representative proves the Blender mapping for that subtype yet. It raises `OBJECT_LOCAL_PREVIEW_V2 requires a physical animated-child representative`.

Segmented `TmFrameCnt > 0` object-local tracks also remain unsupported.

## Commands

```bash
python3 pt_asset.py validate object-local Effect/ObjAnimationData/PikeWind/bong.smd
python3 pt_asset.py validate object-local Effect/AssaEffect/Skill3Mesh/At3_SplitJavelin40.smd
python3 pt_asset.py validate object-local Field/forest/3ani-04.smd
```

## Texture path normalization fix

The supplied `Test1.tga` proved a separate Linux resolver issue: some material strings serialize repeated Windows separators such as `Effect\\ObjAnimationData\\Test\\TEST1.tga`. The old `norm_path()` converted these to `Effect//ObjAnimationData//Test//TEST1.tga`, which did not match the physical resolver index. V0.4.7 collapses repeated separators before lookup. The real `Test1.tga` is a standard RGBA 64×8 blue/cyan texture, not a white material.
