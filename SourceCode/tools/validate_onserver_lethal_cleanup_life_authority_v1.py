#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
text = (ROOT / 'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(encoding='utf-8')


def section(start, end):
    a = text.index(start)
    b = text.index(end, a)
    return text[a:b]

skill_pet = section('smCHAR *OpenMonsterFromSkill(', 'int rsHardCoreEvent_OpenMonster(')
hardcore = section('int rsHardCoreEvent_OpenMonster(', '\n\n#else\n')
stage_main = section('int STG_AREA::Main()', 'int STG_AREA::LoadStage(')
# Ignore the retained historical block comment after the active nightmare path.
if '/*if(FieldNightDay != dwServ_NightDay)' in stage_main:
    stage_main = stage_main.split('/*if(FieldNightDay != dwServ_NightDay)', 1)[0]
autochar = section('int srAutoCharMain(smCHAR *lpChar)', 'int srAutoPlayMain()')

checks = [
    ('skill pet replacement uses life authority', 'ServerLegacyLifeAdapter().ConsumeLethal' in skill_pet),
    ('skill pet replacement has no direct lethal write', 'chrAutoPlayer[cnt].smCharInfo.Life[0] = 0;' not in skill_pet),
    ('hardcore cleanup uses life authority', 'ServerLegacyLifeAdapter().ConsumeLethal' in hardcore),
    ('hardcore cleanup has no direct lethal write', 'lpStgArea->lpCharMonster[cnt]->smCharInfo.Life[0] = 0;' not in hardcore),
    ('nightmare cleanup uses life authority', 'ServerLegacyLifeAdapter().ConsumeLethal' in stage_main),
    ('nightmare active path has no direct lethal write', 'chrAutoPlayer[cnt].smCharInfo.Life[0] = 0;' not in stage_main),
    ('autochar expiry uses life authority twice', autochar.count('ServerLegacyLifeAdapter().ConsumeLethal') >= 2),
    ('autochar expiry has no direct lethal write', 'lpChar->smCharInfo.Life[0] = 0;\n\t\t\t\tlpChar->SetMotionFromCode(CHRMOTION_STATE_DEAD);' not in autochar),
]
failed = []
for name, ok in checks:
    print(('PASS' if ok else 'FAIL') + ': ' + name)
    if not ok:
        failed.append(name)
if failed:
    sys.exit(1)
print('onserver lethal cleanup life authority v1: PASS')
