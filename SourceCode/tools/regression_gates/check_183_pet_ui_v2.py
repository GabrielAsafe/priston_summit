#!/usr/bin/env python3
from pathlib import Path
import sys
r=Path(__file__).resolve().parents[2]
pet=(r/'SrcGame/src/systems/ui/help/legacy_sin/sinHelpPet.cpp').read_text(encoding='utf-8',errors='replace')
hud=(r/'SrcGame/src/systems/ui/v2/legacy/LegacyNotificationHud.cpp').read_text(encoding='utf-8',errors='replace')
req=[('pet suppression','OwnsLegacyNotificationUI()',pet),('short pet notice','cHelpPet.szPetBuff',hud),('tutorial pet notice','cHelpPet.HelpMsgId',hud),('pet activity ownership','cHelpPet.MsgFlag && cHelpPet.szPetBuff[0]',hud)]
miss=[name for name,tok,text in req if tok not in text]
if miss: print('PATCH 183 missing: '+', '.join(miss),file=sys.stderr);sys.exit(1)
print('PATCH 183 gate: OK')
