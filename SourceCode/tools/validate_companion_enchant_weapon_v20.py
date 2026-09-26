#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
CPP=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
H=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h').read_text(errors='ignore')
CAT=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillCatalog.inl').read_text(errors='ignore')
DMG=(ROOT/'Shared/legacy_oracle/client/compat/client_server/server_legacy/Svr_Damge.cpp.inl').read_text(errors='ignore')
SUB=(ROOT/'Shared/legacy_oracle/server/systems/combat/skills/server_support/SkillSub.cpp.inl').read_text(errors='ignore')
ON=(ROOT/'Shared/legacy_oracle/server/core/server/legacy_runtime/OnSever.cpp.inl').read_text(errors='ignore')
err=[]
line=next((ln for ln in CAT.splitlines() if '{ SKILL_ENCHANT_WEAPON,' in ln),'')
if ', true,' not in line: err.append('Enchant Weapon not READY')
for tok in ('enchantWeaponExpiresAt','enchantWeaponPoint','enchantWeaponElement'):
    if tok not in H or tok not in CPP: err.append('Enchant runtime missing '+tok)
for tok in ('ApplyEnchantWeaponBonus','Enchant_Weapon_Damage_Ice[p][0]','Enchant_Weapon_Damage_Lightning[p][0]','Enchant_Weapon_Damage_Fire[p][0]',
            'damage += RandomBetween(low, high)','damage -= (high * resistance) / 100'):
    if tok not in CPP: err.append('Enchant damage semantics missing '+tok)
for tok in ('runtime.enchantWeaponElement == 0','int slowCount = 16 * 3','target->PlaySlowSpeed = 230 - 10 * runtime.enchantWeaponPoint','target->PlayDistortion = 0'):
    if tok not in CPP: err.append('Enchant ice semantics missing '+tok)
for tok in ('runtime.enchantWeaponElement = rand() % 3','visualParam = runtime.enchantWeaponElement','((DWORD)(visualParam & 0xF) << 12)'):
    if tok not in CPP: err.append('Enchant element/visual parameter missing '+tok)
# The enchant augments non-elemental/basic physical damage. It must not stack onto native magic-element skills.
if 'if(!IsMagicDamageSkillCode(skillCode))' not in CPP or 'damage = ApplyEnchantWeaponBonus(*runtime, target, damage, dwPlayServTime);' not in CPP:
    err.append('Enchant non-magic skill hook missing')
if 'value = ApplyEnchantWeaponBonus(runtime, target, value, now);' not in CPP:
    err.append('Enchant basic attack hook missing')
for tok in ('Enchant_Weapon_Time[p]','descriptor.skillCode == SKILL_ENCHANT_WEAPON'):
    if tok not in CPP: err.append('Enchant support cast missing '+tok)
# Oracle: random elemental payload, high bound encoded for resistance, and ice slow.
for tok in ('Enchant_Weapon_Damage_Ice[len - 1][0], Enchant_Weapon_Damage_Ice[len - 1][1]',
            'Enchant_Weapon_Damage_Lightning[len - 1][0], Enchant_Weapon_Damage_Lightning[len - 1][1]',
            'Enchant_Weapon_Damage_Fire[len - 1][0], Enchant_Weapon_Damage_Fire[len - 1][1]',
            'lpTransAttackData->Power -= ((cnt*len) / 100)',
            'lpChar->PlaySlowCount = 16 * 3',
            'lpChar->PlaySlowSpeed = 230 - 10 * (lpPlayInfo->dwSkill_EnchantWeapon_Param & 0xFF)'):
    if tok not in DMG: err.append('Enchant damage oracle missing '+tok)
for tok in ('cnt = rand() % 3','lpCurPlayer->AttackSkil |= (cnt << 12)'):
    if tok not in SUB: err.append('Enchant cast oracle missing '+tok)
for tok in ('dwSkill_EnchantWeapon_Time = dwPlayServTime + Enchant_Weapon_Time[point] * 1000','dwSkill_EnchantWeapon_Param = (point + 1) | (lpTransCommand->EParam << 8)'):
    if tok not in ON: err.append('Enchant server state oracle missing '+tok)
ready=sum(', true,' in ln for ln in CAT.splitlines() if ln.strip().startswith('{ SKILL_'))
if ready < 99: err.append(f'expected at least 99 READY after V20, got {ready}')
if err:
    print('Companion Enchant Weapon V20: FAIL'); [print(' -',e) for e in err]; sys.exit(1)
print(f'Companion Enchant Weapon V20: PASS ({ready} READY; random element + damage/resistance/ice slow + visual param)')
