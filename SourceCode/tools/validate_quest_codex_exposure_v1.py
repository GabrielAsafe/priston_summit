#!/usr/bin/env python3
from pathlib import Path
import json
import re
import sys

root = Path(__file__).resolve().parents[1]
quest_path = root / 'Content/canonical/gameplay_reference/quests/priston_tale_quests.pt-BR.json'
binding_path = root / 'Content/canonical/gameplay_reference/quests/runtime_bindings.json'
gateway_path = root / 'SrcGame/src/systems/ui/v2/legacy/LegacyQuestGateway.cpp'
model_path = root / 'SrcGame/src/systems/ui/v2/domain/QuestUIModels.h'
window_path = root / 'SrcGame/src/systems/ui/v2/quests/QuestJournalWindow.cpp'

quest_doc = json.loads(quest_path.read_text(encoding='utf-8'))
quest_count = sum(len(quest_doc.get(key, [])) for key in (
    'progression_quests', 'daily_quests', 'aline_daily_quests', 'weekly_quests'))
bindings = json.loads(binding_path.read_text(encoding='utf-8')).get('bindings', [])
verified = sum(1 for item in bindings if item.get('supported') is True)
unverified = [item for item in bindings if item.get('supported') is not True]

gateway = gateway_path.read_text(errors='replace')
model = model_path.read_text(errors='replace')
window = window_path.read_text(errors='replace')

failures = []
if quest_count != 88:
    failures.append(f'canonical quest count changed: expected 88, found {quest_count}')
if verified != 13:
    failures.append(f'verified runtime binding count changed: expected 13, found {verified}')
if len(unverified) != 1 or unverified[0].get('guide_id') != 'level-80-4-desafio-classe':
    failures.append('Tier5 must remain the single explicitly unverified binding')
if 'source.Quests(guides)' not in gateway:
    failures.append('Codex must enumerate the full canonical quest pack')
if 'source.SupportedQuests(guides)' in gateway:
    failures.append('Codex still hides documentary-only quests behind SupportedQuests')
if 'out.codex.runtimeVerified = guide.supported;' not in gateway:
    failures.append('Codex snapshots do not preserve verified runtime binding state')
if 'bool runtimeVerified = false;' not in model:
    failures.append('QuestCodexSnapshot lacks explicit runtime verification state')
if 'Runtime: binding verificado' not in window or 'sem binding confirmado' not in window:
    failures.append('Quest UI does not distinguish runtime-verified vs documentary entries')

if failures:
    for failure in failures:
        print('FAIL: ' + failure, file=sys.stderr)
    sys.exit(1)

print(f'quest codex exposure v1: PASS (canonical={quest_count}, runtime_verified={verified}, documentary_only={quest_count-verified})')
