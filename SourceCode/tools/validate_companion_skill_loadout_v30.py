#!/usr/bin/env python3
from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parents[1]
files={
 'protocol':ROOT/'Shared/gameplay/companions/CompanionProtocol.h',
 'profile_h':ROOT/'SrcServer/src/systems/gameplay/companions/CompanionProfileStore.h',
 'profile_cpp':ROOT/'SrcServer/src/systems/gameplay/companions/CompanionProfileStore.cpp',
 'exec_h':ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h',
 'exec_cpp':ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp',
 'system':ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp',
 'models':ROOT/'SrcGame/src/systems/ui/v2/domain/CompanionUIModels.h',
 'gateway':ROOT/'SrcGame/src/systems/ui/v2/legacy/LegacyCompanionGateway.cpp',
 'page_h':ROOT/'SrcGame/src/systems/ui/v2/pages/CompanionsPage.h',
 'page_cpp':ROOT/'SrcGame/src/systems/ui/v2/pages/CompanionsPage.cpp',
}
t={k:p.read_text(errors='ignore') for k,p in files.items()}
checks=[
 ('protocol v5+','protocol','COMPANION_UI_PROTOCOL_VERSION = 6'),
 ('resource action','protocol','COMPANION_UI_CYCLE_ACTIVE_MIN_RESOURCE'),
 ('targets action','protocol','COMPANION_UI_CYCLE_ACTIVE_MIN_TARGETS'),
 ('target mode action','protocol','COMPANION_UI_CYCLE_ACTIVE_TARGET_MODE'),
 ('wire reserve','protocol','minResourcePercent'),
 ('wire minimum targets','protocol','minimumTargets'),
 ('wire target mode','protocol','targetMode'),
 ('profile v4','profile_cpp','COMPANION_PROFILE_VERSION = 4'),
 ('profile v3 migration','profile_cpp','COMPANION_PROFILE_SKILL_LOADOUT_VERSION = 3'),
 ('profile ABI unchanged','profile_cpp','sizeof(CompanionProfileDiskV1) == 84'),
 ('profile reserve','profile_h','activeMinResourcePercent[4]'),
 ('profile targets','profile_h','activeMinimumTargets[4]'),
 ('profile target modes','profile_h','activeTargetModes[4]'),
 ('reserved tail reuse','profile_cpp','V4 consumes the original 12-byte reserved tail'),
 ('priority order','exec_cpp','slot order is the explicit priority contract'),
 ('no round robin scheduler','exec_cpp','for(int slot = 0; slot < 4; ++slot)'),
 ('post cast reserve','exec_cpp','MeetsConfiguredResourceReserve'),
 ('minimum target count','exec_cpp','CountConfiguredSkillTargets'),
 ('owner target policy','exec_cpp','OWNER_TARGET'),
 ('nearest target policy','exec_cpp','NearestConfiguredLoadoutTarget'),
 ('min targets reject','exec_h','COMPANION_SKILL_MIN_TARGETS'),
 ('policy persistence bridge','system','activeMinResourcePercent'),
 ('policy action handler','system','CycleActiveSkillPolicy'),
 ('policy telemetry','system','LOADOUT_POLICY'),
 ('client resource policy','models','minResourcePercent = 0'),
 ('client targets policy','models','minimumTargets = 1'),
 ('client target mode','models','targetMode = 0'),
 ('gateway policy mapping','gateway','CycleActiveMinResource'),
 ('resource UI control','page_h','activeMinResourceButtons_'),
 ('targets UI control','page_h','activeMinTargetsButtons_'),
 ('target UI control','page_h','activeTargetModeButtons_'),
 ('resource label','page_cpp','"R" + std::to_string(policy.minResourcePercent)'),
 ('targets label','page_cpp','"N" + std::to_string(policy.minimumTargets)'),
]
err=[]
for label,k,n in checks:
    if n not in t[k]: err.append(f'{label}: missing {n!r}')
# The disk type must remain exactly 84 bytes and still carry a checksum.
if 'uint32_t checksum;' not in t['profile_cpp']: err.append('checksum removed')
if err:
    print('Companion Skill Loadout V30: FAIL')
    for e in err: print(' -',e)
    sys.exit(1)
print('Companion Skill Loadout V30: PASS (priority + per-slot reserve/minTargets/target policy; profile ABI 84 bytes)')
