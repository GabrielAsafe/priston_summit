#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
paths={
 'protocol':ROOT/'Shared/gameplay/companions/CompanionProtocol.h',
 'profile':ROOT/'SrcServer/src/systems/gameplay/companions/CompanionProfileStore.cpp',
 'store_h':ROOT/'SrcServer/src/systems/gameplay/companions/CompanionTacticsStore.h',
 'store':ROOT/'SrcServer/src/systems/gameplay/companions/CompanionTacticsStore.cpp',
 'system':ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp',
 'models':ROOT/'SrcGame/src/systems/ui/v2/domain/CompanionUIModels.h',
 'gateway':ROOT/'SrcGame/src/systems/ui/v2/legacy/LegacyCompanionGateway.cpp',
 'page':ROOT/'SrcGame/src/systems/ui/v2/pages/CompanionsPage.cpp',
}
text={k:p.read_text(errors='ignore') for k,p in paths.items()}
errors=[]
def req(label,key,needle):
    if needle not in text[key]: errors.append(f"{label}: missing {needle!r} in {paths[key].relative_to(ROOT)}")

for needle in ('COMPANION_UI_PROTOCOL_VERSION = 6','COMPANION_UI_TACTIC_RULES = 4',
               'COMPANION_UI_TOGGLE_CUSTOM_TACTICS','COMPANION_UI_MOVE_TACTIC_RULE',
               'COMPANION_TACTIC_CONDITION_SELF_HP_BELOW','COMPANION_TACTIC_CONDITION_PARTY_THREATENED',
               'COMPANION_TACTIC_ACTION_RETREAT','COMPANION_TACTIC_ACTION_HOLD_POSITION',
               'customTacticsEnabled','CompanionUITacticRule tacticRules'):
    req('protocol v6 custom tactics','protocol',needle)

# The established profile ABI must remain untouched; custom tactics are a new sidecar.
req('legacy profile ABI','profile','static_assert(sizeof(CompanionProfileDiskV1) == 84')
for needle in ('CompanionTacticsConfig','LoadCompanionTactics','SaveCompanionTactics'):
    req('separate tactics store','store_h',needle)
for needle in ('COMPANION_TACTICS_MAGIC','COMPANION_TACTICS_VERSION = 1','sizeof(CompanionTacticsDiskV1) == 40',
               'DataServer\\\\CompanionProfiles','%08X.tac','ComputeChecksum','MoveFileExA',
               'MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH'):
    req('versioned atomic sidecar','store',needle)
req('custom disabled by default','store','enabled(false)')
for needle in ('SELF_HP_BELOW','OWNER_HAS_TARGET','PARTY_THREATENED','ATTACK_NEAREST'):
    req('default rule set','store',needle)

for needle in ('LoadCompanionTactics(state.owner->getAccount(), characterName, &entry->customTactics)',
               'CountNearbyHostiles','CustomTacticConditionMatches','EvaluateCustomTactic',
               'for(int i = 0; i < protocol::COMPANION_UI_TACTIC_RULES; ++i)',
               'EvaluateCustomTactic(profile->customTactics',
               'COMPANION_TACTIC_ACTION_ATTACK_OWNER_TARGET','COMPANION_TACTIC_ACTION_DEFEND_PARTY',
               'COMPANION_TACTIC_ACTION_ATTACK_NEAREST','COMPANION_TACTIC_ACTION_RETREAT',
               'COMPANION_TACTIC_ACTION_HOLD_POSITION','UpdateCustomTactics','SaveCompanionTactics',
               'packet.customTacticsEnabled','packet.tacticRules[tacticRule]',
               'COMPANION_UI_TOGGLE_TACTIC_RULE','COMPANION_UI_CYCLE_TACTIC_CONDITION',
               'COMPANION_UI_CYCLE_TACTIC_ACTION','COMPANION_UI_CYCLE_TACTIC_THRESHOLD',
               'COMPANION_UI_MOVE_TACTIC_RULE'):
    req('server rule engine','system',needle)
# Tactics must select intent, not execute combat/resource semantics themselves.
if 'TryExecuteCompanionDamageLoadout' in text['store'] or 'TryExecuteCompanionSupportLoadout' in text['store']:
    errors.append('separation: persistence/rule store must not execute skills')

for needle in ('ToggleCustomTactics','ToggleTacticRule','CycleTacticCondition','CycleTacticAction',
               'CycleTacticThreshold','MoveTacticRule','customTacticsEnabled','tacticRules'):
    req('client model','models',needle)
for needle in ('COMPANION_UI_TOGGLE_CUSTOM_TACTICS','COMPANION_UI_TOGGLE_TACTIC_RULE',
               'COMPANION_UI_CYCLE_TACTIC_CONDITION','COMPANION_UI_CYCLE_TACTIC_ACTION',
               'COMPANION_UI_CYCLE_TACTIC_THRESHOLD','COMPANION_UI_MOVE_TACTIC_RULE',
               'packet.customTacticsEnabled','packet.tacticRules'):
    req('gateway v6 mapping','gateway',needle)
for needle in ('Taticas custom','TACTICS CUSTOM','CustomConditionName','CustomActionName',
               'ToggleCustomTactics','ToggleTacticRule','CycleTacticCondition','CycleTacticAction',
               'CycleTacticThreshold','MoveTacticRule','Prioridade: regra 1 -> 4'):
    req('rule editor UI','page',needle)

if errors:
    print('Companion Custom Tactics V42: FAIL')
    for e in errors: print('  -',e)
    raise SystemExit(1)
print('Companion Custom Tactics V42: PASS (separate sidecar + ordered intent rules + persisted UI editor)')
