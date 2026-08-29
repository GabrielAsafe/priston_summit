# Pikeman Armor Family Fix V1

## Failure diagnosed

C003 imported successfully:

```text
objects=18
materials=4
bones=58
static_armature_import_check=PASS
```

Its material 0 is intentionally textureless:

```text
MATERIAL 0: tex='' ... decode=NOT_FOUND load=NO_TEXTURE
```

V1 incorrectly sent the empty string through `normalize_rel()`, which is
supposed to reject unsafe/empty filesystem paths.

That was a catalog parser bug, not an SMD/import/runtime failure.

## V1.1 fix

For material inventory only:

```text
tex='' -> texture=None
```

Non-empty texture paths still pass through the same strict `normalize_rel()`
gate.

No geometry, SMD, texture codec, client-write, or validation policy was
weakened.

## Rerun

Existing C001/C002/C003 preview artifacts are intentional, so rerun with
`--overwrite`:

```bash
python3 pt_pikeman_armor_family_v1_1.py \
  --client-root "/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente" \
  --overwrite
```

Expected:

```text
PT_PIKEMAN_ARMOR_FAMILY_V1_1
config_count=5
armor_family_check=PASS
client_modified=False
```
