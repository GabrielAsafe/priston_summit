#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
text=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_bytes().decode('latin1')
adapter=(root/'SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestQueryIngressAdapter.cpp').read_text()
checks=[
 ('onserver no direct spoiler call','cQuestHandler.GetSpoilerInfo' not in text),
 ('onserver uses quest query adapter',text.count('ServerLegacyQuestQueryIngressAdapter().SendSpoilerInfo') >= 2),
 ('spoiler number preserved',text.count('IdOf(lpPlayInfo), 9)') >= 2),
 ('adapter owns spoiler handler','cQuestHandler.GetSpoilerInfo' in adapter),
]
failed=[]
for label,ok in checks:
 print(('PASS: ' if ok else 'FAIL: ')+label)
 if not ok: failed.append(label)
if failed: raise SystemExit(1)
print('onserver quest spoiler boundary: PASS')
