# Summit Tale Blender Tools — Texture Workflow Addendum v1
## 2026-08-27

## Evidence recovered before implementation

For tested protected BMPs including `MfbC01.bmp`, the first 14 bytes replace the
normal Windows `BITMAPFILEHEADER`; starting at offset 14 there is a normal
40-byte `BITMAPINFOHEADER`.

The already-proven decode path was:

```text
save proprietary bytes 0..13
replace them with a standard BMP file header
preserve bytes 14..end
edit standard BMP
restore proprietary bytes 0..13
```

The new production implementation is stricter. It treats the registered game
texture as immutable baseline and copies **only the edited pixel payload** into
that baseline. Consequently:

```text
proprietary prefix       preserved exactly
DIB header               preserved exactly
padding/layout           preserved exactly
non-pixel trailing data  preserved exactly
artist pixels            replace pixel payload only
```

The first gate is a no-op roundtrip:

```text
game protected BMP
→ standard editable BMP
→ game BMP
```

and must be byte-identical before artistic edits are allowed.

## Current scope

Protected BMP support is deliberately limited to the family already proven:

```text
DIB size = 40
256 x 256
24 bpp
BI_RGB
```

The tool must refuse other protected layouts rather than guessing.

Standard BMP is also recognized, which prepares later support for `mo-p-ac.BMP`.

## Character Asset integration

Character Asset V1.2 adds:

```text
texture-inspect
texture-prepare
texture-bind-blend
texture-build
texture-publish
texture-status
texture-deploy-plan
texture-deploy
```

Initial role:

```text
role=texture
AST-B5C43FFBE0EE
char/tmABCD/MfbC01.bmp
material slots 1,2
```

## Next validation

Run inspect and prepare against the real registered MfbC01 texture.

Do not publish/deploy until:

```text
texture_inspect_check=PASS
texture_prepare_check=PASS
noop_byte_identical=True
```
