#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
CPP=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
CAT=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
POL=(ROOT/'Shared/gameplay/combat/LegacyDamagePolicy.inl').read_text(errors='ignore')
SUB=(ROOT/'Shared/gameplay/skills/SkillSub_legacy_server.inl').read_text(errors='ignore')
COMMON=(ROOT/'Shared/gameplay/combat/LegacyDamageCommon.inl').read_text(errors='ignore')
err=[]
for code in ('SKILL_PIKE_WIND','SKILL_CRITICAL_HIT','SKILL_SHIELD_STRIKE','SKILL_SOUL_SUCKER'):
    line=next((ln for ln in CAT.splitlines() if '{ '+code+',' in ln),'')
    if ', true,' not in line: err.append(code+' not READY')
    if 'case '+code+':' not in CPP: err.append(code+' missing executor dispatch')
# Critical Hit: same skill-added crit and GetCritical semantics, including Assassin Eye and 170% crit.
for tok in ('Critical_Hit_Critical[p]','target->dwAssassinEyeTime > dwPlayServTime','critical -= target->AssassinEyeParam','if(critical > 70) critical = 70','damage = (damage * 170) / 100'):
    if tok not in CPP: err.append('Critical Hit missing '+tok)
for tok in ('lpTransAttackData2->Critical[0] += Critical_Hit_Critical[Point]','Result -= lpChar->AssassinEyeParam','if(Result > 70)','Result = Critical_Hit +'):
    src=POL if 'Critical_Hit_Critical' in tok else COMMON
    if tok not in src: err.append('critical oracle missing '+tok)
# Pike Wind: radius/damage and the legacy push distance from remaining radius, with boss exclusions.
for tok in ('Pike_Wind_Push_Lenght[point - 1]','Pike_Wind_DamagePercent[p]','radius - dist','PushTargetAwayFromCaster','snCHAR_SOUND_DEATHKNIGHT','snCHAR_SOUND_MOKOVA'):
    if tok not in CPP: err.append('Pike Wind missing '+tok)
for tok in ('lpTransSkillAttackData->AttackSize = Pike_Wind_Push_Lenght[Point]','pow += (pow*Pike_Wind_DamagePercent[Point]) / 100','dist = lpTransSkilAttackData->AttackSize - dist'):
    if tok not in POL: err.append('Pike Wind oracle missing '+tok)
# Shield Strike: 100-wide x 160 box, damage percent, 10s-equivalent stun ticks scaled by Bionic and push 10.
for tok in ('IsInsideCompanionForwardBox(caster, primary, primary, 50, 160)','S_Strike_DamagePercent[p]','16 * 10 * (100 - resistance)','PushTargetAwayFromCaster(caster, target, 10)'):
    if tok not in CPP: err.append('Shield Strike missing '+tok)
for tok in ('rect.left = -50','rect.right = 50','rect.bottom = 160'):
    if tok not in SUB: err.append('Shield Strike geometry oracle missing '+tok)
for tok in ('pow += (pow*S_Strike_DamagePercent[Point]) / 100','lpTransSkillAttackData->AttackState = 5','lpChar->PlayStunCount = 16 * 10'):
    if tok not in POL: err.append('Shield Strike policy oracle missing '+tok)
# Soul Sucker: forward box, targets above 50%, percent of max life, Bionic=100 immunity, caster heal.
for tok in ('IsInsideCompanionForwardBox(caster, primary, candidate, 30, 200)','Soul_Sucker_Absorb[point - 1]','* 50) / 100 >= candidate->smCharInfo.Life[0]','if(bionic == 100) absorb = 0','caster->smCharInfo.Life[0] = (short)std::min'):
    if tok not in CPP: err.append('Soul Sucker missing '+tok)
for tok in ('rect.left = -30','rect.right = 30','rect.bottom = 200'):
    if tok not in SUB: err.append('Soul Sucker geometry oracle missing '+tok)
for tok in ('Soul_Sucker_Absorb[lpTransSkilAttackData->Power - 1]','((lpChar->smCharInfo.Life[1] * 50) / 100) < lpChar->smCharInfo.Life[0]','TotalLife += life'):
    if tok not in POL: err.append('Soul Sucker policy oracle missing '+tok)
ready=sum(', true,' in ln for ln in CAT.splitlines() if ln.strip().startswith('{ SKILL_'))
if ready < 83: err.append(f'expected at least 83 READY after V13, got {ready}')
if err:
    print('Companion combat specials V13: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
print(f'Companion combat specials V13: PASS ({ready} READY; Critical/PikeWind/ShieldStrike/SoulSucker)')
