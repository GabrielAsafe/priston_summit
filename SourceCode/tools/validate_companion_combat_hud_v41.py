#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
hud=(ROOT/'SrcGame/src/systems/ui/v2/hud/CompanionPartyHud.cpp').read_text(errors='ignore')
errors=[]
def req(label, needle):
    if needle not in hud: errors.append(f"{label}: missing {needle!r}")
for n in ('event.modifiers.shift && event.button == MouseButton::Left',
          'CompanionUiAction::CycleActiveSkill, entry.name, 1',
          'event.modifiers.shift && event.button == MouseButton::Middle',
          'CompanionUiAction::CycleBuffSkill, entry.name, 1'):
    req('quick skill control', n)
req('focus target control', 'CompanionUiTactic::OwnerFocus')
req('normal mode control retained', 'CompanionUiAction::SetMode')
req('dismiss retained', 'CompanionUiAction::Dismiss')
if errors:
    print('Companion Combat HUD V41: FAIL')
    for e in errors: print('  -', e)
    raise SystemExit(1)
print('Companion Combat HUD V41: PASS (row focus + primary active/support quick-cycle controls)')
