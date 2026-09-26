#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
CPP=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
CAT=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
CHAR=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='ignore')
POL=(ROOT/'Shared/gameplay/combat/LegacyDamagePolicy.inl').read_text(errors='ignore')
SUB=(ROOT/'Shared/gameplay/skills/SkillSub_legacy_server.inl').read_text(errors='ignore')
ONS=(ROOT/'Shared/legacy_oracle/server/core/server/legacy_runtime/OnSever.cpp.inl').read_text(errors='ignore')
SIN=(ROOT/'SrcServer/src/core/server/legacy_sin_runtime/sinSubMain.cpp').read_text(errors='ignore')
err=[]
for code in ('SKILL_HOLY_MIND','SKILL_ASSASSIN_EYE','SKILL_HOLY_INCANTATION','SKILL_SPLIT_JAVELIN'):
    line=next((ln for ln in CAT.splitlines() if '{ '+code+',' in ln),'')
    if ', true,' not in line: err.append(code+' not READY')
    if ('case '+code+':') not in CPP and ('skillCode == '+code) not in CPP: err.append(code+' missing executor dispatch')
# Holy Mind exact 15-second command -> <<4 native status, Bionic resistance.
for tok in ('HolyMind_DecDamage[p]','(15 << 4) * (100 - resistance) / 100','PlayHolyMind[0]','PlayHolyMind[1]'):
    if tok not in CPP: err.append('Holy Mind runtime missing '+tok)
for tok in ('smTransCommand.WParam = HolyMind_DecDamage[lpSkill->Point - 1]','smTransCommand.LParam = 15'):
    if tok not in SUB: err.append('Holy Mind cast oracle missing '+tok)
for tok in ('time = lpTransCommand->LParam << 4','PlayHolyMind[0] = lpTransCommand->WParam','PlayHolyMind[1] = time'):
    if tok not in ONS: err.append('Holy Mind server oracle missing '+tok)
if 'TransAttackData.Power -= ((TransAttackData.Power*PlayHolyMind[0]) / 100);' not in CHAR:
    err.append('native Holy Mind damage-reduction hook missing')
# Assassin Eye uses the native target fields already read by legacy critical calculation.
for tok in ('dwAssassinEyeTime = dwPlayServTime + (DWORD)Assassin_Eye_Time[p] * 1000U','AssassinEyeParam = Assassin_Eye_M_SubCritical[p]'):
    if tok not in CPP: err.append('Assassin Eye runtime missing '+tok)
for tok in ('dwAssassinEyeTime = lpPlayInfo->dwSkill_AssassinEye_Time','AssassinEyeParam = Assassin_Eye_M_SubCritical[Point]'):
    if tok not in POL: err.append('Assassin Eye oracle missing '+tok)
# Holy Incantation must rely on the native conversion lifetime and restore path.
for tok in ('H_Incantation_Success[p]','H_Incantation_Time[p] * 17','H_Incantation_AddLife[p]','smCHAR_MONSTER_USER','lpMasterPlayInfo = owner'):
    if tok not in CPP: err.append('Holy Incantation runtime missing '+tok)
for tok in ('PlayHolyIncantation[1]--','smCharInfo.Brood = PlayHolyIncantation[0]','lpMasterPlayInfo = 0'):
    if tok not in CHAR: err.append('native Holy Incantation cleanup missing '+tok)
# Split Javelin: current legacy accuracy helper hard-returns 100, then server sums independent rolls.
for tok in ('Split_Javelin_AttackNum[p]','Split_Javelin_Damage[p]','for(int hit = 0; hit < hits; ++hit)'):
    if tok not in CPP: err.append('Split Javelin runtime missing '+tok)
if 'return 100;' not in SIN: err.append('legacy sinGetAccuracy hard-return oracle changed')
for tok in ('z = Split_Javelin_AttackNum[lpSkill->Point - 1]','y = sinGetAccuracy(lpChar->smCharInfo.Level, lpChar->smCharInfo.Defence)','dm_SendTransDamage(lpChar, x'):
    if tok not in SUB: err.append('Split Javelin cast oracle missing '+tok)
for tok in ('Power2[0] > Split_Javelin_AttackNum[Point]','pow += (pow*Split_Javelin_Damage[Point]) / 100'):
    if tok not in POL: err.append('Split Javelin damage oracle missing '+tok)
# New single target effects must not enter radial iteration.
for tok in ('skillCode == SKILL_HOLY_MIND || skillCode == SKILL_ASSASSIN_EYE','skillCode == SKILL_HOLY_INCANTATION','return ApplyStatusEffectToTarget(caster, owner, primary'):
    if tok not in CPP: err.append('single-target isolation missing '+tok)
ready=sum(', true,' in ln for ln in CAT.splitlines() if ln.strip().startswith('{ SKILL_'))
if ready < 79: err.append(f'expected at least 79 READY after V12, got {ready}')
if err:
    print('Companion targeted debuffs/control V12: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
print(f'Companion targeted debuffs/control V12: PASS ({ready} READY; Holy Mind/Assassin Eye/Holy Incantation/Split Javelin)')
