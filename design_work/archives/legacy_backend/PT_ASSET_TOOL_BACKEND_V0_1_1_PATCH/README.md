# PT Asset Tool Backend v0.1.1 patch

Fixes discovered during the first real validation session.

## Fix 1 — protected TGA in rigid/static Blender imports

The historical rigid importer V3 decodes proprietary BMP but passes `.tga` through as-is.
The backend now builds a temporary texture overlay under `pt_build`:

- protected/standard TGA -> `pt_protected_tga_v1.py decode` -> editable standard TGA;
- other texture files -> symlink to the real client source;
- importer receives the overlay as `--client-root`;
- real client is never modified.

An automatic `TEXTURE_PREPARE` certification is emitted.

## Fix 2 — automatic LOD selection

`validate visual` now defaults to `--lod auto`.
For rigid INX it chooses the first available group in this order:

1. default
2. high
3. low

An explicitly requested empty LOD is rejected instead of silently hiding every mesh.
This fixes assets such as Acero, whose INX has only a `high` group.

## Fix 3 — visual certification split

After Blender closes the user is asked separately:

- geometry/witness correct?
- textures/materials correct?

The backend records:

- `VISUAL_GEOMETRY`
- `VISUAL_TEXTURE_MATERIAL`
- `VISUAL_ASSET` (derived overall result)

`assets list` now shows GEOM, TEX/MAT and VISUAL separately.

## Fix 4 — manual append-only certification

New command:

```bash
python3 pt_asset.py certification record <asset> \
  --test <TEST> --result PASS|FAIL|SKIP \
  --notes '...'
```

Records are appended; previous evidence is never deleted.
