# Canonical legacy skip policy

The canonical tree preserves legacy input faithfully, including references that the historical runtime silently ignored because the referenced item or monster was absent from the executable catalogue.

Compiled runtime packs must not materialize unresolved references with empty IDs.

## NPC shops

For the server profile, `compile_definitions.py` keeps shop tokens only when a normal OpenItem definition for that code is materializable in the same compiled profile. Historical out-of-profile or missing OpenItem references remain in canonical JSON for audit, and are listed in the SCFG report under `filtered_shop_tokens`.

This reproduces the old parser behavior without manufacturing item codes or definitions.

## Stage spawns

An actor with no `monster_id` may be omitted from SSPB only when the canonical actor explicitly declares `resolution: legacy_skip_if_missing`. Any other actor without a monster ID is a build error.

`inspect_stage_spawns.py` rejects compiled packs containing empty actor or boss IDs.

This keeps legacy source evidence in `SourceCode/Content/canonical` while guaranteeing that standalone client/server runtime packs contain only resolvable records.
