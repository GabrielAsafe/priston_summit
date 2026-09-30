#!/usr/bin/env python3
from pathlib import Path
import re, sys

ROOT = Path(__file__).resolve().parents[1]
CATALOG = ROOT / 'Shared/gameplay/skills/v2/content/LegacySkillCatalog.cpp'
SPAWN = ROOT / 'Shared/server/legacy_runtime/OnSever_legacy_server.inl'
SYSTEM = ROOT / 'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp'
EXEC = ROOT / 'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp'

err=[]
cat=CATALOG.read_text(encoding='utf-8', errors='ignore')
spawn=SPAWN.read_text(encoding='utf-8', errors='ignore')
system=SYSTEM.read_text(encoding='utf-8', errors='ignore')
exe=EXEC.read_text(encoding='utf-8', errors='ignore')
pat=re.compile(r'MakeSkill\((0x[0-9A-Fa-f]+)u,\s*(\d+),\s*(\d+),\s*(\d+),\s*SkillBehavior::([A-Za-z]+),\s*(true|false),')
rows=pat.findall(cat)
active=[r for r in rows if r[5]=='true']
passive=[r for r in rows if r[4]=='Passive']
if len(rows) != 160: err.append(f'canonical catalog size {len(rows)} != 160')
if len(active) != 142: err.append(f'expected 142 executable skills, got {len(active)}')
if len(passive) != 18: err.append(f'expected 18 passives, got {len(passive)}')
if any(r[5] != 'false' for r in passive): err.append('passive entry is directly executable')

for needle in [
    'memcpy(&stageInfo.smCharInfo, &recordData->smCharInfo, sizeof(smCHAR_INFO));',
    'memcpy(&companion->smCharInfo, &stageInfo.smCharInfo, sizeof(smCHAR_INFO));',
]:
    if needle not in spawn: err.append('spawn contract missing: '+needle)
for needle in [
    'ReadCompanionLogicalSkillPoints(recordData, logicalSkillPoints, COMPANION_MAX_SKILL_SLOTS);',
    'LoadCompanionLogicalSkillPoints(slot.skillRuntime, logicalSkillPoints, COMPANION_MAX_SKILL_SLOTS);',
    'ApplyCompanionPassiveSkills(entity, slot.skillRuntime);',
]:
    if needle not in system: err.append('persisted skill-point/passive load missing: '+needle)
for needle in [
    'ZeroMemory(runtime.skillPoints, sizeof(runtime.skillPoints));',
    'runtime.skillPoints[useSlot - 1]',
    'DefaultLegacySkillAdapter().SkillsForClass',
    'if(!entry.IsPassive() || entry.useSlot < 1)',
]:
    if needle not in exe: err.append('canonical passive runtime missing: '+needle)

if err:
    print('Companion Passive Audit V29: FAIL')
    for e in err: print(' -',e)
    sys.exit(1)
print('Companion Passive Audit V29: PASS (160 canonical / 142 executable / 18 passive; source-based audit)')
