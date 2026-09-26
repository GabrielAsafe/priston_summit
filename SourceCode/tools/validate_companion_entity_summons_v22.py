#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
CPP=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
CAT=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
CHAR=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='ignore')
ON=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='ignore')
ORACLE=(ROOT/'Shared/legacy_oracle/server/core/server/legacy_runtime/OnSever.cpp.inl').read_text(errors='ignore')
err=[]
for code in ('SKILL_METAL_GOLEM','SKILL_RECALL_WOLVERIN','SKILL_FIRE_ELEMENTAL'):
    line=next((ln for ln in CAT.splitlines() if '{ '+code+',' in ln),'')
    if ', true,' not in line: err.append(code+' not READY')
for tok in ('OpenMonsterFromSkill(SkillPlayCode(descriptor.skillCode)','owner, caster','spawnedSkillPet->dwUpdateCharInfoTime = now + 355U * 1000U'):
    if tok not in CPP: err.append('companion summon execution missing '+tok)
# Exact legacy stat shaping for each entity summon.
for tok in ('Metal_Golem_Damage[p][0]','Metal_Golem_Hit[p]','Metal_Golem_Defense[p]','Metal_Golem_Life[p] + caster->smCharInfo.Strength * 2','fONE + p * 15',
            'Recall_Wolverin_Damage[p][0]','Recall_Wolverin_Hit[p]','Recall_Wolverin_Defense[p] + caster->smCharInfo.Dexterity * 2','Recall_Wolverin_Life[p]','(fONE / 2) + p * 12',
            'Fire_Elemental_Damage[p][0]','Fire_Elemental_Hit[p]','Fire_Elemental_Life[p]'):
    if tok not in CPP: err.append('summon stat semantics missing '+tok)
# Companion-owned summons must not use/overwrite the player real summon slot.
for tok in ('const int CompanionSkillOwner','lpEnemyChar->lpMasterPlayInfo == lpPlayInfo','replaceCompanionPet = CompanionSkillOwner && chrAutoPlayer[cnt].lpLinkChar == lpEnemyChar',
            'lpChar->lpLinkChar = lpEnemyChar','lpChar->dwLinkObjectCode = lpEnemyChar->dwObjectSerial'):
    if tok not in ON: err.append('companion summon ownership missing '+tok)
# Existing player-owned path remains intact.
for tok in ('if(lpPlayInfo && !lpEnemyChar)','lpPlayInfo->lpLinkChar = lpChar','lpPlayInfo->dwLinkCharCode = lpChar->dwObjectSerial'):
    if tok not in ON: err.append('legacy player summon ownership regressed '+tok)
# Pet lifecycle/follow follows companion-caster and dies when that link is invalid.
for tok in ('const int companionOwnedSkillPet','lpLinkChar->lpMasterPlayInfo == lpMasterPlayInfo','!companionOwnedSkillPet && lpMasterPlayInfo->lpLinkChar != this',
            'companionOwnedSkillPet ? lpLinkChar->pX : lpMasterPlayInfo->Position.x','companionOwnedSkillPet ? lpLinkChar->pY : lpMasterPlayInfo->Position.y'):
    if tok not in CHAR: err.append('companion summon lifecycle missing '+tok)
# Oracle stat rules.
for tok in ('Metal_Golem_Damage[point][0]','Metal_Golem_Life[point] + lpPlayInfo->smCharInfo.Strength * 2',
            'Recall_Wolverin_Damage[point][0]','Recall_Wolverin_Defense[point] + lpPlayInfo->smCharInfo.Dexterity * 2',
            'Fire_Elemental_Damage[point][0]','Fire_Elemental_Life[point]'):
    if tok not in ORACLE: err.append('summon oracle missing '+tok)
ready=sum(', true,' in ln for ln in CAT.splitlines() if ln.strip().startswith('{ SKILL_'))
if ready < 105: err.append(f'expected at least 105 READY after V22, got {ready}')
if err:
    print('Companion entity summons V22: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
print(f'Companion entity summons V22: PASS ({ready} READY; Golem/Wolverine/Fire Elemental companion ownership)')
