# V0.4.8 local test results

## Physical supplied representatives

The physical-corpus auditor was run against the three real V0.4.7 multi-object representatives:

- `Effect/ObjAnimationData/PikeWind/bong.smd`
- `Effect/AssaEffect/Skill3Mesh/At3_SplitJavelin40.smd`
- `Field/forest/3ani-04.smd`

Result:

```text
candidate_files_expected = 3
physical_files_audited = 3
full_parser_pass_files = 3
FLAT_MULTI_INDEPENDENT = 2
HIERARCHICAL = 1
STATIC_CHILD_INHERIT = 1
animated_child_files = 0
segmented_tmframe_files = 0
scan_errors = []
```

This matches the previously validated V0.4.7 visual behavior.

## Synthetic rare-subtype detection

A byte-copy of the real `bong.smd` was modified only in object-header metadata:

1. animated `Plane03` was assigned `NodeParent=Plane01`, preserving its own real curves;
2. `Plane01.TmFrameCnt` was set non-zero.

V0.4.8 detected both orthogonal rare classes:

```text
ANIMATED_CHILD_LOCAL_PLUS_PARENT = 1
CHILD_WITH_SERIALIZED_CURVES = 1
SEGMENTED_TMFRAME = 1
object_local_complete_candidate = false
```

The generated rare-representative ZIP contained the modified SMD exactly once despite matching multiple tags.

## Packaging/compile checks

- `python3 -m compileall pt_asset_tool`: PASS
- V0.4.7 behavior preserved; only new audit module/CLI entry added.
