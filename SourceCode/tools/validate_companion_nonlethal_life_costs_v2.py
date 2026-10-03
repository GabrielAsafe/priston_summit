#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
path=ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp'
text=path.read_text('utf-8',errors='ignore')

def section(start,end):
    try: return text.split(start,1)[1].split(end,1)[0]
    except IndexError: return ''

ruthless=section('static void PrepareAttackTriggeredBuffs(smCHAR *caster, CompanionSkillRuntime &runtime)\n{','static void FinishAttackTriggeredBuffs(smCHAR *caster, CompanionSkillRuntime &runtime)\n{')
raving=section('static void ApplyRavingLifeCost(smCHAR *caster, int point)\n{','static void StartMultiHitSchedule(smCHAR *caster, CompanionSkillRuntime &runtime,')
checks=[]
def ck(ok,label): checks.append((ok,label)); print(('PASS' if ok else 'FAIL')+': '+label)
ck('ServerLegacyLifeAdapter().ConsumeNonLethal' in ruthless,'Ruthless HP cost consumes through V2 life boundary')
ck('caster->smCharInfo.Life[0] =' not in ruthless,'Ruthless has no direct life write')
ck('ServerLegacyLifeAdapter().ConsumeNonLethal' in raving,'Raving HP cost consumes through V2 life boundary')
ck('nonLethalCost' in raving and 'caster->smCharInfo.Life[0] <= 1' in raving,'Raving preserves minimum-one-HP rule')
ck('caster->smCharInfo.Life[0] =' not in raving,'Raving has no direct life write')
if not all(x for x,_ in checks):
    print('companion nonlethal life costs v2: FAIL',file=sys.stderr); sys.exit(1)
print(f'companion nonlethal life costs v2: PASS ({len(checks)}/{len(checks)})')
