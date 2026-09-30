#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
CPP=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
CAT=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
LEG=(ROOT/'Shared/legacy_oracle/server/systems/combat/server_legacy/Svr_Damge.cpp.inl').read_text(errors='ignore')
CHAR=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='ignore')
STATUS=(ROOT/'SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyStatusEffectAdapter.cpp').read_text(errors='ignore')
err=[]
line=next((ln for ln in CAT.splitlines() if '{ SKILL_VENOM_SPEAR,' in ln),'')
if ', true,' not in line: err.append('Venom Spear not READY')
for tok in ('case SKILL_VENOM_SPEAR:','VenomSpear_Range[p]','VenomSpear_Num[point - 1]','damage = std::max(1, damage >> 1)','sITEMINFO_POISON','ServerLegacyStatusEffectAdapter().ApplyPoison(','VenomSpear_Time[p] * 17','poisonBase * VenomSpear_DamagePercent[p]'):
    if tok not in CPP: err.append('V11 missing '+tok)
for tok in ('resolved->PlayPoison[1] = static_cast<short>(durationTicks)','resolved->PlayPoison[0] = static_cast<short>(damage)'):
    if tok not in STATUS: err.append('status adapter poison parity missing '+tok)
for tok in ('PlayPoison[1] = VenomSpear_Time[SkillPoint] * 17','PlayPoison[0] = (lpTransSkilAttackData->Power*VenomSpear_DamagePercent[SkillPoint])/100'):
    if tok not in LEG: err.append('legacy poison oracle missing '+tok)
for tok in ('if(PlayPoison[1] > 0)','PlayPoison[1]--','smCharInfo.Life[0] -= PlayPoison[0]'):
    if tok not in CHAR: err.append('native poison lifecycle missing '+tok)
ready=sum(', true,' in ln for ln in CAT.splitlines() if ln.strip().startswith('{ SKILL_'))
if ready < 75: err.append(f'expected at least 75 READY after V11, got {ready}')
if err:
    print('Companion Venom Spear V11: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
print(f'Companion Venom Spear V11: PASS ({ready} READY; native poison lifecycle)')
