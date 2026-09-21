#include "systems/ui/v2/legacy/LegacyQuestGateway.h"

#include <algorithm>
#include <string>

#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/gameplay/quests/ui/cQuestWindow.h"
#include "systems/ui/messages/legacy_sin/sinMsg.h"

namespace ui::v2::legacy
{
namespace
{
struct QuestRef
{
    sSPOILER* section = nullptr;
    sQUEST* quest = nullptr;
};

QuestRef FindQuest(int sectionId, int questId)
{
    sSPOILER* section = cQuestWindow.GetSpoiler(sectionId);
    if (!section)
        return {};
    for (auto* quest : section->vQuests)
        if (quest && quest->ID == questId)
            return {section, quest};
    return {};
}

std::string SectionTitle(int id)
{
    switch (id)
    {
        case cQUESTWINDOW::SPOILER_Repeated: return "Repetiveis";
        case cQUESTWINDOW::SPOILER_DAILY: return "Diarias";
        case cQUESTWINDOW::SPOILER_QUESTS: return "Quests";
        case cQUESTWINDOW::SPOILER_PROGRESS: return "Em andamento";
        case cQUESTWINDOW::SPOILER_FINALIZED: return "Finalizadas";
        default: return "Quests";
    }
}

bool ComputeComplete(sQUEST& quest)
{
    const int count = std::clamp(quest.QuestInfo.MonsterCount, 0, 4);
    if (count <= 0)
        return quest.Complete;

    bool complete = true;
    for (int i = 0; i < count; ++i)
    {
        if (quest.QuestInfo.MonsterKilled[i] < quest.QuestInfo.MonsterCounts[i])
        {
            complete = false;
            break;
        }
    }
    quest.Complete = complete;
    return complete;
}

QuestEntrySnapshot MakeQuestSnapshot(sSPOILER& section, sQUEST& quest, int trackedSectionId, int trackedQuestId)
{
    QuestEntrySnapshot out;
    out.id = quest.ID;
    out.kind = quest.Kind;
    out.sectionId = section.ID;
    out.title = quest.szTitle;
    out.name = quest.QuestInfo.QuestName;
    out.description = quest.QuestInfo.QuestDesc;
    out.levelMin = quest.QuestInfo.LevelMin;
    out.levelMax = quest.QuestInfo.LevelMax;
    out.questType = quest.QuestInfo.QuestType;
    out.party = quest.QuestInfo.PartyFlag != 0;
    out.active = quest.Active;
    out.complete = ComputeComplete(quest);
    out.selected = section.pSelectedQuest == &quest;
    out.tracked = trackedSectionId == section.ID && trackedQuestId == quest.ID;

    const int monsterCount = std::clamp(quest.QuestInfo.MonsterCount, 0, 4);
    out.objectives.reserve(static_cast<std::size_t>(monsterCount));
    for (int i = 0; i < monsterCount; ++i)
    {
        QuestObjectiveSnapshot objective;
        objective.name = quest.QuestInfo.MonsterNames[i];
        if (quest.QuestInfo.MonsterMaps[i] >= 0)
            objective.location = "Mapa " + std::to_string(quest.QuestInfo.MonsterMaps[i]);
        objective.current = quest.QuestInfo.MonsterKilled[i];
        objective.required = quest.QuestInfo.MonsterCounts[i];
        objective.complete = objective.current >= objective.required;
        out.objectives.push_back(std::move(objective));
    }

    const int rewardCount = std::clamp(quest.QuestInfo.RewardCount, 0, 4);
    out.rewards.reserve(static_cast<std::size_t>(rewardCount));
    for (int i = 0; i < rewardCount; ++i)
    {
        QuestRewardSnapshot reward;
        reward.name = quest.QuestInfo.RewardNames[i];
        reward.count = quest.QuestInfo.RewardCounts[i];
        out.rewards.push_back(std::move(reward));
    }
    out.rewardGold = quest.QuestInfo.RewardGold;
    out.rewardExp = static_cast<std::int64_t>(quest.QuestInfo.RewardExp);
    out.rewardClanExp = static_cast<std::int64_t>(quest.QuestInfo.RewardClanExp);
    out.rewardCoin = quest.QuestInfo.RewardCoin;
    out.rewardTCoin = quest.QuestInfo.RewardTCoin;
    return out;
}

constexpr int ClassicQuestSectionId = 0x7010;

QuestEntrySnapshot MakeClassicChangeJobQuest(int id, int level, const char* title, const sQUEST_CHANGEJOB& quest, bool selected)
{
    QuestEntrySnapshot out;
    out.id = id;
    out.kind = -1;
    out.sectionId = ClassicQuestSectionId;
    out.title = title;
    out.name = out.title;
    out.description = "Quest classica de mudanca de classe. Estado legado: " + std::to_string(quest.State) +
                      ". Dialogo, progressao e recompensa continuam usando as regras originais.";
    out.levelMin = level;
    out.active = false; // read-only mirror; master/NPC owns state transitions
    out.complete = false;
    out.selected = selected;
    return out;
}

QuestEntrySnapshot MakeClassicChangeJobQuest(int id, int level, const char* title, const sQUEST_CHANGEJOB3& quest, bool selected)
{
    QuestEntrySnapshot out;
    out.id = id;
    out.kind = -1;
    out.sectionId = ClassicQuestSectionId;
    out.title = title;
    out.name = out.title;
    out.description = "Quest classica de mudanca de classe. Estado legado: " + std::to_string(quest.State) +
                      ". Dialogo, progressao e recompensa continuam usando as regras originais.";
    out.levelMin = level;
    out.active = false; // read-only mirror; master/NPC owns state transitions
    out.complete = false;
    out.selected = selected;

    if (quest.Monster[1] > 0)
    {
        QuestObjectiveSnapshot objective;
        objective.name = "Alvo da mudanca de classe";
        objective.current = std::max(0, static_cast<int>(quest.Monster[0]));
        objective.required = std::max(0, static_cast<int>(quest.Monster[1]));
        objective.complete = objective.required > 0 && objective.current >= objective.required;
        out.objectives.push_back(std::move(objective));
    }
    return out;
}

QuestEntrySnapshot MakeClassicLevelQuest(int id, int level, const char* monsterName, const sQUEST_CHANGEJOB3& quest, bool selected)
{
    QuestEntrySnapshot out;
    out.id = id;
    out.kind = -1;
    out.sectionId = ClassicQuestSectionId;
    out.title = "Missao classica - Nivel " + std::to_string(level);
    out.name = out.title;
    out.description = "Quest mantida pelo sistema classico. Interacao, aceite e recompensa continuam usando as regras originais.";
    out.levelMin = level;
    out.active = false; // read-only mirror; NPC/dialog owns state transitions
    out.complete = quest.State >= SIN_QUEST_COMPLETE;
    out.selected = selected;

    if (quest.Monster[1] > 0)
    {
        QuestObjectiveSnapshot objective;
        objective.name = monsterName && *monsterName ? monsterName : "Objetivo da quest";
        objective.current = std::max(0, static_cast<int>(quest.Monster[0]));
        objective.required = std::max(0, static_cast<int>(quest.Monster[1]));
        objective.complete = objective.required > 0 && objective.current >= objective.required;
        out.objectives.push_back(std::move(objective));
    }
    return out;
}

void AddClassicLevelQuests(QuestJournalSnapshot& out, int selectedClassicQuestId)
{
    QuestSectionSnapshot section;
    section.id = ClassicQuestSectionId;
    section.title = "Quests classicas";

    auto add = [&](int id, int level, const char* monster, const sQUEST_CHANGEJOB3& quest)
    {
        if (quest.CODE)
            section.quests.push_back(MakeClassicLevelQuest(id, level, monster, quest, selectedClassicQuestId == id));
    };
    auto addChangeJob = [&](int id, int level, const char* title, const auto& quest)
    {
        if (quest.CODE)
            section.quests.push_back(MakeClassicChangeJobQuest(id, level, title, quest, selectedClassicQuestId == id));
    };

    addChangeJob(11001, 20, "Mudanca de classe - etapa 1", sinQuest_ChangeJob);
    addChangeJob(11002, 20, "Mudanca de classe - etapa 1 (rota alternativa)", sinQuest_ChangeJob2);
    addChangeJob(11003, 40, "Mudanca de classe - etapa 2", sinQuest_ChangeJob3);
    addChangeJob(11004, 60, "Mudanca de classe - etapa 3", sinQuest_ChangeJob4);

    add(10030, 30, sinLevelQuestMonster[0], sinQuest_Level30);
    add(10055, 55, sinLevelQuestMonster[1], sinQuest_Level55);
    add(10155, 55, sinLevelQuestMonster[2], sinQuest_Level55_2);
    add(10070, 70, sinLevelQuestMonster[3], sinQuest_Level70);
    add(10080, 80, sinLevelQuestMonster[4], sinQuest_Level80);
    add(10085, 85, sinLevelQuestMonster[5], sinQuest_Level85);
    add(10090, 90, sinLevelQuestMonster[6], sinQuest_Level90);
    add(10180, 80, nullptr, sinQuest_Level80_2);

    const char* level90Monster = nullptr;
    if (sinQuest_Level90_2.Kind >= 1 && sinQuest_Level90_2.Kind <= 3)
        level90Monster = sinLevelQuest90_2Mon[sinQuest_Level90_2.Kind - 1];
    add(10190, 90, level90Monster, sinQuest_Level90_2);

    if (!section.quests.empty())
    {
        if (selectedClassicQuestId)
        {
            out.selectedSectionId = ClassicQuestSectionId;
            out.selectedQuestId = selectedClassicQuestId;
        }
        out.sections.push_back(std::move(section));
    }
}
}

QuestJournalSnapshot LegacyQuestGateway::GetSnapshot() const
{
    QuestJournalSnapshot out;
    out.valid = !cQuestWindow.vTabs.empty();
    out.trackedSectionId = trackedSectionId_;
    out.trackedQuestId = trackedQuestId_;

    if (cQuestWindow.pSelectedSpoiler && cQuestWindow.pSelectedSpoiler->pSelectedQuest)
    {
        out.selectedSectionId = cQuestWindow.pSelectedSpoiler->ID;
        out.selectedQuestId = cQuestWindow.pSelectedSpoiler->pSelectedQuest->ID;
    }

    for (auto* tab : cQuestWindow.vTabs)
    {
        if (!tab)
            continue;
        for (auto* section : tab->vSpoilers)
        {
            if (!section)
                continue;
            QuestSectionSnapshot sectionSnapshot;
            sectionSnapshot.id = section->ID;
            sectionSnapshot.title = SectionTitle(section->ID);
            sectionSnapshot.progressGroup = section->ID == cQUESTWINDOW::SPOILER_PROGRESS;
            sectionSnapshot.finalizedGroup = section->ID == cQUESTWINDOW::SPOILER_FINALIZED;
            sectionSnapshot.quests.reserve(section->vQuests.size());
            for (auto* quest : section->vQuests)
                if (quest)
                    sectionSnapshot.quests.push_back(MakeQuestSnapshot(*section, *quest, trackedSectionId_, trackedQuestId_));
            out.sections.push_back(std::move(sectionSnapshot));
        }
    }
    AddClassicLevelQuests(out, selectedClassicQuestId_);
    out.valid = out.valid || !out.sections.empty();
    return out;
}

QuestEntrySnapshot LegacyQuestGateway::GetTrackedQuest() const
{
    const QuestRef ref = FindQuest(trackedSectionId_, trackedQuestId_);
    if (!ref.section || !ref.quest)
        return {};
    return MakeQuestSnapshot(*ref.section, *ref.quest, trackedSectionId_, trackedQuestId_);
}

bool LegacyQuestGateway::SelectQuest(int sectionId, int questId)
{
    if (sectionId == ClassicQuestSectionId)
    {
        QuestJournalSnapshot snapshot;
        AddClassicLevelQuests(snapshot, questId);
        for (const auto& section : snapshot.sections)
            for (const auto& quest : section.quests)
                if (quest.id == questId)
                {
                    selectedClassicQuestId_ = questId;
                    return true;
                }
        return false;
    }
    selectedClassicQuestId_ = 0;
    const QuestRef ref = FindQuest(sectionId, questId);
    if (!ref.section || !ref.quest)
        return false;

    // cQUESTWINDOW::handlePacket(full) historically applies the response to
    // pSelectedSpoiler. Keep this pointer synchronized before sending the
    // legacy request so the asynchronous response has the correct target.
    cQuestWindow.pSelectedSpoiler = ref.section;
    return ref.section->SetQuest(ref.quest);
}

bool LegacyQuestGateway::RequestAcceptSelected()
{
    sSPOILER* section = cQuestWindow.pSelectedSpoiler;
    if (!section || !section->pSelectedQuest ||
        section->ID == cQUESTWINDOW::SPOILER_PROGRESS ||
        section->ID == cQUESTWINDOW::SPOILER_FINALIZED ||
        !section->pSelectedQuest->Active)
        return false;
    section->StartQuest(true);
    return true;
}

bool LegacyQuestGateway::RequestCancelSelected()
{
    sSPOILER* section = cQuestWindow.pSelectedSpoiler;
    if (!section || section->ID != cQUESTWINDOW::SPOILER_PROGRESS || !section->pSelectedQuest)
        return false;
    if (ComputeComplete(*section->pSelectedQuest))
        return false;
    section->CancelQuest(true);
    return true;
}

bool LegacyQuestGateway::ClaimSelectedReward()
{
    sSPOILER* section = cQuestWindow.pSelectedSpoiler;
    if (!section || section->ID != cQUESTWINDOW::SPOILER_PROGRESS || !section->pSelectedQuest)
        return false;
    if (!ComputeComplete(*section->pSelectedQuest))
        return false;

    const int claimedQuest = section->pSelectedQuest->ID;
    section->GetReward();
    if (trackedSectionId_ == section->ID && trackedQuestId_ == claimedQuest)
    {
        trackedSectionId_ = 0;
        trackedQuestId_ = 0;
    }
    return true;
}

bool LegacyQuestGateway::TrackSelected()
{
    sSPOILER* section = cQuestWindow.pSelectedSpoiler;
    if (!section || section->ID != cQUESTWINDOW::SPOILER_PROGRESS || !section->pSelectedQuest)
        return false;
    trackedSectionId_ = section->ID;
    trackedQuestId_ = section->pSelectedQuest->ID;
    return true;
}

bool LegacyQuestGateway::EnsureBackendOpen()
{
    if (cQuestWindow.vTabs.empty())
        cQuestWindow.Open();
    else
        cQuestWindow.OpenFlag = true;
    return cQuestWindow.OpenFlag;
}

void LegacyQuestGateway::HideBackend()
{
    // Keep the populated vectors alive for the tracker and for asynchronous
    // progress packets, but release the legacy modal/open state so gameplay is
    // not blocked after the V2 journal closes.
    cQuestWindow.OpenFlag = false;
}

void LegacyQuestGateway::ShutdownBackend()
{
    if (!cQuestWindow.vTabs.empty())
        cQuestWindow.Close();
    else
        cQuestWindow.OpenFlag = false;
    trackedSectionId_ = 0;
    trackedQuestId_ = 0;
    selectedClassicQuestId_ = 0;
}
}
