#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
paths = {
    'protocol': ROOT / 'Shared/gameplay/companions/CompanionProtocol.h',
    'visual': ROOT / 'Shared/gameplay/character/character_legacy_client.inl',
    'profile_h': ROOT / 'SrcServer/src/systems/gameplay/companions/CompanionProfileStore.h',
    'profile_cpp': ROOT / 'SrcServer/src/systems/gameplay/companions/CompanionProfileStore.cpp',
    'executor_h': ROOT / 'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h',
    'executor_cpp': ROOT / 'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp',
    'system': ROOT / 'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp',
    'character_adapter': ROOT / 'SrcServer/src/systems/gameplay/player/v2/legacy/LegacyCharacterRecordAdapter.cpp',
    'models': ROOT / 'SrcGame/src/systems/ui/v2/domain/CompanionUIModels.h',
    'gateway': ROOT / 'SrcGame/src/systems/ui/v2/legacy/LegacyCompanionGateway.cpp',
    'page_h': ROOT / 'SrcGame/src/systems/ui/v2/pages/CompanionsPage.h',
    'page_cpp': ROOT / 'SrcGame/src/systems/ui/v2/pages/CompanionsPage.cpp',
}
text = {name: path.read_text(errors='ignore') for name, path in paths.items()}

checks = [
    ('UI protocol v5++', 'protocol', 'COMPANION_UI_PROTOCOL_VERSION = 6'),
    ('active skill wire action', 'protocol', 'COMPANION_UI_CYCLE_ACTIVE_SKILL'),
    ('buff skill wire action', 'protocol', 'COMPANION_UI_CYCLE_BUFF_SKILL'),
    ('buff interval wire action', 'protocol', 'COMPANION_UI_ADJUST_BUFF_INTERVAL'),
    ('wire active slots', 'protocol', 'CompanionUISkillSlot activeSkills[4]'),
    ('wire buff slots', 'protocol', 'CompanionUISkillSlot buffSkills[4]'),

    ('profile active slots', 'profile_h', 'BYTE activeSkillSlots[4]'),
    ('profile buff slots', 'profile_h', 'BYTE buffSkillSlots[4]'),
    ('profile buff intervals', 'profile_h', 'WORD buffIntervalSeconds[4]'),
    ('profile v4', 'profile_cpp', 'COMPANION_PROFILE_VERSION = 4'),
    ('profile v3 migration', 'profile_cpp', 'COMPANION_PROFILE_SKILL_LOADOUT_VERSION = 3'),
    ('profile v2 migration', 'profile_cpp', 'COMPANION_PROFILE_V2_VERSION = 2'),
    ('profile ABI retained', 'profile_cpp', 'sizeof(CompanionProfileDiskV1) == 84'),
    ('active profile flag', 'profile_cpp', 'COMPANION_PROFILE_SKILL_LOADOUT_ACTIVE'),
    ('buff profile flag', 'profile_cpp', 'COMPANION_PROFILE_BUFF_LOADOUT_ACTIVE'),

    ('learned descriptor API', 'executor_h', 'GetCompanionSkillDescriptor'),
    ('canonical skill names independent of FILE bytes', 'executor_cpp', 'CanonicalCompanionSkillName'),
    ('fighter expanded catalog', 'executor_cpp', 'SKILL_TRIPLE_IMPACT'),
    ('mechanician expanded catalog', 'executor_cpp', 'SKILL_MECHANIC_BOMB'),
    ('archer expanded catalog', 'executor_cpp', 'SKILL_ARROW_OF_RAGE'),
    ('magician expanded catalog', 'executor_cpp', 'SKILL_FIRE_BALL'),
    ('skill cycle API', 'executor_h', 'CycleCompanionSkillUseSlot'),
    ('active loadout executor API', 'executor_h', 'TryExecuteCompanionDamageLoadout'),
    ('buff loadout executor API', 'executor_h', 'TryExecuteCompanionBuffLoadout'),
    ('buff modifier expiry API', 'executor_h', 'UpdateCompanionBuffModifiers'),
    ('buff duration API', 'executor_h', 'GetCompanionBuffDurationSeconds'),
    ('active priority compatibility cursor', 'executor_cpp', 'activeSkillCursor'),
    ('active priority order V30', 'executor_cpp', 'slot order is the explicit priority contract'),
    ('active fallback V1', 'executor_cpp', 'return TryExecuteCompanionDamageSkill'),
    ('buff interval schedule', 'executor_cpp', 'nextBuffSkillTime[slot] = now + (DWORD)interval * 1000U'),
    ('buff stat authority', 'executor_cpp', 'ApplyBuffModifier'),
    ('buff max duration guard', 'executor_cpp', 'BuffDurationSeconds'),
    ('mechanician physical absorption buff', 'executor_cpp', 'SKILL_PHYSICAL_ABSORB'),
    ('priestess virtual life buff', 'executor_cpp', 'SKILL_VIRTUAL_LIFE'),
    ('virtual life max HP authority', 'executor_cpp', 'maxLifeDelta'),

    ('character snapshot repository', 'system', 'ServerLegacyCharacterRecordAdapter().TryLoadSnapshot'),
    ('skill points legacy normalization', 'character_adapter', 'record.GameSaveInfo.SkillInfo.bSkillPoint[useSlot & 15]'),
    ('skill points rank5 normalization', 'character_adapter', 'GetTier5RecordSkillPoint(&record.GameSaveInfo, tier5Index)'),
    ('snapshot consumes repository skill DTO', 'system', 'recordData.logicalSkillPoints'),
    ('active loadout action handler', 'system', 'ChangeSkillLoadout(owner, name, false'),
    ('buff loadout action handler', 'system', 'ChangeSkillLoadout(owner, name, true'),
    ('buff interval action handler', 'system', 'AdjustBuffInterval'),
    ('support interval defaults are explicit', 'system', 'GetCompanionSupportDefaultIntervalSeconds'),
    ('live interval edit reschedules cast', 'system', 'dwPlayServTime + (DWORD)interval * 1000U'),
    ('active loadout execution', 'system', 'TryExecuteCompanionDamageLoadout'),
    ('support loadout execution', 'system', 'TryExecuteCompanionSupportLoadout'),
    ('support cast telemetry', 'system', 'kind=support'),
    ('duplicate loadout prevention without side effects', 'system', 'skip a skill already assigned to'),
    ('per-slot buff reset', 'system', 'ResetCompanionBuffModifier(activeSlot->entity, activeSlot->skillRuntime, slot)'),
    ('loadout edit telemetry', 'system', 'LOADOUT_EDIT'),

    ('client active skill model', 'models', 'activeSkills{}'),
    ('client buff skill model', 'models', 'buffSkills{}'),
    ('gateway active mapping', 'gateway', 'CompanionUiAction::CycleActiveSkill'),
    ('gateway buff mapping', 'gateway', 'CompanionUiAction::CycleBuffSkill'),
    ('skills/equipment explicit view', 'page_cpp', 'skillsView_ = true'),
    ('active/buff explicit subview', 'page_cpp', 'buffSkillsView_ = true'),
    ('active name row retained', 'page_cpp', '{root.x + 330, y, 218, 22}'),
    ('wide buff name row', 'page_cpp', '{root.x + 330, y, 226, 22}'),
    ('four active buttons', 'page_h', 'activeSkillPrevButtons_'),
    ('four buff buttons', 'page_h', 'buffSkillPrevButtons_'),
    ('buff interval buttons', 'page_h', 'buffIntervalPrevButtons_'),
    ('active reset UI', 'page_cpp', 'Ativas: auto V1'),
    ('support reset UI', 'page_cpp', 'Desligar suporte'),

    ('visual detail telemetry', 'visual', 'VISUAL_SKILL_DETAIL'),
    ('visual target resolution telemetry', 'visual', 'targetFound=%d'),
    ('visual event bridge telemetry', 'visual', 'VISUAL_SKILL_EVENT'),
    ('visual EventAttack bridge', 'visual', 'character->EventAttack(FALSE)'),
    ('visual direct EventSkill fallback', 'visual', 'character->EventSkill()'),
    ('visual dispatch guard', 'visual', 'skillEffectDispatched'),
    ('visual pending clip guard', 'visual', 'runtime.skillVisualPending && desired != CHRMOTION_STATE_DEAD'),
    ('motion result no longer authoritative', 'visual', 'const bool desiredSelected = character->MotionInfo'),
    ('motion fallback only after state mismatch', 'visual', 'if(!desiredSelected && desired == CHRMOTION_STATE_SKILL)'),
]

errors = []
for label, name, needle in checks:
    if needle not in text[name]:
        errors.append(f"{label}: missing {needle!r} in {paths[name].relative_to(ROOT)}")

# The exact regression that hid skill clips: never restore the old boolean-return
# fallback, because SetMotionFromCode may select FoundNum[0] and return NULL.
if re.search(r'if\s*\(\s*!character->SetMotionFromCode\(desired\)', text['visual']):
    errors.append('visual regression: SetMotionFromCode(desired) return value is still used as fallback authority')


# Every advertised damage case must share an explicit true branch. The switch
# intentionally groups many case labels before one return true.
damage_switch = re.search(r'static bool IsDamageSkillCode\(DWORD skillCode\)\s*\{(.*?)\n\}', text['executor_cpp'], re.S)
if not damage_switch:
    errors.append('executor: missing IsDamageSkillCode switch')
else:
    body = damage_switch.group(1)
    if 'DefaultLegacySkillAdapter().IsDamage' in body:
        if '"Holy Bolt"' not in text['executor_cpp'] and 'CanonicalCompanionSkillName' not in text['executor_cpp']:
            errors.append('executor: canonical damage adapter is not wired to catalog naming')
    elif 'case SKILL_HOLY_BOLT:' not in body or 'return true;' not in body:
        errors.append('executor: supported damage cases do not terminate in return true')

# Spirit Elemental may only be selectable when its dedicated magic-only damage
# modifier and mana-regen semantics are present.
buff_switch = re.search(r'static bool IsBuffSkillCode\(DWORD skillCode\).*?\n\}', text['executor_cpp'], re.S)
if buff_switch and 'SKILL_SPIRIT_ELEMENTAL' in buff_switch.group(0):
    for marker in ('magicSkillDamagePercent', 'IsMagicDamageSkillCode', 'Spirit_Elemental_Damage', 'Spirit_Elemental_RegenMana'):
        if marker not in text['executor_cpp'] and marker not in text['executor_h']:
            errors.append('executor safety: Spirit Elemental missing dedicated semantics: '+marker)

# Vigor Ball in this source is the direct legacy percent-power adapter. Require
# both the formula and visual play-code mapping before advertising it.
if damage_switch and 'SKILL_VIGOR_BALL' in damage_switch.group(0):
    for marker in ('Vigor_Ball_Damage_Percent[p]', 'SKILL_PLAY_VIGOR_BALL'):
        if marker not in text['executor_cpp']:
            errors.append('executor safety: Vigor Ball missing legacy adapter marker: '+marker)

# Prevent accidental direct DAT writes from the skill-loadout UI path.
for fn in ('ChangeSkillLoadout', 'AdjustBuffInterval', 'ResetSkillLoadout'):
    match = re.search(rf'int {fn}\(.*?\n\}}', text['system'], re.S)
    if not match:
        errors.append(f'missing function {fn}')
        continue
    body = match.group(0)
    if 'rsSave' in body or 'rsAddOfflineCharacterExp' in body:
        errors.append(f'{fn}: must persist through CompanionProfile, not the character DAT')

if errors:
    print('Companion Skill Loadout V2 + visual fallback fix: FAIL')
    for error in errors:
        print('  -', error)
    raise SystemExit(1)

print('Companion Skill Loadout V2 + visual fallback fix: PASS')
