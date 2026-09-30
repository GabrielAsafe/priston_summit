#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
path=root/'SrcServer/src/core/server/legacy_runtime/PartySever.cpp'
text=path.read_bytes().decode('latin1')
checks=[
 ('party no quest handler include','systems/gameplay/quests/server/cQuestHandler.h' not in text),
 ('party no cQuestHandler use','cQuestHandler' not in text),
 ('party still publishes killer event','PublishMonsterKilled(lpPlayInfo, lpChar, false)' in text),
 ('party still publishes member event','PublishMonsterKilled(lpPartyPlayers[cnt], lpChar, true)' in text),
 ('party exp function preserved','int rsPLAYINFO::GetPartyExp(smCHAR* lpChar, rsPLAYINFO* lpPlayInfo)' in text),
]
failed=[]
for label,ok in checks:
 print(('PASS: ' if ok else 'FAIL: ')+label)
 if not ok: failed.append(label)
if failed: raise SystemExit(1)
print('party dead quest probe cleanup: PASS')
