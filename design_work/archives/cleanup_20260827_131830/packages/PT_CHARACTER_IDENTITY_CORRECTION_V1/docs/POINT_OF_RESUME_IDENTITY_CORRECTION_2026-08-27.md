# POINT OF RESUME — Identity correction
## 2026-08-27

Critical correction:

```text
MfbC01.smd + m5.smb + M5Bip.inx = PRIESTESS
NOT PIKEMAN
```

Evidence:
- historical/source mapping: m4 -> Pikeman, m5 -> Priestess;
- visual validation of MfbC01 with M5Bip/m5 was recognized as Priestess;
- Default/High/Low C01 groups belong to that Priestess model-info.

All prior geometry runtime proofs remain valid for the physical Priestess asset.
Do not discard them. The mistake was identity labeling.

Texture runtime state:
- protected BMP inspect: PASS
- decode/prepare no-op byte-identical: PASS
- Blender material-slot binding: PASS
- red game BMP build: PASS
- red publish to workspace: PASS
- red deploy-plan: PASS
- red deploy to client: NOT EXECUTED
- source/no-op texture was republished before apply
- therefore red runtime visual is NOT TESTED

Current client MfbC01.bmp remained/restored at registered source SHA:
`4c97f923db485407b3fe1fcd77e7c36ada36ea2e22255b93937332ada3ad4e18`.

Next action:
discover the real Pikeman using `pt_character_identity_discover_v1.py`
with `--motion-stem m4`.

Do not continue texture work under the deprecated `pikeman_c01_m5` profile.
