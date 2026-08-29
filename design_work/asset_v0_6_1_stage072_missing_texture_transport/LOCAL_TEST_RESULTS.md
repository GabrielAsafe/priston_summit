# V0.6.1 local test results

## Compile/install

- `python3 -m compileall`: PASS
- install script over V0.6.0 test tree: PASS
- installed version: `0.6.1`
- `pt_asset audit stage072-textures --help`: PASS

## Stage0.72 parser regression

Reparsed the 33 physical Stage0.72 representatives from `PT_STAGE072_REPS_V060.zip`:

- audited: 33/33
- full parse: 33/33 PASS
- structural fail: 0
- depth distribution: 1=6, 2=24, 3=3
- area: DWORD32=29, DWORD32_TRAILING_OPAQUE=3, WORD16=1

## quest_IV physical evidence

Parsed `Field/Quest/quest_IV.smd` directly:

- material 6 is referenced by 67 faces
- TextureCounter=2
- stage 0 texture: `field\\quest\\fw.bmp`
- stage 1 texture: `field\\quest\\ir_m_eff1.bmp`
- therefore `fw.bmp` is not an unused historical material reference

## Texture-presence audit unit test

Synthetic client built from physical `quest_IV.smd` + `office.smd`, with every
face-material-scoped texture represented except `field/quest/fw.bmp`:

- audit status: PASS
- missing unique references: 1
- file: `Field/Quest/quest_IV.smd`
- material: 6
- slot: BASE/0
- face_uses_sum: 67

After adding a dummy `field/quest/fw.bmp`:

- missing unique references: 0

## Transport policy tests

- `MISSING` required texture => transport_may_continue=true, material_preview_complete=false
- existing-file decode failure => transport_may_continue=false

Blender/Flatpak execution is intentionally left for the real client machine.
