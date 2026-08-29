# V0.5.2 Local Test Results

## Compile

`python3 -m compileall pt_asset_tool`: PASS.

## Final physical failure resolved

`Effect/NewEffect/Res/Object/Skill5_HyperSonic.smd`:

- object count: 1 (`Sphere01`)
- channels: SCALE only
- `nScale`: 101
- first key: frame 0, scale `(0,0,0)`
- frame 3200: scale `(1,1,1)`
- frame 8000: scale `(2.5,2.5,2.5)`
- frame 16000: scale `(5,5,5)`
- motion class: ANIMATED
- `scale_reference_semantics`: `ZERO_SCALE_SPAWN_ABSOLUTE_OVER_STATIC_BIND`
- zero reference axes: `[0,1,2]`
- parser: PASS

## Source-code semantic confirmation

The supplied client SourceCode implements scale interpolation in `smOBJ3D::GetScaleFrame()` as direct fixed-point interpolation and multiplies the resulting scale matrix into `qmat` in `TmAnimation()`. There is no first-key division in the runtime path.

## Physical representative regression

20 unique physical representatives accumulated during the object-local investigation were parsed with V0.5.2:

- original ROT / POS / SCALE single-object representatives;
- flat multi-object representatives;
- parent + static child;
- animated children + parent;
- deep hierarchy;
- unresolved runtime roots;
- three legacy-affine effects;
- final zero-scale HyperSonic effect.

Result: **20/20 PASS**.

## V0.5.1 numeric regression

For the 19 representatives that already passed V0.5.1, first/middle/last normalized transform samples were compared between V0.5.1 and V0.5.2 after ignoring only the newly-added diagnostic metadata fields.

Result: `existing_numeric_regression_identical=true` for **19/19** files.

## Corpus completion logic regression

Synthetic corpus-audit tests verify:

- supported animated-child / unresolved-parent tags no longer force `complete_candidate=false`;
- a real full-parser failure still forces `complete_candidate=false`;
- segmented `TmFrameCnt` remains an unsupported blocker.

## Blender limitation of this environment

This container does not provide the user's Blender/Flatpak runtime. The zero-scale preview path uses the existing normal `delta_scale` animation channel, which accepts zero scale. Final visual confirmation of `Skill5_HyperSonic.smd` and the full 432-file corpus audit should run on the user's installation.
