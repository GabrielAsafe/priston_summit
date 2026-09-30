#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT=Path(__file__).resolve().parents[1]
CPP=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
CAT=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
LEG=(ROOT/'Shared/gameplay/combat/LegacyDamagePolicy.inl').read_text(errors='ignore')
CHAR=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='ignore')
STATUS=(ROOT/'SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyStatusEffectAdapter.cpp').read_text(errors='ignore')
err=[]
checks={
'SKILL_GROUND_PIKE':['G_Pike_DamagePercent[p]','G_Pike_Time[p] * 17','combat::SlowEffect(slowTime'],
'SKILL_ROAR':['Roar_Time[p] << 4','Resistance[sITEMINFO_BIONIC]','ServerLegacyStatusEffectAdapter().ApplyStun'],
'SKILL_GLACIAL_SPIKE':['Glacial_Spike_Damage_Percent[p]','8 * 17','case SKILL_GLACIAL_SPIKE: return 50'],
'SKILL_DISTORTION':['Distortion_Time[p] * 17','Distortion_SpeedSubPercent[p]','combat::SlowEffect(time'],
'SKILL_EXTINCTION':['Extinction_Percent[p]','Extinction_Amount[p]','smCHAR_MONSTER_UNDEAD','ApplyDirectLifeLoss']
}
for code,toks in checks.items():
    line=next((ln for ln in CAT.splitlines() if '{ '+code+',' in ln),'')
    if ', true,' not in line: err.append('catalog not READY '+code)
    for tok in toks:
        if tok not in CPP: err.append(code+' adapter missing '+tok)
for tok in ('resolved->PlaySlowCount = effect.durationTicks','resolved->PlaySlowSpeed = effect.moveSpeed',
            'resolved->PlayStunCount = durationTicks','resolved->PlayDistortion = effect.distortionTicks'):
    if tok not in STATUS: err.append('status boundary missing legacy mutation '+tok)
for tok in ('case SKILL_PLAY_GROUND_PIKE:','G_Pike_Time[cnt] * 17','case SKILL_PLAY_ROAR:','Roar_Time[Point]','case SKILL_PLAY_GLACIAL_SPIKE:','time = 8 * 17','case SKILL_PLAY_DISTORTION:','Distortion_Time[SkillPoint] * 17','case SKILL_PLAY_EXTINCTION:','Extinction_Amount[lpTransSkilAttackData->Power - 1]'):
    if tok not in LEG: err.append('legacy status oracle missing '+tok)
for tok in ('if(PlaySlowSpeed)','moveStep = SERVER_CHAR_SKIP >> 1','if(PlayStunCount > 0)','if(PlayDistortion > 0)'):
    if tok not in CHAR: err.append('native status lifecycle missing '+tok)
ready=sum(', true,' in ln for ln in CAT.splitlines() if ln.strip().startswith('{ SKILL_'))
if ready < 74: err.append(f'expected at least 74 READY after V10, got {ready}')
if err:
    print('Companion Status Engine V10: FAIL')
    for e in err: print(' -',e)
    sys.exit(1)
print(f'Companion Status Engine V10: PASS ({ready} READY; status mutations routed through v2 boundary)')
