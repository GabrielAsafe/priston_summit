# PT Asset Tool 0.2.7 — Motion Segment / Key Audit

Diagnostic-only continuation after Monbagon ATTACK and WALK pass while STAND deforms in both SOURCE_DIRECT and Blender.

Adds:

    python3 pt_asset.py diagnose motion-segment Monbagon-C.smd --motion STAND

The command does not alter client assets and does not open Blender. It reports:
- matrix pose steps around each motion boundary;
- sensitivity of evaluate_pose to the supplied motion end;
- parser SMB top-level/bone schema;
- automatically discovered likely keyframe arrays, grouped by bone layout.

No source evaluator semantics, INX/SMB data, Blender bridge, validators, or client files are modified.
