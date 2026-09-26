# Summit Text Content Port

Offline migration toolchain for legacy textual content. Model/stage geometry formats
(`SMD`, `SMB`, `INX`, `ASE`) are deliberately outside this project; paths to those
assets are treated as opaque references only.

The implementation follows the same horizontal-migration pattern used by the newer
Summit systems: a neutral domain model, an interface boundary, a legacy source and a
compiled source can coexist. Calls are switched only after parity is demonstrated.

## Current vertical slice

Stage spawn (`GameServer/Field/*.spm`) is the first end-to-end slice:

```
legacy SPM ───────────> LegacyStageSpawnContentSource ─┐
                                                       ├─> IStageSpawnContentSource
UTF-8 JSON -> asset compiler -> SSPB v2 -> Compiled... ┘
                                                       ↓
                                              StageSpawnDefinition
                                                       ↓
                                             legacy materializer
                                                       ↓
                                           rsSTG_MONSTER_LIST
```

By default the active `DecodeOpenMonster()` bridge still uses the legacy source. Set
`SUMMIT_STAGE_SPAWN_PACK=<path>` to A/B-test the compiled source. If the pack/path is
not usable it falls back to legacy. `SUMMIT_STAGE_SPAWN_STRICT=1` disables that
fallback for validation runs.

SSPB v2 carries both stable monster IDs when the current corpus permits an exact
resolution and the exact legacy monster-name bytes needed while `smCHAR_INFO::szName`
is still the runtime join key.

## Map catalog slice

The static map catalog is now end-to-end. `semanticize_maps.py` converts the current
legacy `InitField()` source into profile-specific `summit.map-catalog/v1` UTF-8 JSON;
`compile_maps.py` emits deterministic `SMAP v1`; `CompiledMapContentSource` verifies
the canonical layout and SHA-256 before loading neutral `MapDefinition` objects; and
`LegacyMapAdapter` materializes them into the existing `sFIELD` runtime.

Client/server remain separate because 14 legacy fields differ semantically. Dynamic
field 53 (`smConfig`) intentionally remains on the legacy runtime path. The static
compiled path preserves reciprocal `AddGate()` behavior, feature-gated
`WORLDCUP_FIELD` stage objects and the four orphan warp `LimitLevel` writes.

`InitField()` remains legacy by default for rollout safety. Set
`SUMMIT_MAP_PACK=<path>` to A/B-test the compiled source. An unusable/wrong-profile
pack falls back to legacy; `SUMMIT_MAP_STRICT=1` disables fallback for validation
runs. The source-derived compiler reproduces the handoff client/server packs
byte-for-byte. Format details are in `SMAP_V1.md`.

```bash
python3 tools/content_port/semanticize_maps.py \
  Shared/world/field/field_legacy_client.inl --profile client --source-root . \
  --output /tmp/client.maps.json
python3 tools/content_port/compile_maps.py /tmp/client.maps.json /tmp/maps_client.smap
python3 tools/content_port/inspect_maps.py /tmp/maps_client.smap
SUMMIT_MAP_PACK=/tmp/maps_client.smap SUMMIT_MAP_STRICT=1 ./your-client
```

## Shadow JSON

Every supplied textual file is converted to UTF-8 JSON with provenance, SHA-256 and
raw byte evidence. Known gameplay families are parsed semantically at byte-token
level. This is necessary because the corpus contains files that mix CP949 directives
with Western byte strings, so assigning one codec to an entire file is unsafe.

The shadow import is intentionally non-destructive. It is an audit/authoring bridge,
not yet a declaration that every generic `.ini`, `.sin`, `.msg` or `.lua` line has a
closed semantic schema.

## Reproducible build

```bash
python3 tools/content_port/build_text_pipeline.py \
  --corpus /path/to/summit_text_corpus \
  --source-root . \
  --out-root /tmp/summit_content_build
```

This performs:

1. byte-aware import of the entire supplied textual corpus;
2. shadow validation;
3. cross-reference audit (SPM -> active top-level monster INF, linked files, IDs);
4. extraction of all 54 hardcoded maps from both client and server `InitField()`;
5. client/server map semantic comparison;
6. semanticization of static fields 0..52 and compilation of client/server SMAP v1;
7. compilation of all SPM data into SSPB v2.

Individual commands remain available:

```bash
python3 tools/content_port/import_corpus.py --corpus ... --source-root . --out ...
python3 tools/content_port/validate_shadow.py ...
python3 tools/content_port/validate_relations.py --shadow ... --corpus ... --out .../RELATIONS.json
python3 tools/content_port/compile_stage_spawns.py .../content_shadow .../stage_spawns.sspb
python3 tools/content_port/inspect_stage_spawns.py .../stage_spawns.sspb
```

`compile_shadow.py`/`inspect_binary.py` provide a generic audit container for the
whole shadow tree. It is not the final domain runtime format; runtime families get
explicit versioned packs such as SSPB.
