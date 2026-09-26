#!/usr/bin/env python3
from pathlib import Path
import csv, re, sys

ROOT = Path(__file__).resolve().parents[1]
CSV = ROOT / 'docs' / 'COMPANION_SKILL_CATALOG_ZENIT_2026-09-24.csv'
DOC = ROOT / 'docs' / 'COMPANION_PASSIVE_AUDIT_V29_2026-09-26.md'
SPAWN = ROOT / 'Shared' / 'server' / 'legacy_runtime' / 'OnSever_legacy_server.inl'
SYSTEM = ROOT / 'SrcServer' / 'src' / 'systems' / 'gameplay' / 'companions' / 'CompanionSystem.cpp'
EXEC = ROOT / 'SrcServer' / 'src' / 'systems' / 'gameplay' / 'companions' / 'CompanionSkillExecutor.cpp'

err=[]
rows=list(csv.DictReader(CSV.open(encoding='utf-8')))
if len(rows) != 128:
    err.append(f'catalog size {len(rows)} != 128')
not_ready=[r for r in rows if r['companion_adapter'] != 'READY']
if len(not_ready) != 16:
    err.append(f'expected 16 non-READY entries, got {len(not_ready)}')
non_passive=[r['code'] for r in not_ready if r['role'] != 'PASSIVE']
if non_passive:
    err.append('non-passive entries remain non-READY: '+','.join(non_passive))
wrong_state=[r['code'] for r in not_ready if r['companion_adapter'] != 'CATALOG_ONLY']
if wrong_state:
    err.append('passives use unexpected adapter state: '+','.join(wrong_state))

spawn=SPAWN.read_text(encoding='utf-8', errors='ignore')
system=SYSTEM.read_text(encoding='utf-8', errors='ignore')
exe=EXEC.read_text(encoding='utf-8', errors='ignore')
doc=DOC.read_text(encoding='utf-8', errors='ignore') if DOC.exists() else ''

for needle in [
    'memcpy(&stageInfo.smCharInfo, &recordData->smCharInfo, sizeof(smCHAR_INFO));',
    'memcpy(&companion->smCharInfo, &stageInfo.smCharInfo, sizeof(smCHAR_INFO));',
]:
    if needle not in spawn:
        err.append('spawn contract missing: '+needle)
for needle in [
    'ReadCompanionLogicalSkillPoints(recordData, logicalSkillPoints, COMPANION_MAX_SKILL_SLOTS);',
    'LoadCompanionLogicalSkillPoints(slot.skillRuntime, logicalSkillPoints, COMPANION_MAX_SKILL_SLOTS);',
    'ApplyCompanionPassiveSkills(entity, slot.skillRuntime);',
]:
    if needle not in system:
        err.append('canonical persisted skill-point/passive load missing: '+needle)
for needle in [
    'ZeroMemory(runtime.skillPoints, sizeof(runtime.skillPoints));',
    'runtime.skillPoints[useSlot - 1]',
]:
    if needle not in exe:
        err.append('canonical skill point normalization missing: '+needle)
if '112 READY + 16 audited passives = 128/128 classified' not in doc:
    err.append('audit document closure marker missing')

if err:
    print('Companion Passive Audit V29: FAIL')
    for e in err: print(' -',e)
    sys.exit(1)
print('Companion Passive Audit V29: PASS (112 READY active/special + 16 materialized passives; 128/128 classified)')
