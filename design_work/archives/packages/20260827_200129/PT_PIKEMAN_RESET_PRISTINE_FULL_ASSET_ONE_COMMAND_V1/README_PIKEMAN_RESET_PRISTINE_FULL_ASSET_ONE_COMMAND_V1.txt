PT_PIKEMAN_RESET_PRISTINE_FULL_ASSET_ONE_COMMAND_V1

Single-command reset and rebuild.

Key design:
- restores client pristine first;
- moves prior G5/G6/G7 user-edited .blend files out of the active workspace into a timestamped archive;
- archives prior G7 generated outputs to avoid overwrite collisions;
- fresh-imports pristine TmbC01.smd;
- changes ONLY SMD_UV on all 9 meshes using the same affine UV mapping;
- vertex positions, topology and skin weights remain pristine before writer round-trip;
- runs the existing strict G7 inspector/builder;
- validates C001/C002 at high/default/low;
- audits materials;
- corrects manifest provenance to AUTOMATED_UV_AFFINE_WITNESS_FROM_PRISTINE;
- publishes and deploys through workspace bridge + roundtrip restore V4;
- verifies deployed SHA exactly equals build artifact SHA.

No manual Blender work is required.
