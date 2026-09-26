#!/usr/bin/env python3
from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parents[1]
cpp=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
h=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h').read_text(errors='ignore')
cat=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
legacy=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='ignore')
dmg=(ROOT/'Shared/gameplay/combat/LegacyDamagePolicy.inl').read_text(errors='ignore')
inv=(ROOT/'SrcServer/src/systems/inventory/legacy/sinInvenTory.cpp').read_text(errors='ignore')
err=[]
for code in ['SKILL_FALCON','SKILL_GOLDEN_FALCON']:
 line=next((x for x in cat.splitlines() if code in x),'')
 if ', true,' not in line: err.append(code+' not READY')
for tok in ['falconExpiresAt','falconPoint','goldenFalconPoint','falconDamageMin','falconDamageMax']:
 if tok not in h: err.append('runtime missing '+tok)
for tok in ['Falcon_Damage[falconPoint - 1][0]','Golden_Falcon_Damage[p]','Golden_Falcon_Time[p]',
            'Golden_Falcon_LifeRegen[runtime.goldenFalconPoint - 1]', 'forceNatureFalconDamage',
            'CompanionFalconDamage(runtime, now)', 'ApplyCompanionFalconStrike(caster, owner, target, runtime, now)']:
 if tok not in cpp: err.append('executor missing '+tok)
for tok in ['dwSKill_Falcon_Damage[0] = Falcon_Damage[point][0]', 'Golden_Falcon_Damage[point]']:
 if tok not in legacy: err.append('legacy runtime oracle missing '+tok)
for tok in ['case SKILL_PLAY_FALCON:', 'wSkill_ForceOfNature_Param[1]']:
 if tok not in dmg: err.append('damage oracle missing '+tok)
if 'sinfRegenLife += Golden_Falcon_LifeRegen[ContinueSkill[i].Point - 1]' not in inv:
 err.append('golden regen oracle missing')
ready=len(re.findall(r'\{\s*SKILL_[A-Z0-9_]+,\s*COMPANION_SKILL_ROLE_[A-Z_]+,\s*true,',cat))
if ready < 109: err.append(f'expected at least 109 READY after V25, got {ready}')
if err:
 print('Companion Falcon V25: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
print(f'Companion Falcon V25: PASS ({ready} READY; Falcon/Golden persistent damage + Golden regen)')
