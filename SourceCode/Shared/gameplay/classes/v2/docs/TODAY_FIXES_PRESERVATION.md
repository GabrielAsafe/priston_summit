# Today Fixes Preservation Report

This source is based on the literal contents of `srcLastBugFix.zip`, not on the older Git history.

The archive differed from the previous bugfix baseline in exactly **23 files**. These files include the Tier 1-4/Tier 5 skill-animation fixes, model/motion loading, character runtime, particles/effects, audio and network paths.

## Mechanical preservation check

For every one of those 23 files, every non-trivial source line **added by today's fixes** was searched in the final integrated tree after applying Playable Class System V2.

Result: **ALL_RECENT_ADDITIONS_PRESERVED = YES**.

| File | Added lines checked | Missing | Result |
|---|---:|---:|---|
| `Shared/formats/model_config/LegacyModelConfigExact_client.inl` | 147 | 0 | **PASS** |
| `Shared/formats/model_config/LegacyModelConfigExact_server.inl` | 21 | 0 | **PASS** |
| `Shared/gameplay/character/character_legacy_client.inl` | 486 | 0 | **PASS** |
| `Shared/gameplay/combat/Svr_Damge_legacy_server.inl` | 1 | 0 | **PASS** |
| `Shared/gameplay/legacy_support/PlaySubHairSkinLegacyCommon.inl` | 13 | 0 | **PASS** |
| `Shared/gameplay/legacy_support/PlaySubLegacyShared.h` | 1 | 0 | **PASS** |
| `Shared/gameplay/legacy_support/playsub_legacy_client.h` | 1 | 0 | **PASS** |
| `Shared/gameplay/legacy_support/playsub_legacy_server.h` | 1 | 0 | **PASS** |
| `Shared/gameplay/skills/Morayion_legacy_client.inl` | 4 | 0 | **PASS** |
| `Shared/gameplay/skills/SkillSub_legacy_client.inl` | 354 | 0 | **PASS** |
| `Shared/gameplay/skills/SkillSub_legacy_server.inl` | 8 | 0 | **PASS** |
| `Shared/gameplay/skills/sinSkill_legacy_client.inl` | 0 | 0 | **PASS** |
| `Shared/rendering/effects/legacy_new/HoEffectMainLegacyCommon.inl` | 12 | 0 | **PASS** |
| `Shared/rendering/effects/legacy_new/HoEffectResMgrLegacyCommon.inl` | 8 | 0 | **PASS** |
| `Shared/rendering/effects/particles/HoNewParticleLegacyCommon.inl` | 1 | 0 | **PASS** |
| `Shared/rendering/effects/particles/HoNewParticleMgrLegacyCommon.inl` | 21 | 0 | **PASS** |
| `Shared/server/legacy_runtime/OnSever_legacy_server.inl` | 12 | 0 | **PASS** |
| `SrcGame/src/core/client/legacy_sin_runtime/sinSubMain.cpp` | 1 | 0 | **PASS** |
| `SrcGame/src/core/data/legacy_loader/SkillDataCode.inl` | 2 | 0 | **PASS** |
| `SrcGame/src/systems/audio/effects/legacy/effectsnd.cpp` | 8 | 0 | **PASS** |
| `SrcGame/src/systems/combat/skills/client/SkillSub.h` | 2 | 0 | **PASS** |
| `SrcGame/src/systems/network/client/netplay.cpp` | 12 | 0 | **PASS** |
| `SrcServer/src/core/data/legacy_loader/SkillDataCode.inl` | 2 | 0 | **PASS** |

## Overlap audit

Only one recent-fix area overlaps directly with a Class V2 rewrite: `Shared/gameplay/legacy_support/PlaySubHairSkinLegacyCommon.inl`. The bounds validation added today for `FACE_MODEL_MAX` / `HEAREPOTION_MAX` is retained before the V2 appearance resolver.

The Tier skill/effect files that differ today remain based on `srcLastBugFix` and were not replaced with older versions. The V2 changes around them are routing/metadata decoupling only.

## Validation performed

- `run_class_system_tests.sh`: PASS
- playable class registry smoke: PASS
- legacy quest adapter smoke: PASS
- legacy commerce adapter smoke: PASS
- legacy onboarding adapter smoke: PASS
- legacy event-kit adapter smoke: PASS
- class architecture guard: PASS
- client build script shell syntax: PASS
- server build script shell syntax: PASS

Full MinGW i686 compile/link and in-game Tier 1-5 animation regression still need to run in the native Windows toolchain.
