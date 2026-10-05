#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]

def read(rel: str) -> str:
    return (ROOT / rel).read_text(encoding='utf-8', errors='strict')

errors = []

def require(text: str, token: str, label: str) -> None:
    if token not in text:
        errors.append(f'{label}: missing {token!r}')

social_h = read('SrcGame/src/systems/ui/v2/social/SocialWindow.h')
social_cpp = read('SrcGame/src/systems/ui/v2/social/SocialWindow.cpp')
bridge_h = read('SrcGame/src/systems/ui/v2/legacy/LegacyUIV2Bridge.h')
bridge_cpp = read('SrcGame/src/systems/ui/v2/legacy/LegacyUIV2Bridge.cpp')
winmain = read('SrcGame/src/platform/windows/application/Winmain.cpp')

require(social_h, 'void WhisperSelectedFriend();', 'SocialWindow friend action')
require(social_h, 'Button* whisperButton_ = nullptr;', 'SocialWindow whisper button state')
require(social_cpp, '"Sussurrar"', 'SocialWindow Portuguese action label')
require(social_cpp, 'WhisperPartyPlayer(name.data());', 'SocialWindow legacy whisper backend bridge')
require(social_cpp, 'whisperButton_->SetVisible(selectedFriend);', 'SocialWindow friend-only action visibility')
require(bridge_h, 'bool OpenFriendsWindow();', 'V2 bridge friend entry declaration')
require(bridge_cpp, 'socialWindow->SetTab(SocialWindow::Tab::Party);', 'party entry deterministic tab')
require(bridge_cpp, 'bool OpenFriendsWindow()', 'V2 bridge friend entry implementation')
require(bridge_cpp, 'socialWindow->SetTab(SocialWindow::Tab::Friends);', 'friend entry deterministic tab')
require(winmain, 'ui::v2::legacy::OpenFriendsWindow()', 'legacy add-friend flow routed to V2 friends tab')

if errors:
    print('PATCH 202 gate: FAIL')
    for error in errors:
        print(f'- {error}')
    sys.exit(1)

print('PATCH 202 gate: OK — Friends V2 restores whisper action and add-friend flow opens the Friends tab directly')
