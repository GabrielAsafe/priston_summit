# POINT OF RESUME — real Pikeman bootstrap

Real Pikeman chain discovered and imported successfully:

```text
M4Bip.inx SHA b8bb4adaaa2175b57fa604ccdacf419ca699d48d47bb04710e6169189de2f192
TmbC01.smd SHA 9c5b137bc637f485419cafae6864c4a37f77835d32210810295c1b58c746443c
m4.smb     SHA 48af614678593eaa0b659c2be514bd60f86b94f72cfb4305ed8352f48f12cc4a
```

Importer:
```text
objects=9
materials=2
bones=58
static_armature_import_check=PASS
```

LOD:
```text
HIGH    TbhC01 TahC01
DEFAULT TbmC01 TamC01
LOW     TblC01 TalC01
```

Textures:
```text
material 1 -> char/tmABCD/TmbC01.bmp
material 0 -> char/tmABCD/TmbC03.bmp
```

Three C03 objects exist but are not in M4Bip.inx LOD groups:
```text
TbhC03 TbmC03 TblC03
```

Bootstrap generated profile paths with duplicated slashes for textures because
the importer log displays escaped backslashes. Fix profile before registry.

Next commands:
1. `pt_pikeman_profile_fix_v1.py`
2. `pt_character_workspace_init_v1.py`
3. Character Asset `inspect`
4. Character Asset `sync-workspace`

Do not reuse legacy `blender_workspace_pikeman`; it belongs to Priestess history.
