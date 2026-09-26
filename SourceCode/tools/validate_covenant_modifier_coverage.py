#!/usr/bin/env python3
"""Fail closed when Covenant content advertises a modifier with no declared delivery status."""
from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parents[1]
CONTENT=(ROOT/'Database/migrations/040_covenant_default_content.sql').read_text(errors='replace')

def fail(msg):
    print('Covenant modifier coverage: FAIL:',msg,file=sys.stderr); raise SystemExit(1)

# Only semantic gameplay keys, not localization keys.
keys=set(re.findall(r"'((?:xp|monster|combat|hunt|gathering|crafting|loot|fortune|ascendant|skill|vendor)\.[a-z0-9_.]+)'", CONTENT))

# Directly delivered by authoritative runtime hooks or chat/runtime capabilities.
live={
 'xp.monster_gain','monster.drop_chance','combat.damage_taken',
 'hunt.drop_chance','hunt.trophy_chance','hunt.trophy_refining_enabled',
 'gathering.surveyor_enabled','gathering.yield','gathering.extra_yield_chance',
 'gathering.special_material_chance','gathering.refined_material_chance',
 'loot.extra_roll_chance','vendor.blacksmith_sell_value',
 'crafting.reforge_enabled','crafting.forgemaster_enabled','crafting.master_artisan_enabled',
}
# These are capability labels whose actual effects are supplied by data-driven PowerDefinition/PowerModifier rows.
power_caps={'hunt.mark_enabled','fortune.tempt_fate_enabled','ascendant.focused_training_enabled'}
# Explicit downstream ownership: do not pretend these mutate legacy data until an authoritative subsystem contract exists.
extension=set()
covered=live|power_caps|extension
missing=sorted(keys-covered)
extra=sorted(covered-keys)
if missing: fail('unclassified content keys: '+', '.join(missing))
if extra: fail('coverage table contains keys no longer in content: '+', '.join(extra))

# Prove live hooks/capabilities exist in the owning surface.
contracts={
 'xp.monster_gain':['Shared/server/legacy_runtime/OnSever_legacy_server.inl','SrcServer/src/core/server/legacy_runtime/PartySever.cpp'],
 'monster.drop_chance':['Shared/server/legacy_runtime/OnSever_legacy_server.inl'],
 'combat.damage_taken':['Shared/server/legacy_runtime/OnSever_legacy_server.inl'],
 'hunt.drop_chance':['Shared/server/legacy_runtime/OnSever_legacy_server.inl'],
 'hunt.trophy_chance':['SrcServer/src/systems/gameplay/events/v2/GameplayEventRuntime.cpp'],
 'hunt.trophy_refining_enabled':['SrcServer/src/systems/gameplay/progression/v2/ProgressionRuntime.cpp'],
 'gathering.surveyor_enabled':['SrcServer/src/systems/gameplay/covenants/v2/CovenantChatCommand.cpp'],
 'gathering.yield':['SrcServer/src/systems/gameplay/mining/v2/MiningService.cpp'],
 'gathering.extra_yield_chance':['SrcServer/src/systems/gameplay/mining/v2/MiningService.cpp'],
 'gathering.special_material_chance':['SrcServer/src/systems/gameplay/mining/v2/MiningService.cpp'],
 'gathering.refined_material_chance':['SrcServer/src/systems/gameplay/mining/v2/MiningService.cpp'],
 'loot.extra_roll_chance':['Shared/server/legacy_runtime/OnSever_legacy_server.inl'],
 'vendor.blacksmith_sell_value':['Shared/server/legacy_runtime/OnSever_legacy_server.inl'],
 'crafting.reforge_enabled':['Shared/server/legacy_runtime/OnSever_legacy_server.inl'],
 'crafting.forgemaster_enabled':['Shared/server/legacy_runtime/OnSever_legacy_server.inl'],
 'crafting.master_artisan_enabled':['Shared/server/legacy_runtime/OnSever_legacy_server.inl'],
}
for key, paths in contracts.items():
    needle = 'kGatheringSurveyorEnabled' if key == 'gathering.surveyor_enabled' else key
    if not any(needle in (ROOT/p).read_text(errors='replace') for p in paths): fail(f'live key {key} lost owning hook')

readme=(ROOT/'Shared/gameplay/covenants/v2/README.md').read_text(errors='replace')
for key in extension:
    if key not in readme and key.split('.')[0].capitalize() not in readme:
        # Names may be described semantically; specific assertions below are stronger.
        pass
if extension:
    fail('all seeded Covenant modifiers must be live; unexpected extension keys remain')
print('Covenant modifier coverage: PASS')
