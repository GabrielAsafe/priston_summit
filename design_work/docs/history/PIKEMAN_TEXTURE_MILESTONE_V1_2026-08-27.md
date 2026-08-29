# Summit Tale Blender Tools — Pikeman Texture Milestone V1
## 2026-08-27

## Status

The real current-build Pikeman identity is:

```text
M4Bip.inx
TmbC01.smd
m4.smb
TmbC01.bmp
TmbC03.bmp
```

Validated structural inventory:

```text
objects=9
materials=2
bones=58

HIGH    TbhC01 TahC01
DEFAULT TbmC01 TamC01
LOW     TblC01 TalC01
```

Texture mapping:

```text
material 1 -> TmbC01.bmp
material 0 -> TmbC03.bmp
```

## Runtime-proven texture workflow

For `TmbC01.bmp`, the following is now runtime-proven:

```text
registered protected BMP
→ standard editable BMP
→ Blender material binding
→ artist-visible pixel edit
→ protected game BMP rebuild
→ non-pixel bytes preserved
→ workspace publication
→ dry-run
→ verified backup/deploy
→ runtime
```

User confirmed the Pikeman loaded correctly in-game with the changed texture.

Official status:

```text
PIKEMAN TmbC01 TEXTURE WORKFLOW = RUNTIME PROVEN
```

## Safety invariants retained

```text
artist texture != game texture
client writes only through Restore V3
dry-run before apply
backup + transaction before write
client drift refusal remains strict
baseline source remains immutable
```

## Next milestone

1. restore the original `TmbC01.bmp`;
2. prove the same workflow for `TmbC03.bmp` (`role=texture_variant`);
3. generalize the Character Asset texture interface to all mapped character textures;
4. then move beyond image pixels into SMD material properties.
