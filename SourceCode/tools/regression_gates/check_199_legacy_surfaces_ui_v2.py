#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[2]
errors=[]
def text(rel, enc='utf-8'):
    try:return (root/rel).read_text(encoding=enc)
    except UnicodeDecodeError:return (root/rel).read_text(encoding='cp1252')
bridge=text('SrcGame/src/systems/ui/v2/legacy/LegacyUIV2Bridge.cpp')
bridge_h=text('SrcGame/src/systems/ui/v2/legacy/LegacyUIV2Bridge.h')
sub=text('SrcGame/src/core/client/legacy_sin_runtime/sinSubMain.cpp')
main=text('SrcGame/src/core/client/legacy_sin_runtime/sinMain.cpp')
clan=text('SrcGame/src/systems/social/clan/tjclan.cpp', 'cp1252')
npc=text('SrcGame/src/systems/ui/v2/legacy/LegacyNpcDialogWindow.cpp')
for rel in (
 'SrcGame/src/systems/ui/v2/help/GuideWindow.cpp',
 'SrcGame/src/systems/ui/v2/help/GuideWindow.h',
 'SrcGame/src/systems/ui/v2/social/SocialWindow.cpp',
 'SrcGame/src/systems/ui/v2/social/SocialWindow.h',
 'SrcGame/src/systems/ui/v2/social/PartyStatusHud.cpp',
 'SrcGame/src/systems/ui/v2/social/PartyStatusHud.h'):
    if not (root/rel).exists(): errors.append('missing UI V2 surface: '+rel)
for token in ('BuildGuideWindow()', 'BuildSocialWindow()', 'BuildPartyStatusHud()', 'OpenGuideWindow()', 'OpenSocialWindow()', 'OpenQuestJournalWindow()'):
    if token not in bridge+bridge_h: errors.append('bridge missing '+token)
if 'OpenGuideWindow();' not in npc: errors.append('normal help guide entry still does not route to UI V2 guide')
if 'ui::v2::legacy::OpenGuideWindow()' not in clan: errors.append('direct legacy GuideHelp entry is not routed to UI V2')
if 'ui::v2::legacy::OpenSocialWindow()' not in sub: errors.append('ShowParty does not route to UI V2 social window')
if 'ui::v2::legacy::OpenQuestJournalWindow()' not in sub: errors.append('ShowQuest does not route to UI V2 quest journal')
if '\tcParty.Draw();' in main and '// cParty.Draw();' not in main: errors.append('legacy cParty HUD draw remains active')
for token in ('if (cParty.LButtonDown())', 'if(cParty.RButtonDown())'):
    active=[ln for ln in main.splitlines() if token in ln and not ln.lstrip().startswith('//')]
    if active: errors.append('legacy party HUD input remains active: '+token)
if errors:
    for e in errors: print('ERROR:',e,file=sys.stderr)
    sys.exit(1)
print('PATCH 199 gate: OK — Guide, Party/Friends and legacy quest entry surfaces are owned by UI V2')
