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

By default the cooker mirrors the current runtime compression policy: only
textures whose uncompressed RGBA footprint is at least 1 MiB (512x512 RGBA) and whose top
level is DXT block-compatible are emitted. This threshold is intentionally lower than the
old 16 MiB rollout because runtime traces showed 512x512 and 1024x1024 source uploads
already causing visible frame stalls. PNG/TGA/STA use DXT5; opaque
JPG/BMP/STG use DXT1. Sources remain untouched.

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
