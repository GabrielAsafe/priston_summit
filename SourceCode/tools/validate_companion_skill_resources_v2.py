#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
EXECUTOR = ROOT / 'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp'
CONTRACT = ROOT / 'Shared/gameplay/skills/v2/SkillResourceCommands.h'
ADAPTER_H = ROOT / 'SrcServer/src/systems/gameplay/skills/v2/legacy/LegacySkillResourceAdapter.h'
ADAPTER_CPP = ROOT / 'SrcServer/src/systems/gameplay/skills/v2/legacy/LegacySkillResourceAdapter.cpp'

executor = EXECUTOR.read_text('utf-8', errors='ignore')
contract = CONTRACT.read_text('utf-8', errors='ignore') if CONTRACT.exists() else ''
adapter_h = ADAPTER_H.read_text('utf-8', errors='ignore') if ADAPTER_H.exists() else ''
adapter_cpp = ADAPTER_CPP.read_text('utf-8', errors='ignore') if ADAPTER_CPP.exists() else ''

checks = []
def ck(ok, label):
    checks.append((bool(ok), label))
    print(('PASS' if ok else 'FAIL') + ': ' + label)

ck('class ISkillResourceCommands' in contract and 'Spend(EntityId caster, int mana, int stamina)' in contract,
   'GREEN skill resource command boundary exists')
ck('Restore(EntityId caster, int mana, int stamina)' in contract and
   'Drain(EntityId caster, int mana, int stamina)' in contract,
   'resource boundary owns clamped restore and drain mutations')
ck('LegacySkillResourceAdapter final : public ISkillResourceCommands' in adapter_h,
   'server legacy resource adapter implements the V2 command boundary')
ck('resolved->smCharInfo.Mana[0]' in adapter_cpp and 'resolved->smCharInfo.Stamina[0]' in adapter_cpp,
   'legacy MP/SP mutation is isolated in the adapter')
ck('#include "systems/gameplay/skills/v2/legacy/LegacySkillResourceAdapter.h"' in executor,
   'CompanionSkillExecutor consumes the resource adapter')
ck('InspectCompanionSkillResources' in executor and 'ServerLegacySkillResourceAdapter().Inspect' in executor,
   'cast admission reads MP/SP through the V2 adapter')
ck(executor.count('(void)SpendCompanionSkillResources(caster,') >= 15,
   'all companion cast/channel cost sites delegate MP/SP spending')

mana_direct = re.findall(
    r'caster->smCharInfo\.Mana\[0\]\s*=\s*\(short\)std::max\(0,\s*\(int\)caster->smCharInfo\.Mana\[0\]\s*-\s*(?:mana|manaDrain)\)',
    executor)
stamina_direct = re.findall(
    r'caster->smCharInfo\.Stamina\[0\]\s*=\s*\(short\)std::max\(0,\s*\(int\)caster->smCharInfo\.Stamina\[0\]\s*-\s*(?:stamina|staminaDrain)\)',
    executor)
ck(not mana_direct, 'no direct companion cast Mana cost writes remain')
ck(not stamina_direct, 'no direct companion cast Stamina cost writes remain')
ck(executor.count('RestoreCompanionSkillResources(') >= 5,
   'regen, mastery refill and Agony restore route through V2 resources')
ck(executor.count('DrainCompanionSkillResources(') >= 2,
   'Energy Shield drain routes through V2 resources')
current_writes = re.findall(
    r'(?:caster|target)->smCharInfo\.(?:Mana|Stamina)\[0\]\s*=', executor)
ck(not current_writes, 'CompanionSkillExecutor has no direct current Mana/Stamina writes')

if not all(ok for ok, _ in checks):
    print('companion skill resources v2: FAIL', file=sys.stderr)
    sys.exit(1)
print(f'companion skill resources v2: PASS ({len(checks)}/{len(checks)})')
