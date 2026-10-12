# Summit V3 offline DDS texture cooker

This is an additive optimization for `AssetStreamingV3`. The game keeps each
legacy logical texture name (for example `field\\foo\\ground.png`), while the
worker first probes `cooked\\material\\field\\foo\\ground.png.dds`. The
DIB inventory loader probes `cooked\\dib\\image\\Sinimage\\Items\\...bmp.dds`.
If no compatible cooked artifact exists, the original source path is loaded exactly as before. A cooked file older than its source is treated as stale and ignored.

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
DXT5 is chosen when the consumer-profiled image has semantic alpha; otherwise DXT1. Sources remain untouched.

The pre-existing Patch24 `item-icons-required` policy is preserved as an
**opt-in-to-cooking** rule during ordinary scans. Assets under
`image/Sinimage/Items/**` are not skipped merely for size; block-compatible
large images get alpha-aware DXT1/DXT5; smaller or non-block-compatible images
get A8R8G8B8. This does not prove every historical physical file is runtime
reachable. `--no-item-icons-required` retains the old threshold behavior for A/B.

For a **safe focused** inventory-icon cook (only BMP icons under
`image/Sinimage/Items/**` except `DropItem`, not the old broad physical-family
scan):

```bash
python3 /path/to/SourceCode/tools/cook_textures_dds.py --input . --item-icons-only
```

The incremental reuse check is stronger than Patch24: an existing DDS is
up-to-date only if manifest profile/revision, source SHA-256, source size,
DDS SHA-256, source freshness, DDS dimensions and DDS pixel format all match.
A stale, mismatched or pre-profile DDS is rebuilt even if its mtime is newer.
The old flat `cooked/foo.dds` path is never used by either loader.

To preview without writing:

```bash
python3 /path/to/SourceCode/tools/cook_textures_dds.py --input . --dry-run
```

**Do not use an unrestricted broad cook yet.** The material dependency graph,
41 historical collisions, unverified sources and excluded UI/sky families must
be resolved before any broad runtime publish. Use synthetic/test roots until
CLI gates pass, then a staged isolated `--item-icons-only` test; do not run it
against the production client before validating the handoff's acceptance gates.

To rebuild everything eligible (reference only; not authorized against runtime):

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

The cooker writes `cooked/manifest.json` with profile, profile revision,
semantic alpha, source and cooked SHA-256, source dimensions, pixel format and
cooked path. Reuse is conditional on this metadata. Runtime path lookup is
profile-qualified and deterministic and does not yet consume the manifest;
source fallback remains required during the transition.

## Validation order (no UI until all CLI gates pass)

1. Run `bash tools/run_cooked_images_cli_gates.sh` from `SourceCode`.
2. Run the existing streaming/media validations and `sh compboth.sh`.
3. Stage a small set of authoritative inventory BMPs in an isolated test tree.
4. Run the focused `--item-icons-only` cooker against that staging tree.
5. Only after those checks pass, use an explicit, limited Wine/runtime A/B.
6. To examine DIB cooked hits, set `SUMMIT_COOKED_DIB_TRACE=1` temporarily.

`SUMMIT_ASSET_TRACE=1` continues to report material `COOKED_DDS_HIT` events.

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


## Profiled Cooked Identity — Summit cooked-image bugfix (2026-10-10)

The legacy *logical filename* remains the request identity. The consumer owns
its color-key profile. The new destinations preserve the source extension:

| Consumer | Legacy request | New cooked path |
|---|---|---|
| smTexture / material | `Field/foo.bmp` | `cooked/material/Field/foo.bmp.dds` |
| smTexture / material | `Field/foo.tga` | `cooked/material/Field/foo.tga.dds` |
| Inventory DIB icon | `image/Sinimage/Items/WA/itwa101.bmp` | `cooked/dib/image/Sinimage/Items/WA/itwa101.bmp.dds` |

**Do not reuse older `cooked/Field/foo.dds`**. It could be the winner of a
`.bmp`/`.tga` name collision and is deliberately invisible to the new resolver.
Never delete quarantined or legacy assets as part of this patch.

### Decode and alpha contracts

`open_legacy_image_with_info()` decrypts/decodes to **authored RGBA only**.
`apply_cook_profile()` is a separate operation:

- **DIB**: BMP uses RGB `(0,0,0)` as a transparent key.
- **MATERIAL**: BMP uses RGB `(240,255,255)` as a transparent key.
- TGA/PNG retain their authored alpha for either consumer.
- Source alpha is combined with, not replaced by, the relevant BMP key.
- Large, aligned textures with semantic alpha use **DXT5**, not DXT1.
- Small or non-aligned, runtime-required inventory images use **A8R8G8B8**.

Profiles are assigned by verified consumer paths, **not by extension alone**.
Currently only `image/Sinimage/Items/<category>/*.bmp`, excluding
`DropItem`, is automatically classified as DIB. Other DIB consumers remain on
legacy source fallback until their reachability is recorded. ASE material
textures use the MATERIAL consumer; unusual `DropItem/DRboots.bmp` black-key
semantics remain under investigation rather than being auto-guessed.

### Cache and collision policy

The manifest records `cook_profile`, `profile_revision`, `semantic_alpha`,
`source_sha256`, `cooked_sha256`, dimensions and format. Incremental reuse
requires **all** these fields, a valid DDS header, matching file hashes and a
source that has not become newer than the DDS. Missing or legacy metadata
forces a new cook. All Windows case-insensitive destination collisions fail
closed *before writing any DDS or manifest*.

The runtime identity constructor and cooker use the same profile + extension
scheme. DIB tries cooked DDS first; a missing, stale or rejected DDS uses the
old source loader. DDS is imported without decrypting BMP or applying an
additional DIB color key. Existing `SUMMIT_TEXTURE_COOKED=0` can disable the
new runtime path for comparison. DIB telemetry is **off by default** to avoid
per-icon disk logging; set `SUMMIT_COOKED_DIB_TRACE=1` to emit
`DIB_COOKED_DDS_HIT`, `DIB_COOKED_REJECT` and `DIB_SOURCE_FALLBACK` while
`SUMMIT_ASSET_TRACE` is enabled.

### Required CLI gates (no UI)

From `SourceCode` run:

```bash
bash tools/run_cooked_images_cli_gates.sh
python3 tools/validate_async_streaming_v3.py
```

A *synthetic* cook is included in the regression suite. **Do not run a new
broad cook on `cliente/`** or rerun the old broad `--item-icons-only` script.
After CLI and MinGW build validation, start with a separate copied test
fixture containing a few known inventory BMPs and run the new focused command:

```bash
python3 tools/cook_textures_dds.py --input /path/to/test-client-assets --item-icons-only
```

Do not publish this test fixture to `cliente/cooked` until its manifest and
visual behavior have been checked. Never move source files, quarantine outputs
or enable runtime-only cooked under this patch. The long-term target is 100%
of **runtime-reachable** assets cooked, using canonical reachability and
consumer-role manifest ownership. Sky/fog, login legacy UI and historical
quarantine remain frozen for this intervention.


## PATCH 237 — Regressão de arranque / recuperação de DDS anteriores

A identidade nova `cooked/material/<legacy-file.ext>.dds` é incompatível com
DDS históricos `cooked/<legacy-file>.dds`. Quando os DDS material novos ainda
não existem, o runtime usa o source legacy e D3DX pode fazer decode/conversão
no arranque, com custo elevado de CPU/GPU. **O Game.exe não executa o cooker
Python**; esse decode no cliente pode parecer um cook em runtime.

O cliente evita agora procurar caminhos cozinhados em perfis sem diretório e
só testa o perfil DIB para ícones BMP de inventário comprovados.
`SUMMIT_COOKED_DIB=0` desativa apenas o novo caminho DIB para A/B;
`SUMMIT_TEXTURE_COOKED=0` desativa ambos os caminhos cooked. Reiniciar depois
de publicar DDS novos (o cache de existência de perfis é por processo).

**Recuperação offline, nunca no arranque do cliente**: executar com Game.exe
fechado e os DDS anteriores intactos, a partir de SourceCode:

```bash
python3 tools/restore_cooked_runtime_performance_v1.py --client ../cliente
python3 tools/restore_cooked_runtime_performance_v1.py --client ../cliente --apply
```

O primeiro comando é apenas auditoria sem escrita. O segundo copia somente
DDS históricos provados por fonte única (sem colisão case-insensitive),
formato e dimensões, timestamp, conteúdo visual e alpha compatíveis com o
perfil MATERIAL. Não converte imagens, não sobrescreve DDS novos, não move
assets legacy nem quarentenas. Assets ambíguos ou sem prova continuam em
source fallback até serem cozinhados offline sob controlo do manifest canónico.
O log final indica quantos DDS foram recuperados: zero candidatos recuperáveis
**não significa** recuperação de performance. Não usar o `--item-icons-only`
antigo contra o client vivo. A auditoria não valida ainda performance real:
comparar os mesmos mapas com logs de `COOKED_DDS_HIT` e tempos de frame.


## Streaming queue fairness and opt-in 60-frame diagnostics (PATCH 238)

`AssetLoadV3RequestTexture` has independent bounded counts for CPU preparation
(`preparing`) and GPU-ready staging (`ready`). The previous global-only
`in-flight` admission included GPU-ready uploads and could reject new HIGH
requests while worker threads were idle. LOW/NORMAL requests now stop when
the prepared GPU queue reaches `2 * SUMMIT_TEXTURE_IO_INFLIGHT`; HIGH requests
keep two extra slots. Total outstanding stays bounded by `3 * limit + reserve`
and staging remains hard-limited to `SUMMIT_TEXTURE_STAGING_BUDGET_MB` (128 MB
by default). Existing priority-ordered GPU completion remains intact.
The worker publishes the READY state **while holding the GPU-queue mutex**;
this closes a race where the render thread could pop/complete an entry before
the worker published READY, leaving a permanently pending texture slot.

To A/B the queue admission change without rebuilding, set
`SUMMIT_TEXTURE_QUEUE_FAIR=0` to restore the previous total-outstanding
admission limit; unset the variable to re-enable the improved separation.

The upload policy is intentionally **not increased**: gameplay remains at two
GPU commits per frame and a nominal 1800 us budget by default, because each
`D3DXCreateTexture...` commit may exceed that time and cause a stall.

Read-only offline gate:

```bash
cd SourceCode
python3 tools/validate_texture_queue_v1.py
```

Optional diagnostic during a *separate* runtime A/B:

```bash
cd cliente
SUMMIT_ASSET_TRACE=0 SUMMIT_TEXTURE_QUEUE_DIAG=1 SUMMIT_TEXTURE_COOKED=1 wine Game.exe
# After exit:
cd ../SourceCode
python3 tools/validate_texture_queue_v1.py --log ../cliente/LOGS/texture_queue_v3_diag.tsv
```

Only one `QUEUE_60F` line is appended per 60 frames; no per-texture diagnostic
I/O is introduced. Disable `SUMMIT_TEXTURE_QUEUE_DIAG` for normal play.
Setting `SUMMIT_ASSET_TRACE=0` independently silences legacy detailed event
files, whose per-DDS `fopen_s` + `fclose` instrumentation can affect Wine.
Do not interpret the number of historical `COOKED_DDS_HIT` lines as a current
session count. The 60-frame diagnostic contains separate counters for rejected
admissions (CPU/GPU/total), upload count, peak ready backlog, largest upload
time, and staging occupancy. This confirms or excludes queue pressure; it does
not by itself prove an FPS regression is caused solely by textures.
