#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
gw=(ROOT/'SrcGame/src/systems/ui/v2/legacy/LegacyCompanionGateway.cpp').read_text(errors='ignore')
model=(ROOT/'SrcGame/src/systems/ui/v2/domain/CompanionUIModels.h').read_text(errors='ignore')
hud=(ROOT/'SrcGame/src/systems/ui/v2/hud/CompanionPartyHud.cpp').read_text(errors='ignore')
proto=(ROOT/'Shared/gameplay/companions/CompanionProtocol.h').read_text(errors='ignore')
errors=[]
def req(label,text,needle):
    if needle not in text: errors.append(f"{label}: missing {needle!r}")
req('canonical lookup',gw,'StableCompanionSkillName')
req('stable text avoids DAT names',gw,'case SKILL_FORCE_OF_NATURE: return "Force of Nature";')
req('effect model name',model,'std::string name;')
req('gateway assigns effect name',gw,'.name = StableCompanionSkillName(source.effects[effect].effectCode)')
for n in ('effects = "Buff: " + firstEffect.name', 'firstEffect.remainingMs', 'entry.effectCount - 1'):
    req('HUD effect detail',hud,n)
req('packet remains compact',proto,'sizeof(CompanionRuntimeStatePacket) < 1024')
if errors:
    print('Companion Combat HUD V40: FAIL')
    for e in errors: print('  -',e)
    raise SystemExit(1)
print('Companion Combat HUD V40: PASS (canonical positive-effect name + remaining time, no packet growth)')
