# Summit runtime release packaging

The release package contains only standalone runtime files from `cliente/` and
`Servidor/`. Source code and canonical JSON remain in `SourceCode/` and are not
distributed.

## Files intentionally excluded

- `cliente/LOGS/**`
- `Servidor/LOGS/**`
- `*.log`, `*.dmp`, `*.tmp`
- `cliente/LoadItemImage_debug.txt`
- `cliente/content/maps/maps_client.smap` (obsolete lower-case duplicate; active map pack is `cliente/Content/packs/maps_client.smap`)

Additional exclusions can be passed with `--exclude`.

## First/base release

```bash
cd ~/Área\ de\ Trabalho/summit
mkdir -p SourceCode/packaging/checkpoints releases

python3 SourceCode/tools/runtime_release_packager.py full \
  --repo-root . \
  --output releases/SUMMIT_RUNTIME_BASE.zip \
  --checkpoint SourceCode/packaging/checkpoints/runtime_checkpoint.json
```

The checkpoint is the exact SHA-256 snapshot represented by that base package.
Keep it in SourceCode/version control.

## See what changed

```bash
python3 SourceCode/tools/runtime_release_packager.py status \
  --repo-root . \
  --checkpoint SourceCode/packaging/checkpoints/runtime_checkpoint.json
```

## Build a delta

```bash
python3 SourceCode/tools/runtime_release_packager.py delta \
  --repo-root . \
  --checkpoint SourceCode/packaging/checkpoints/runtime_checkpoint.json \
  --output releases/SUMMIT_RUNTIME_DELTA.zip
```

Inspect/test the delta. Once that delta is accepted as the new release baseline,
advance the checkpoint by rebuilding the same delta with `--advance-checkpoint`:

```bash
python3 SourceCode/tools/runtime_release_packager.py delta \
  --repo-root . \
  --checkpoint SourceCode/packaging/checkpoints/runtime_checkpoint.json \
  --output releases/SUMMIT_RUNTIME_DELTA.zip \
  --advance-checkpoint
```

Do not advance the checkpoint before the release is accepted, otherwise the
next delta will no longer include the unshipped changes.

## Apply a delta

Place `apply_runtime_delta.py` next to the target installation root and run:

```bash
python3 apply_runtime_delta.py \
  --repo-root . \
  --delta SUMMIT_RUNTIME_DELTA.zip
```

The applier extracts only paths listed in the signed-by-hash manifest, removes
paths listed as deleted, then validates each written file's SHA-256.

## Cooked assets

`cooked/` is currently an optimized DDS fast path, not a universal replacement
for all authored assets. The client can fall back to original textures and some
interactive UI paths intentionally bypass cooked DDS. Therefore a fresh/base
client should not remove whole asset directories merely because a cooked copy
exists. Use the full package as the base, then incremental deltas keep future
releases small without risking missing source assets.
