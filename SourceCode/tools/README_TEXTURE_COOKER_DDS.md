# Summit V3 offline DDS texture cooker

This is an additive optimization for `AssetStreamingV3`. The game keeps each
legacy logical texture name (for example `field\\foo\\ground.png`), while the
worker first probes `cooked\\field\\foo\\ground.dds`. If no compatible cooked
artifact exists, the original source path is loaded exactly as before. A cooked file older than its source is treated as stale and ignored.

## Runtime behavior

Cooked DDS lookup is enabled by default when a matching file exists.

```bash
SUMMIT_TEXTURE_COOKED=0 wine Game.exe   # force legacy/source path (A/B)
SUMMIT_TEXTURE_COOKED=1 wine Game.exe   # prefer cooked DDS (default)
```

The default cooked root is `cooked` relative to the client working directory.
It can be overridden:

```bash
SUMMIT_TEXTURE_COOKED_ROOT=cooked_test wine Game.exe
```

With `SUMMIT_ASSET_TRACE=1`, successful cooked imports emit `COOKED_DDS_HIT`
in `LOGS\\asset_load_v2_events.txt`. Invalid cooked artifacts are rejected for
the remainder of the process so a retry uses the original source file.

## Cooking

The tool needs Pillow only at build/cook time; the game has no new runtime
dependency.

From the client asset root:

```bash
python3 /path/to/SourceCode/tools/cook_textures_dds.py --input .
```

By default the cooker mirrors the current runtime compression policy for ordinary
world textures: only textures whose uncompressed RGBA footprint is at least 1 MiB
(512x512 RGBA) and whose top level is DXT block-compatible are emitted. This
threshold is intentionally lower than the old 16 MiB rollout because runtime traces
showed 512x512 and 1024x1024 source uploads already causing visible frame stalls.
PNG/TGA/STA use DXT5; opaque JPG/BMP/STG use DXT1. Sources remain untouched.

Item icons under `image/Sinimage/Items/**` are runtime-required by default and are
never skipped solely because they are below the world-texture threshold or are not
DXT block-compatible. Large/default-eligible item icons keep DXT1/DXT5; smaller or
non-block-compatible icons are cooked losslessly as A8R8G8B8 DDS. This keeps the
legacy/protected source representation as authoring evidence while allowing runtime
to consume a deterministic cooked DDS. Use `--no-item-icons-required` only for A/B
against the historical threshold behavior.

For a focused item-icon cook:

```bash
python3 /path/to/SourceCode/tools/cook_textures_dds.py --input . --item-icons-only
```

The item-icon incremental path does not trust mtime alone. Before an existing DDS is
accepted as up-to-date, the cooker decodes the source with the official legacy codec
and verifies the cooked DDS width, height and expected pixel format. A stale/mismatched
DDS is rebuilt even if its mtime is newer than the source.

To preview without writing:

```bash
python3 /path/to/SourceCode/tools/cook_textures_dds.py --input . --dry-run
```

To rebuild everything eligible:

```bash
python3 /path/to/SourceCode/tools/cook_textures_dds.py --input . --force
```

A full mip chain can be generated explicitly:

```bash
python3 /path/to/SourceCode/tools/cook_textures_dds.py --input . --mipmaps
```

Mipmaps are off by default in the first rollout so the cooked path preserves
the current V3 visual/runtime behavior as closely as possible. The runtime can
import precomputed mip chains when a texture request asks for mipmaps.

The cooker writes `cooked/manifest.json` for audit/debug purposes. The runtime
does not depend on the manifest: lookup is deterministic from the logical path.

## Recommended first A/B

1. Cook with defaults (no `--mipmaps`).
2. Start once with `SUMMIT_TEXTURE_COOKED=0` and reproduce the same map route.
3. Start with default cooked lookup and compare stutters/loading.
4. Enable `SUMMIT_ASSET_TRACE=1` if you need to confirm `COOKED_DDS_HIT` events.

Do not raise texture budgets solely because the cooked path exists. Resident
and staging budgets remain part of the i686 safety policy.

## Manifest V2 e retry de failures

O cooker agora grava `cooked/manifest.json` no formato `SUMMIT_TEXTURE_COOK_MANIFEST_V2` e persiste também os failures. Cada failure inclui a fase (`decode`, `encode`, `write`), classe normalizada, exception original, tamanho/mtime/SHA-256 da fonte e os primeiros bytes do header para diagnóstico de formatos legacy.

Um resumo dedicado é escrito em:

```text
cooked/failed.json
```

Retry somente dos failures persistidos:

```bash
python3 /path/to/SourceCode/tools/cook_textures_dds.py --input . --retry-failed
```

Para um manifest alternativo:

```bash
python3 /path/to/SourceCode/tools/cook_textures_dds.py --input . --retry-failed /path/to/manifest.json
```

Para migrar um log antigo que contém linhas `ERROR <path>: <message>`:

```bash
python3 /path/to/SourceCode/tools/cook_textures_dds.py --input . --retry-log old_cooker.log
```

No modo retry o manifest anterior é preservado/mesclado: entradas não retentadas continuam presentes e somente os sources selecionados são atualizados. O source original continua sendo fallback runtime e nunca é removido pelo cooker.

## Protected TGA variants (V2.1)

The cooker now mirrors the protected-TGA contract already proven by PT Asset
Tool (`PT_PROTECTED_TGA_V2`). Two legacy header modes are accepted, fail-closed:

- `FULL18`: the historical `G8` form; all 18 TGA header bytes are protected.
- `KEEP_00_16`: bytes 0..1 remain authored while bytes 2..17 use the same
  Priston header transform.

A protected candidate is used only when the decoded TGA header is plausible and
its type-2 or type-10 payload closes exactly. Unknown encodings remain failures
and are persisted in `cooked/failed.json`; the cooker never mutates/removes the
source texture.

Successful manifest rows include `legacy_decode_mode`, for example
`TGA_FULL18`, `TGA_KEEP_00_16`, or `TGA_STANDARD`.

### Count the persisted failures

`cooked/failed.json` stores failures in `entries`:

```bash
python3 - <<'PY'
import json
with open('cooked/failed.json', encoding='utf-8') as f:
    data = json.load(f)
print('DDS failures:', len(data.get('entries', [])))
print('classes:', data.get('failure_class_counts', {}))
PY
```

`cooked/manifest.json` additionally exposes the same rows in its `failures`
field for compatibility/convenience.


## Rare texture families integrated from PT Asset Tool (V2.2)

V2.2 consumes two additional physical-family contracts that were already
reverse-engineered and fail-closed in PT Asset Tool. The cooker duplicates only
the proven decode equations so it remains standalone at build time.

### TGA `DIMENSIONS_12_15`

For this physical family, bytes 0..11 and 16..17 are already a standard
uncompressed true-color TGA header, while only width/height bytes 12..15 use the
historical additive protection key.

The cooker accepts this mode only when:

- the literal physical header shape matches the proven family;
- decoded width/height/pixel depth are plausible;
- the payload boundary closes exactly.

Successful rows use:

```text
legacy_decode_mode = TGA_DIMENSIONS_12_15
```

### Character-selection ship BMPs

The ten historically known `game/maps/chrselect/ship_*.bmp` files use the
normal protected 14-byte BMP file header plus an independently recovered
40-byte protected DIB header. V2.2 recognizes this family only when all
structural equations close exactly:

- DIB40;
- positive dimensions;
- planes=1;
- 24 bpp;
- compression=0;
- pixel offset=54;
- image size equals width*height*3;
- exactly two physical zero bytes follow the pixel payload;
- pixel bytes remain byte-identical while decoding the header.

Successful rows use:

```text
legacy_decode_mode = BMP_SHIP_DIB40
```

### Documented non-cookable outliers

PT Asset Tool already classifies a small set of physical outliers that should
not be converted by guessing. They remain `status=error` in `failed.json`, but
V2.2 enriches the failure row with `documented_outlier`,
`asset_classification`, `cook_policy`, and `runtime_note`.

Current documented examples include the HTML-mislabeled
`Field/Ricarten/ch_tree1.tga`, the zero-filled MonsterFury texture,
`ArchivoTmp.tga` (preview-only recovery; old-client runtime validity unproven),
and the unsupported historical `force.bmp` outlier and `Effect/sky/s.bmp`, whose observed payload begins with ASCII server/config text rather than a BMP header.

Keeping these as failures is intentional: a green cooker must not be obtained
by manufacturing DDS files from physical data whose game-runtime semantics are
not proven.

## Integrity + retention metadata (V2.3)

V2.3 does not add permissive decoders. It makes the distinction between build
state, asset integrity, and retention explicit so PT Asset Tool can consume the
cooker as evidence without treating a conversion failure as proof that a source
is unused.

### Successful decode but intentionally not cooked

A source can be a valid decoded image and still be skipped by the DDS policy.
Such rows now include:

```text
status = skipped
skip_reason = below-runtime-dxt-threshold | too-small-for-dxt | top-level-not-block-aligned
skip_class = VALID_DECODE_*
asset_integrity_state = VALID_IMAGE_POLICY_SKIPPED
decoded_ok = true
runtime_dds_expected = false
deletion_authorized = false
```

This is the expected state for valid legacy images that do not meet the current
runtime DDS threshold; it is not a cook failure.

### Residual decode failures

Failure rows now always include:

```text
asset_integrity_state
retention_decision_required = true
deletion_authorized = false
```

Known PT Asset Tool outliers also expose a normalized `outlier_reason` in
addition to the existing `asset_classification`, `cook_policy`, and
`runtime_note` fields. Unknown failures remain
`UNRESOLVED_DECODE_FAILURE`; the cooker never promotes them to delete or
quarantine decisions because reachability belongs to PT Asset Tool.

### Incremental metadata preservation

When an existing DDS is already up-to-date, V2.3 preserves known decode and
integrity metadata from the previous manifest instead of replacing it with a
minimal `up-to-date` row. This progressively reduces `UNKNOWN` decode-mode
entries as assets are actually decoded by new/full/retry cooks, without forcing
an expensive recook solely for metadata.
