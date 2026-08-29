# Summit Tale — Production Automation Findings v12.3
## Local investigation of supplied M1…M7 roots — 2026-08-27

## 1. Scope available locally

The supplied package contains the active toolchain, profiles, similarity/refine
reports, and physical:

```text
M1Bip.inx .. M7Bip.inx
m1.smb .. m7.smb
```

It does **not** contain the raw representative mob/NPC/weapon SMDs or their
textures. Therefore exact binary import/rebuild tests for those representatives
remain pending. A corrected collector is included.

The prior collection package's `SHA256SUMS.txt` also listed itself while it was
being generated; therefore only that self-entry fails verification. This is a
collector bug, not evidence of asset corruption. The new collector excludes
`SHA256SUMS.txt` from its own checksum list.

---

## 2. Seven root chains

Observed physical root configs:

```text
M1Bip -> MmbA01 + m1
M2Bip -> TfbD01 + m2
M3Bip -> MmbD01 + m3
M4Bip -> TmbC01 + m4
M5Bip -> MfbC01 + m5
M6Bip -> TfbE02 + m6
M7Bip -> MmbE31 + m7
```

The existing source-derived scanner table maps family letters:

```text
non-Moryon: A Mechanician, B Fighter, C Pikeman, D Archer, E Assassin
Moryon:     A Knight, B Atalanta, C Priestess, D Magician, E Shaman
```

Crossing root model family with that table yields semantic candidates:

```text
M1 Knight
M2 Archer
M3 Magician
M4 Pikeman
M5 Priestess
M6 Assassin
M7 Shaman
```

Evidence policy:

- M4/Pikeman and M5/Priestess: independently established earlier.
- M1/M2/M3/M6/M7: source-table + physical-chain corroborated; not yet promoted
  to runtime identity proof.
- Mechanician/Fighter/Atalanta are **not** assigned a root here. Do not guess.

---

## 3. Skeleton family result

All seven SMB files parse as valid Model0.62 skeleton containers, have one
`Bip01` root, resolve all parents, and build a valid parent-first `TmSort`.

Bone counts:

```text
m1  64
m2  67
m3 102
m4  58
m5  95
m6  70
m7  50
```

The seven share a universal 43-bone Biped/input/weapon core. Differences are
mostly class-specific accessory/attachment bones.

Correct architecture:

```text
UNIVERSAL BIPED CORE
+ ROOT-SPECIFIC ATTACHMENT BONES
```

not seven unrelated skeleton formats.

---

## 4. Animation storage result

Serialized key totals reveal two content subfamilies:

```text
M1..M5  SPARSE_TRACKS
M6/M7   DENSE_TRS
```

M6 and M7 store rotation/position/scale densely for the skeleton, while
M1–M5 are mainly sparse rotation/position with little/no scale.

This is a storage/content distinction inside the same SMB format.

---

## 5. Correction to old segment validators

The old animation track inventory rejected negative `PosNum` even when
`PosCnt <= 0`.

That is not source-faithful.

The engine's `GetTmFrame*` first requires:

```text
PosCnt > 0
StartFrame <= frame
EndFrame > frame
```

Only then is `PosNum` used. Therefore inactive records may retain legacy garbage
such as `0xCCCCCCCC`.

A later V2 fixed that rule but added additional assumptions that active segment
key slices must themselves cover StartFrame/EndFrame and partition all keys.
Those assumptions are not required by `GetRotFrame/GetPosFrame/GetScaleFrame`,
which start at the selected PosNum and search the global key array.

New V3 policy:

```text
for representative frames inside every active segment:
    reproduce GetTmFrame selection
    run source-equivalent GetRot/GetPos/GetScale lookup
    fail only when the source-equivalent lookup becomes invalid
```

This is stricter with respect to actual engine accesses and removes invented
format constraints.

---

## 6. Source-equivalent motion core

The existing source-equivalent evaluator was exercised locally on M1…M7
MotionInfo[10]. Structural evaluation succeeds for all seven roots, including
dense M6/M7.

Conclusion:

```text
SMB motion mathematics core
= structurally reusable across M1..M7
```

This is not a visual runtime certification of every class.

---

## 7. Production auto-profile generator

Implemented:

```text
pt_auto_profile_production_v1.py
```

Strict V1 input:

```text
resolved INX
+ physical Model0.62 SMD
+ exactly one physical SMB
```

Config-only models without an SMB are rejected; no class skeleton is guessed.

It emits the same major schema used by current Pike/Priestess production
profiles:

```text
profile_id
asset_set_id
model_smd + SHA
skeleton_smb + SHA
model_info_inx + SHA
roundtrip_target
lod_groups
known_mesh_objects
baseline_counts
components
geometry_status
identity_evidence
```

Serialized material strings are decoded to create texture components with their
material-slot IDs.

Generated profiles explicitly remain:

```text
runtime_certified = false
```

until representative testing closes the family.

---

## 8. Generic workspace initialization

Old `pt_character_workspace_init_v1.py` hardcoded:

```text
profile_id == pikeman_c01_m4
```

New:

```text
pt_character_workspace_init_v2.py
```

removes that identity restriction and validates generic required profile fields.

---

## 9. Character asset wrapper bridge correction

`pt_character_asset_v1_4.py` referenced
`summit_workspace_asset_bridge_v1_1.py` in two paths although V1.2 is the active
hardened bridge.

New:

```text
pt_character_asset_v1_5.py
```

switches those bridge references to V1.2 without changing validator semantics.

---

## 10. Static/no-Physique path

Inspection proved the existing rigid pipeline was not yet a weapon/static
authoring pipeline:

```text
free-mesh writer V3/V4   requires Physique
free-mesh validator      requires rigid vertex groups
V3 bind importer         expects skeleton/Physique for final armature path
```

But static Model0.62 shares the same:

```text
container
smVERTEX
smFACE
smTEXLINK
material group
```

and differs principally by:

```text
Physique pointer = 0
no per-vertex 32-byte bone-name payload
```

Implemented first strict branch:

```text
pt_smd_static_capability_probe_v1.py
pt_smd_blender_import_v4_generic.py
pt_blender_to_smd_static_v1.py
```

V1 eligibility requires:

```text
no Physique
no object-local TmRot/TmPos/TmScale
valid face indices
one TexLink layer
3..30000 vertices
1..30000 faces
```

The static importer reuses the proven V3 Model0.62/material/mesh parsing but
skips bind reconstruction and armature creation.

The static writer maps Blender positions directly to Model0.62 fixed point:

```text
SMD XYZ = round(Blender XYZ * 256)
```

and rebuilds vertices/faces/TexLinks while preserving non-target object blocks.

Because the supplied package omitted actual weapon SMD files, these Blender
static tools are **implemented but not yet certified on a real weapon**.

---

## 11. Missing representative collector

The previous collector joined inventory `relative_path` to the Linux client
root. Case differences caused representative files to be missed.

New collector:

```text
collect_missing_representatives_v1.sh
```

uses the exact physical absolute path recorded by the inventory and collects:

```text
Monbagon
Monshadow
Vampiric

SN-002
MN-005
MN-009

itPR101
itWA104
itWH118
```

plus referenced textures and paired INX/SMB.

---

## 12. Remaining local gates after representatives arrive

1. Generate production profiles for real self-contained mob/NPC.
2. Run direct importer/reimport structural checks on them.
3. Run static capability probe on simple/medium/complex weapon representatives.
4. Run static import + no-op writer and verify semantic/no-op behavior.
5. Generalize protected TGA from real representative headers/payloads.
6. Only then request client runtime tests for representative families.

This keeps runtime tests representative rather than per-asset.
