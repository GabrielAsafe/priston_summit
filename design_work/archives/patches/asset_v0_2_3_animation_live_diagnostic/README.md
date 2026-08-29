# PT Asset Tool 0.2.3 — Animated Bridge Diagnostic

This patch replaces the frozen worst-frame diagnostic with a live, side-by-side animation comparison.

Changes:
- SOURCE_DIRECT is animated across the selected motion frames using source skinning (`current * inverse(bind)`).
- BLENDER_ARMATURE keeps playing the baked Action on the same frame range.
- both witnesses are spatially separated without freezing the Action.
- a diagnostic camera frames the complete pair.
- Blender GUI opens in camera view and starts playback automatically.
- visual questions explicitly refer to the animated witnesses.
- client files remain untouched.

Install from `design_work` or pass `design_work` as the installer argument.
