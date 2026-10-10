#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[2]
checks={
'SrcGame/src/systems/ui/v2/legacy/LegacyQuestGateway.cpp':[
'constexpr int AmandaFinalizedSectionId = 0x7031;',
'finalized.finalizedGroup = true;',
'quest.State == 100 && quest.CODE == 0 && quest.BackUpCode != 0',
'sectionId == AmandaQuestSectionId || sectionId == AmandaFinalizedSectionId',
],
'SrcGame/src/systems/ui/v2/quests/QuestJournalWindow.cpp':[
'selectedSection->progressGroup', 'selectedSection->finalizedGroup',
'trackButton_->SetVisible(!inCodex && inProgress);', 'amandaNpcManaged',
],
'SrcGame/src/core/client/legacy_sin_runtime/sinMain.cpp':[
'Continuous-skill timers are gameplay status', 'cSkill.DrawUp();',
],
'Shared/server/legacy_runtime/OnSever_legacy_client.inl':[
'GetLocalizationLegacyValue', 'SummitApplyFixedNpcLocalizedName(dst.smCharInfo, src.npcId);',
],
'Shared/server/legacy_runtime/OnSever_legacy_server.inl':[
'GetLocalizationLegacyValue', 'SummitApplyFixedNpcLocalizedName(dst.smCharInfo, src.npcId);',
'SummitApplyFixedNpcLocalizedName(dst.smCharInfo, "npc.ji_woo");',
],
}
for rel,tokens in checks.items():
    text=(root/rel).read_text(errors='replace')
    for token in tokens:
        if token not in text: raise SystemExit(f'FAIL: {rel}: missing {token}')
# Guard the timer from being re-suppressed by main HUD ownership.
s=(root/'SrcGame/src/core/client/legacy_sin_runtime/sinMain.cpp').read_text(errors='replace')
needle='if (!ui::v2::legacy::SuppressLegacyMainHud())\n\t\tcSkill.DrawUp();'
if needle in s: raise SystemExit('FAIL: continuous-skill timer renderer is still suppressed with legacy main HUD')
print('PATCH 208 gate: OK — Amanda finalized/tracking, continuous-skill timers and fixed-NPC localized names are guarded')
