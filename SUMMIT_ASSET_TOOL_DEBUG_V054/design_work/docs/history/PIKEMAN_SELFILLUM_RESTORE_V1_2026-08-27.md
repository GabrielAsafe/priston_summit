# Pikeman SelfIllum Restore V1

Runtime result:

```text
TmbC01.smd material 0
SelfIllum 0.0f -> 1.0f
C002 / Battle Suit chest rendered with the expected self-illumination effect
runtime visual = PASS
```

Official gate:

```text
PIKEMAN smMATERIAL::SelfIllum = RUNTIME PROVEN
```

This package creates a byte-identical restore artifact from the immutable local
registered source `TmbC01.smd`.

Run:

```bash
python3 pt_pikeman_selfillum_restore_build_v1.py
```

Required:

```text
artifact_sha256=9c5b137bc637f485419cafae6864c4a37f77835d32210810295c1b58c746443c
changed_byte_count=0
noop_build=True
byte_identical_to_registered_source=True
build_check=PASS
```

Then publish using `--replace-published`, dry-run with Restore V4, apply, and
verify the client SHA equals the registered source.
