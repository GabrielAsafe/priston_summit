#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[2]
clan=(root/'SrcGame/src/systems/social/clan/tjclan.cpp').read_bytes().decode('latin1')
mouse=(root/'SrcGame/src/systems/presentation/opening/legacy_runtime/HoLinkHeader.cpp').read_text(errors='replace')
errors=[]
for t in ('isDrawClanMenu = 0;','ClanCharMove = 0;','ClanCLCharMove = 0;'):
 if t not in clan: errors.append('clan missing '+t)
for t in ('if(ClanCharMove && isDrawClanMenu)','if(ClanCLCharMove && isDrawClanMenu)'):
 if t not in mouse: errors.append('mouse gate missing '+t)
if errors:
 print('\n'.join('ERROR: '+x for x in errors));sys.exit(1)
print('PATCH 187 gate: OK')
