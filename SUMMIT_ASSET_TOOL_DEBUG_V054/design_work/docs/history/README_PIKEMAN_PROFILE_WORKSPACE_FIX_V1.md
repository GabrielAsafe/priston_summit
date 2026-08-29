# PT Pikeman Profile + Workspace Fix V1

## Bootstrap result accepted

The real current-build Pikeman is:

```text
M4Bip.inx
TmbC01.smd
m4.smb
TmbC01.bmp
TmbC03.bmp
```

Observed structural inventory:

```text
objects=9
materials=2
bones=58

HIGH    TbhC01 TahC01
DEFAULT TbmC01 TamC01
LOW     TblC01 TalC01

C03 objects present but not selected:
TbhC03 TbmC03 TblC03
```

Texture mapping:

```text
material 1 -> TmbC01.bmp
material 0 -> TmbC03.bmp
```

## Bug fixed before registry

The bootstrap parsed importer display strings containing escaped backslashes and
generated:

```text
char//tmABCD//TmbC01.bmp
char//tmABCD//TmbC03.bmp
```

Those are not the canonical client targets.

`pt_pikeman_profile_fix_v1.py`:

- creates a verified profile backup;
- canonicalizes duplicate separators;
- verifies every required component exists in the client;
- records SHA and size for each component;
- does not modify the client.

## Fresh workspace

Do not reuse `blender_workspace_pikeman`; it is the legacy misnamed Priestess workspace.

Create:

```text
blender_workspace_pikeman_real
```

with an empty registry manifest, then let Character Asset `sync-workspace`
register only the five real Pikeman components.
