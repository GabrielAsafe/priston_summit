#!/usr/bin/env python3
from pathlib import Path
import re, sys

ROOT = Path(__file__).resolve().parents[1]
checks = []

def require(path, needle, label):
    text=(ROOT/path).read_text('utf-8', errors='ignore')
    ok=needle in text
    checks.append((ok,label))
    if not ok:
        print(f'FAIL: {label}: missing {needle!r} in {path}')
    return ok

require('Shared/gameplay/covenants/v2/CovenantUIProtocol.h','OPCODE_COVENANT_UI_ACTION','protocol action opcode')
require('Shared/gameplay/covenants/v2/CovenantUIProtocol.h','OPCODE_COVENANT_UI_STATE','protocol state opcode')
require('SrcServer/src/systems/gameplay/covenants/v2/CovenantUIHandler.cpp','ActivateServerCovenant','server activate uses runtime')
require('SrcServer/src/systems/gameplay/covenants/v2/CovenantUIHandler.cpp','UseServerCovenantPower','server power uses runtime')
require('SrcServer/src/systems/gameplay/covenants/v2/CovenantUIHandler.cpp','SetServerCovenantArtisanMode','server artisan uses runtime')
require('SrcServer/src/systems/gameplay/covenants/v2/CovenantUIHandler.cpp','RefineServerCovenantTrophy','server refine uses runtime')
require('Shared/server/legacy_runtime/OnSever_legacy_server.inl','HandleCovenantUIAction','legacy server dispatch wired')
require('SrcGame/src/systems/network/client/netplay.cpp','ApplyLegacyCovenantStatePacket','client packet dispatch wired')
require('SrcGame/src/systems/ui/v2/legacy/LegacyCovenantGateway.cpp','OPCODE_COVENANT_UI_ACTION','client gateway sends structured action')
require('SrcGame/src/systems/ui/v2/windows/CharacterWindow.cpp','CharacterTab::Covenants','character shell has Covenant tab')
require('SrcGame/src/systems/ui/v2/legacy/LegacyUIV2Bridge.cpp',"sinGetKeyClick('N')",'N hotkey wired')
require('SrcGame/src/systems/ui/v2/pages/CovenantsPage.cpp','COVENANTS','Covenant page renders')

require('SrcGame/src/systems/ui/v2/legacy/CovenantEffectHud.cpp','BadgeFor','Covenant HUD maps modifiers to semantic Covenant badges')
require('SrcGame/src/systems/ui/v2/legacy/CovenantEffectHud.cpp','"DROP"','Covenant HUD exposes a drop/loot badge')
require('SrcGame/src/systems/ui/v2/legacy/CovenantEffectHud.cpp','"GATH"','Covenant HUD exposes a gathering badge')
require('SrcGame/src/systems/ui/v2/legacy/CovenantEffectHud.cpp','"CRFT"','Covenant HUD exposes a crafting badge')
hud=(ROOT/'SrcGame/src/systems/ui/v2/legacy/CovenantEffectHud.cpp').read_text('utf-8', errors='ignore')
checks.append(('UpKeepItemTGAImage' not in hud,'Covenant HUD does not borrow unrelated premium upkeep icons'))
require('SrcGame/src/systems/ui/v2/legacy/LegacyUIV2Bridge.cpp','covenantEffectHud.Render','Covenant HUD renders from authoritative snapshot')
require('SrcGame/src/systems/ui/v2/legacy/LegacyCovenantGateway.cpp','void LegacyCovenantGateway::Tick','gateway periodically refreshes Covenant state')
require('SrcServer/src/systems/gameplay/covenants/v2/CovenantUIHandler.cpp','COVENANT_UI_HARVEST_MINING','server handles mining UI action')
require('SrcServer/src/systems/gameplay/covenants/v2/CovenantUIHandler.cpp','COVENANT_UI_HARVEST_GATHERING','server handles gathering UI action')
require('SrcGame/src/systems/ui/v2/pages/CovenantsPage.cpp','RECURSOS COLETADOS','Gatherer resource inventory is player-facing')

# Structured UI must not fall back to writing slash commands into chat.
gateway=(ROOT/'SrcGame/src/systems/ui/v2/legacy/LegacyCovenantGateway.cpp').read_text('utf-8', errors='ignore')
if '/covenant' in gateway:
    checks.append((False,'gateway does not synthesize chat commands'))
    print('FAIL: LegacyCovenantGateway contains /covenant text')
else:
    checks.append((True,'gateway does not synthesize chat commands'))

# Wire packet remains below the historical 4 KB pressure threshold by contract.
proto=(ROOT/'Shared/gameplay/covenants/v2/CovenantUIProtocol.h').read_text('utf-8', errors='ignore')
checks.append(('sizeof(CovenantUIStatePacket) < 4096' in proto,'protocol has 4KB static assert'))

failed=[label for ok,label in checks if not ok]
if failed:
    print(f'Covenant UI validation: FAIL ({len(failed)}/{len(checks)})')
    sys.exit(1)
print(f'Covenant UI validation: PASS ({len(checks)}/{len(checks)})')
