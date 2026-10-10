#!/usr/bin/env python3
from pathlib import Path
import re, sys

ROOT = Path(__file__).resolve().parents[2]
checks=[]

def need(rel, *tokens):
    text=(ROOT/rel).read_text(encoding='utf-8', errors='replace')
    for token in tokens:
        ok=token in text
        checks.append((ok, f'{rel}: {token[:90]}'))
    return text

server = need('Shared/server/legacy_runtime/OnSever_legacy_server.inl',
    'static void SummitApplyFixedNpcIdentity(',
    'target.wPlayClass[0] = decoded.wPlayClass[0];',
    'SummitApplyFixedNpcIdentity(dst.smCharInfo, decodedInfo, decodedMonster);',
    'char\tszFindPath[MAX_PATH];',
    'char\tszFilePath[MAX_PATH];',
    'nomeMapa = "Abismo do Mar 2 Andar";',
    'nomeMapa = "Ruinas de Mars";',
    'nomeMapa = "Ilha da Morte";',
    'nomeMapa = "Lar do Eragon";',
)
client = need('Shared/server/legacy_runtime/OnSever_legacy_client.inl',
    'static void SummitApplyFixedNpcIdentity(',
    'target.wPlayClass[0] = decoded.wPlayClass[0];',
    'SummitApplyFixedNpcIdentity(dst.smCharInfo, decodedInfo, decodedMonster);',
    'char\tszFindPath[MAX_PATH];',
    'char\tszFilePath[MAX_PATH];',
)
quest = need('SrcGame/src/systems/ui/v2/legacy/LegacyQuestGateway.cpp',
    'constexpr int AmandaQuestSectionId = 0x7030;',
    'chaQuest.sHaQuestElementary[kind]',
    'out.tracked = trackedSectionId == AmandaQuestSectionId',
    'trackedSectionId_ = AmandaQuestSectionId;',
    'section.title = "Instrutora Amanda";',
)
checks.append((quest.count('void AddCodexQuests(QuestJournalSnapshot& out, int selectedCodexQuestId)') == 1,
               'LegacyQuestGateway.cpp: AddCodexQuests definition is unique'))
need('SrcGame/src/systems/ui/v2/legacy/LegacyQuestGateway.h', 'mutable int selectedAmandaQuestId_ = 0;')

comp = need('SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp',
    'bool presentationChanged = false;',
    'SendRuntimeStateLocked(state, true);',
    'SendVisualStateLocked(state, true);',
    'stale HUD/visual state after natural death, expiry or ownership loss',
)

help_src = need('SrcGame/src/systems/social/clan/tjclan.cpp',
    'void InitHelp()', 'ui::v2::legacy::OpenGuideWindow()',
    'Guide/help has no gameplay legacy fallback after the UI V2 migration.')
# In the two entry points themselves, legacy LEVELUP_HELP must not be reopened.
for fn in ('InitHelp','InitGuideHelp'):
    m=re.search(rf'void {fn}\(\)\s*\{{(.*?)\n\}}', help_src, re.S)
    checks.append((bool(m) and 'cSinHelp.sinShowHelp(LEVELUP_HELP' not in m.group(1), f'{fn}: no LEVELUP_HELP fallback'))
need('SrcGame/src/systems/ui/v2/help/GuideWindow.cpp',
    'if (heading && raw == "Keyboard")', 'raw = "Teclado";',
    'if (raw == "(CH\'s name will be appeared in chat box)")')
need('SrcGame/src/systems/gameplay/teleport/ui/TCore.cpp',
    '"Laboratorio Secreto"', '"Arma Secreta"', '"Abismo do Mar 1\\272 Andar"', '"Abismo do Mar 2\\272 Andar"')
need('SrcGame/src/systems/social/party/ui/cPartyWindow.h',
    '#include <vector>', '#include "smPacket.h"', 'class smCHAR;', 'std::vector<sMENUOPT*> vOption;')

# The malformed Batch-02 member-call and duplicate player-id helper must stay gone.
checks.append(('->SendOnServerLegacySocket(lpsmSock' not in server, 'OnServer: malformed socket member-call absent'))
checks.append((server.count('SummitOnServerPlayerId(rsPLAYINFO* player)') == 1, 'OnServer: player-id helper unique'))

bad=[msg for ok,msg in checks if not ok]
for ok,msg in checks:
    print(('PASS: ' if ok else 'FAIL: ') + msg)
if bad:
    print(f'PATCH 206 gate: FAIL ({len(bad)} contract(s))')
    sys.exit(1)
print(f'PATCH 206 gate: OK — Amanda tracking/interaction, companion HUD retirement, Help V2/PT, fixed-NPC names, monster startup buffers and map labels are guarded ({len(checks)} contracts)')
