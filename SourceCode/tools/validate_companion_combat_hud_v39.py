#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
proto=(ROOT/'Shared/gameplay/companions/CompanionProtocol.h').read_text(errors='ignore')
server=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
hud=(ROOT/'SrcGame/src/systems/ui/v2/hud/CompanionPartyHud.cpp').read_text(errors='ignore')
errors=[]
def req(label, text, needle):
    if needle not in text: errors.append(f"{label}: missing {needle!r}")
req('protocol threat flag', proto, 'COMPANION_RUNTIME_STATUS_THREAT = 1u << 4')
for n in ('HasIncomingCompanionThreat', 'candidate->lpTargetChar == companion',
          'entry.statusFlags |= protocol::COMPANION_RUNTIME_STATUS_THREAT'):
    req('server threat snapshot', server, n)
req('HUD aggro marker', hud, 'effects += "  AGGRO"')
# No packet fields were added; retain the compact packet guard.
req('compact runtime packet guard', proto, 'sizeof(CompanionRuntimeStatePacket) < 1024')
if errors:
    print('Companion Combat HUD V39: FAIL')
    for e in errors: print('  -', e)
    raise SystemExit(1)
print('Companion Combat HUD V39: PASS (authoritative incoming-threat flag + AGGRO HUD marker)')
