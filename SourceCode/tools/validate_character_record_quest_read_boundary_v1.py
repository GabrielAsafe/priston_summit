#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[1]
path = root / 'Shared/persistence/character_record/record_legacy_server.inl'
text = path.read_bytes().decode('latin1')

checks = [
    ('record no quest handler include', 'systems/gameplay/quests/server/cQuestHandler.h' not in text),
    ('record no cQuestHandler use', 'cQuestHandler' not in text),
    ('record uses quest repository', 'ServerLegacyQuestRepository()' in text),
    ('rank5 arena id preserved', 'gameplay::QuestId(QUESTID_T5_QUEST_ARENA)' in text),
    ('green liquid id preserved', 'gameplay::QuestId(QUESTID_T5_GREEN_LIQUID_STEP2)' in text),
    ('one-point quest loop preserved', 'gameplay::QuestId(onePointQuestIDs[i])' in text),
    ('db failure fail-closed comment preserved', 'QuestDB unavailable: never permit a point escalation' in text),
    ('rank normalization db guard preserved', 'if(gameplay::quests::legacy::ServerLegacyQuestRepository().TryIsCompleted(szName' in text),
]
failed=[]
for label, ok in checks:
    print(('PASS: ' if ok else 'FAIL: ') + label)
    if not ok: failed.append(label)
if failed:
    raise SystemExit(1)
print('character record quest read boundary: PASS')
