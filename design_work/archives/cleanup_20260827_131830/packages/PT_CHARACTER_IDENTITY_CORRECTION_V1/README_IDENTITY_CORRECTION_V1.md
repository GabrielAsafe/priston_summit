# PT Character Identity Correction V1

## Correction

The runtime-proven asset set:

```text
MfbC01.smd
m5.smb
M5Bip.inx
MfbC01.bmp
MfbC03.bmp
mo-p-ac.BMP
```

is **Priestess**, not Pikeman.

The previous toolchain label `pikeman_c01_m5` was wrong. The geometry/texture
research itself remains valid for the physical files; only the class identity
label was incorrect.

Do not delete or rewrite old manifests/builds. They are historical evidence and
their hashes remain meaningful.

A corrected profile is supplied:

```text
profiles/pt_character_profile_priestess_c01_m5_v1.json
profile_id=priestess_c01_m5
asset_set_id=CHARSET-PRIESTESS-C01-M5
```

The existing workspace named `blender_workspace_pikeman` should currently be
treated as a **legacy misnamed Priestess workspace**. This package intentionally
does not rename or mutate it.

## Why the red runtime test did not show red

The changed texture was published to the workspace and `deploy-plan` passed, but
the modified texture was never applied to the client. Before `deploy --apply`,
the original/no-op texture was built and republished. Therefore normal game
color was expected.

The `textureclear` error was a command typo and was safely rejected before any
write.

## Discover the actual Pikeman

Historical/source material maps Pikeman motions to `m4`, but filenames alone are
not sufficient. Resolve the current build through its INX:

```bash
python3 pt_character_identity_discover_v1.py \
  --client-root "/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente" \
  --motion-stem m4 \
  --report logs/pikeman_identity_discovery_v1.json
```

The scanner reads only the first three `smMODELINFO` string fields:

```text
0x00 szModelFile
0x40 szMotionFile
0x80 szSubModelFile
```

and only selects INX entries whose motion basename is exactly `m4`.

For each candidate it verifies:

```text
INX
derived .smd from szModelFile
derived .smb from szMotionFile
SHA/size
texture-like strings inside the SMD
```

No file is modified.

Do not create a Pikeman profile until there is at least one
`COMPLETE_PHYSICAL_CHAIN` candidate and its visual identity is confirmed.
