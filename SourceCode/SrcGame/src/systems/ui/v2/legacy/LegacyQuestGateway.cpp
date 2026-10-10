#include "systems/ui/v2/legacy/LegacyQuestGateway.h"

#include <algorithm>
#include <string>
#include <vector>

#include "compat/legacy/sin/sinLinkHeader.h"
#include "content/v1/gameplay_reference/content/GameplayReferenceRuntime.h"
#include "systems/gameplay/quests/ui/cQuestWindow.h"
#include "systems/ui/messages/legacy_sin/sinMsg.h"

namespace ui::v2::legacy
{
namespace
{
using summit::content::gameplay_reference::QuestGuideEntry;

constexpr int ClassicQuestSectionId = 0x7010;
constexpr int CodexQuestSectionId = 0x7020;
constexpr int AmandaQuestSectionId = 0x7030;
constexpr int AmandaFinalizedSectionId = 0x7031;

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

int CodexQuestId(const std::string& guideId)
{
    std::uint32_t hash = 2166136261u;
    for (std::size_t i = 0; i < guideId.size(); ++i)
    {
        hash ^= static_cast<unsigned char>(guideId[i]);
        hash *= 16777619u;
    }
    return static_cast<int>(0x60000000u | (hash & 0x0fffffffu));
}

void CopyGuide(QuestEntrySnapshot& out, const QuestGuideEntry& guide)
{
    out.codex.available = true;
    out.codex.runtimeVerified = guide.supported;
    out.codex.guideId = guide.guideId;
    out.codex.type = guide.type;
    out.codex.category = guide.category;
    out.codex.source = guide.source;
    out.codex.lore = guide.lore;
    out.codex.repeatable = guide.repeatable;
    out.codex.prerequisites = guide.prerequisites;
    out.codex.classes = guide.classes;
    out.codex.factions = guide.factions;
    out.codex.stages = guide.stages;
    out.codex.objectives = guide.objectives;
    out.codex.npcs = guide.npcs;
    out.codex.locations = guide.locations;
    out.codex.questItems = guide.questItems;
    out.codex.rewards = guide.rewards;
    out.codex.notes = guide.notes;
}

void EnrichWithGuide(QuestEntrySnapshot& out, int runtimeId)
{
    auto& source = summit::content::gameplay_reference::GameplayReferenceRuntime();
    if (!source.IsValid())
        return;

    QuestGuideEntry guide;
    bool found = runtimeId != 0 && source.FindByRuntimeId(runtimeId, guide);
    if (!found && !out.name.empty())
        found = source.FindByName(out.name.c_str(), guide);
    if (!found && !out.title.empty())
        found = source.FindByName(out.title.c_str(), guide);
    if (!found)
        return;

    CopyGuide(out, guide);
    // Classic mirrors historically had placeholder names/descriptions. Once a
    // verified guide binding exists, prefer the user-facing quest name while
    // retaining the live counters/rewards from the runtime snapshot.
    if (out.sectionId == ClassicQuestSectionId)
    {
        out.title = guide.name;
        out.name = guide.name;
    }
}

bool ComputeComplete(sQUEST& quest)
{
    const int count = std::clamp(quest.QuestInfo.MonsterCount, 0, QUEST_FULL_ENTRY_MAX);
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

    const int monsterCount = std::clamp(quest.QuestInfo.MonsterCount, 0, QUEST_FULL_ENTRY_MAX);
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

    const int rewardCount = std::clamp(quest.QuestInfo.RewardCount, 0, QUEST_FULL_ENTRY_MAX);
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
    EnrichWithGuide(out, quest.ID);
    return out;
}

QuestEntrySnapshot MakeClassicChangeJobQuest(int id, int level, const char* title, const sQUEST_CHANGEJOB& quest, bool selected)
{
    QuestEntrySnapshot out;
    out.id = id;
    out.kind = -1;
    out.sectionId = ClassicQuestSectionId;
    out.title = title;
    out.name = out.title;
    out.description = "Estado atual da quest classica: " + std::to_string(quest.State) + ".";
    out.levelMin = level;
    out.active = false;
    out.complete = false;
    out.selected = selected;
    EnrichWithGuide(out, id);
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
    out.description = "Estado atual da quest classica: " + std::to_string(quest.State) + ".";
    out.levelMin = level;
    out.active = false;
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
    EnrichWithGuide(out, id);
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
    out.description = "Estado atual da quest classica: " + std::to_string(quest.State) + ".";
    out.levelMin = level;
    out.active = false;
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
    EnrichWithGuide(out, id);
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
    addChangeJob(11003, 40, "Mudanca de classe - etapa 2 (consulte Codex por classe)", sinQuest_ChangeJob3);
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

const char* AmandaStageName(int kind)
{
    static const char* names[] = {
        "O Vigia Exausto",
        "Marcas do Medo",
        "Entrega de Doces",
        "Colina dos Duendes",
        "Ameaca Imortal",
        "Desafio Ciclope",
        "O Martelo Perdido",
    };
    return (kind >= Quest_A && kind <= Quest_G) ? names[kind] : "Quest da Instrutora Amanda";
}

QuestEntrySnapshot MakeAmandaQuest(
    int kind,
    const sQUEST_ELEMENTARY& quest,
    int sectionId,
    int selectedAmandaQuestId,
    int trackedSectionId,
    int trackedQuestId)
{
    QuestEntrySnapshot out;
    out.id = HAQUEST_CODE_ELEMENTARY_A + kind;
    out.kind = kind;
    out.sectionId = sectionId;
    out.title = AmandaStageName(kind);
    out.name = out.title;
    out.description = "Quest da Instrutora Amanda. O progresso e controlado pelo fluxo legacy da quest e projetado na UI V2.";
    out.levelMin = 0;
    out.levelMax = 39;
    out.active = quest.CODE != 0;
    out.complete = quest.State == 100;
    out.selected = selectedAmandaQuestId == out.id;
    out.tracked = trackedSectionId == AmandaQuestSectionId && sectionId == AmandaQuestSectionId && trackedQuestId == out.id;

    QuestObjectiveSnapshot objective;
    if (quest.Monster[1] > 0)
    {
        objective.name = "Monstros derrotados";
        objective.current = std::max(0, static_cast<int>(quest.Monster[0]));
        objective.required = std::max(1, static_cast<int>(quest.Monster[1]));
        objective.complete = objective.current >= objective.required || out.complete;
    }
    else
    {
        objective.name = "Progresso da etapa";
        objective.current = out.complete ? 1 : 0;
        objective.required = 1;
        objective.complete = out.complete;
    }
    out.objectives.push_back(std::move(objective));

    std::vector<QuestGuideEntry> guides;
    auto& source = summit::content::gameplay_reference::GameplayReferenceRuntime();
    if (source.IsValid() && source.Quests(guides))
    {
        for (std::size_t i = 0; i < guides.size(); ++i)
        {
            if (guides[i].guideId != "instrutora-amanda")
                continue;
            CopyGuide(out, guides[i]);
            if (kind >= 0 && static_cast<std::size_t>(kind) < guides[i].stages.size())
            {
                out.codex.stages.clear();
                out.codex.stages.push_back(guides[i].stages[kind]);
            }
            break;
        }
    }
    return out;
}

void AddAmandaQuests(
    QuestJournalSnapshot& out,
    int selectedAmandaQuestId,
    int trackedSectionId,
    int trackedQuestId)
{
    QuestSectionSnapshot section;
    section.id = AmandaQuestSectionId;
    section.title = "Instrutora Amanda";
    section.progressGroup = true;

    QuestSectionSnapshot finalized;
    finalized.id = AmandaFinalizedSectionId;
    finalized.title = "Finalizadas - Instrutora Amanda";
    finalized.finalizedGroup = true;

    for (int kind = Quest_A; kind <= Quest_G; ++kind)
    {
        const sQUEST_ELEMENTARY& quest = chaQuest.sHaQuestElementary[kind];
        if (!quest.CODE && !quest.BackUpCode && quest.State == 0)
            continue;

        // The legacy Amanda flow retires CODE into BackUpCode after reward/turn-in.
        // Keep a completed-but-not-yet-turned-in quest in the progress group so the
        // NPC remains the authority for reward delivery; only retired quests belong
        // to the finalized history.
        const bool retired = quest.State == 100 && quest.CODE == 0 && quest.BackUpCode != 0;
        QuestSectionSnapshot& targetSection = retired ? finalized : section;
        targetSection.quests.push_back(MakeAmandaQuest(
            kind, quest, targetSection.id, selectedAmandaQuestId, trackedSectionId, trackedQuestId));
    }

    if (selectedAmandaQuestId)
    {
        for (const auto& quest : section.quests)
            if (quest.id == selectedAmandaQuestId)
            {
                out.selectedSectionId = AmandaQuestSectionId;
                out.selectedQuestId = selectedAmandaQuestId;
                break;
            }
        if (!out.selectedQuestId)
            for (const auto& quest : finalized.quests)
                if (quest.id == selectedAmandaQuestId)
                {
                    out.selectedSectionId = AmandaFinalizedSectionId;
                    out.selectedQuestId = selectedAmandaQuestId;
                    break;
                }
    }

    if (!section.quests.empty())
        out.sections.push_back(std::move(section));
    if (!finalized.quests.empty())
        out.sections.push_back(std::move(finalized));
}

QuestEntrySnapshot MakeCodexQuest(const QuestGuideEntry& guide, bool selected)
{
    QuestEntrySnapshot out;
    out.id = CodexQuestId(guide.guideId);
    out.kind = -2;
    out.sectionId = CodexQuestSectionId;
    out.title = guide.name;
    out.name = guide.name;
    out.description = guide.supported
        ? "Entrada do Codex com binding runtime verificado no Summit."
        : "Referencia documental do Codex; binding runtime ainda nao foi comprovado.";
    out.levelMin = guide.levelMin > 0 ? guide.levelMin : 0;
    out.levelMax = guide.levelMax > 0 ? guide.levelMax : 0;
    out.active = false;
    out.complete = false;
    out.selected = selected;
    CopyGuide(out, guide);
    return out;
}

bool ContainsGuide(const std::vector<QuestGuideEntry>& quests, const std::string& guideId)
{
    for (std::size_t i = 0; i < quests.size(); ++i)
        if (quests[i].guideId == guideId)
            return true;
    return false;
}

void AddCodexQuests(QuestJournalSnapshot& out, int selectedCodexQuestId)
{
    auto& source = summit::content::gameplay_reference::GameplayReferenceRuntime();
    if (!source.IsValid())
        return;

    std::vector<QuestGuideEntry> guides;
    // The Codex is a read-only reference surface, so it can expose the full
    // canonical guide pack. Runtime authority remains separate: CopyGuide
    // carries guide.supported into runtimeVerified and action paths still use
    // live quest/runtime bindings only.
    if (!source.Quests(guides))
        return;

    // Generic QuestDB rows may represent daily/weekly entries which are not
    // statically bound. If the live title matches the supplied guide, include
    // that guide in the Codex only because the server proved it exists.
    for (std::size_t s = 0; s < out.sections.size(); ++s)
    {
        if (out.sections[s].codexGroup)
            continue;
        for (std::size_t q = 0; q < out.sections[s].quests.size(); ++q)
        {
            const QuestEntrySnapshot& runtimeQuest = out.sections[s].quests[q];
            if (!runtimeQuest.codex.available || ContainsGuide(guides, runtimeQuest.codex.guideId))
                continue;
            QuestGuideEntry guide;
            if (source.FindByName(runtimeQuest.codex.guideId.c_str(), guide))
                guides.push_back(guide);
        }
    }

    std::sort(guides.begin(), guides.end(), [](const QuestGuideEntry& a, const QuestGuideEntry& b)
    {
        const int al = a.levelMin < 0 ? 100000 : a.levelMin;
        const int bl = b.levelMin < 0 ? 100000 : b.levelMin;
        if (al != bl)
            return al < bl;
        if (a.name != b.name)
            return a.name < b.name;
        return a.guideId < b.guideId;
    });

    QuestSectionSnapshot section;
    section.id = CodexQuestSectionId;
    section.title = "Codex PT-BR";
    section.codexGroup = true;
    for (std::size_t i = 0; i < guides.size(); ++i)
        section.quests.push_back(MakeCodexQuest(guides[i], CodexQuestId(guides[i].guideId) == selectedCodexQuestId));
    if (section.quests.empty())
        return;
    if (selectedCodexQuestId)
    {
        out.selectedSectionId = CodexQuestSectionId;
        out.selectedQuestId = selectedCodexQuestId;
    }
    out.sections.push_back(std::move(section));
}
}

QuestJournalSnapshot LegacyQuestGateway::GetSnapshot() const
{
    QuestJournalSnapshot out;
    out.valid = !cQuestWindow.vTabs.empty();
    out.trackedSectionId = trackedSectionId_;
    out.trackedQuestId = trackedQuestId_;

    if (cQuestWindow.pSelectedSpoiler && cQuestWindow.pSelectedSpoiler->pSelectedQuest && !selectedClassicQuestId_ && !selectedAmandaQuestId_ && !selectedCodexQuestId_)
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
    AddAmandaQuests(out, selectedAmandaQuestId_, trackedSectionId_, trackedQuestId_);
    AddCodexQuests(out, selectedCodexQuestId_);
    out.valid = out.valid || !out.sections.empty();
    return out;
}

QuestEntrySnapshot LegacyQuestGateway::GetTrackedQuest() const
{
    if (trackedSectionId_ == AmandaQuestSectionId)
    {
        QuestJournalSnapshot snapshot;
        AddAmandaQuests(snapshot, selectedAmandaQuestId_, trackedSectionId_, trackedQuestId_);
        for (const auto& section : snapshot.sections)
            for (const auto& quest : section.quests)
                if (quest.id == trackedQuestId_ && quest.active)
                    return quest;
        return {};
    }

    const QuestRef ref = FindQuest(trackedSectionId_, trackedQuestId_);
    if (!ref.section || !ref.quest)
        return {};
    return MakeQuestSnapshot(*ref.section, *ref.quest, trackedSectionId_, trackedQuestId_);
}

bool LegacyQuestGateway::SelectQuest(int sectionId, int questId)
{
    if (sectionId == AmandaQuestSectionId || sectionId == AmandaFinalizedSectionId)
    {
        QuestJournalSnapshot snapshot;
        AddAmandaQuests(snapshot, questId, trackedSectionId_, trackedQuestId_);
        for (const auto& section : snapshot.sections)
            for (const auto& quest : section.quests)
                if (quest.id == questId)
                {
                    selectedAmandaQuestId_ = questId;
                    selectedClassicQuestId_ = 0;
                    selectedCodexQuestId_ = 0;
                    return true;
                }
        return false;
    }
    selectedAmandaQuestId_ = 0;
    if (sectionId == CodexQuestSectionId)
    {
        QuestJournalSnapshot snapshot;
        AddCodexQuests(snapshot, questId);
        for (const auto& section : snapshot.sections)
            for (const auto& quest : section.quests)
                if (quest.id == questId)
                {
                    selectedCodexQuestId_ = questId;
                    selectedClassicQuestId_ = 0;
                    selectedAmandaQuestId_ = 0;
                    return true;
                }
        return false;
    }
    selectedCodexQuestId_ = 0;

    if (sectionId == ClassicQuestSectionId)
    {
        QuestJournalSnapshot snapshot;
        AddClassicLevelQuests(snapshot, questId);
        for (const auto& section : snapshot.sections)
            for (const auto& quest : section.quests)
                if (quest.id == questId)
                {
                    selectedClassicQuestId_ = questId;
                    selectedAmandaQuestId_ = 0;
                    return true;
                }
        return false;
    }
    selectedClassicQuestId_ = 0;
    const QuestRef ref = FindQuest(sectionId, questId);
    if (!ref.section || !ref.quest)
        return false;

    cQuestWindow.pSelectedSpoiler = ref.section;
    return ref.section->SetQuest(ref.quest);
}

bool LegacyQuestGateway::RequestAcceptSelected()
{
    if (selectedCodexQuestId_ || selectedClassicQuestId_ || selectedAmandaQuestId_)
        return false;
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
    if (selectedCodexQuestId_ || selectedClassicQuestId_ || selectedAmandaQuestId_)
        return false;
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
    if (selectedCodexQuestId_ || selectedClassicQuestId_ || selectedAmandaQuestId_)
        return false;
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
    if (selectedAmandaQuestId_)
    {
        QuestJournalSnapshot snapshot;
        AddAmandaQuests(snapshot, selectedAmandaQuestId_, trackedSectionId_, trackedQuestId_);
        for (const auto& section : snapshot.sections)
            for (const auto& quest : section.quests)
                if (quest.id == selectedAmandaQuestId_ && quest.active)
                {
                    trackedSectionId_ = AmandaQuestSectionId;
                    trackedQuestId_ = selectedAmandaQuestId_;
                    return true;
                }
        return false;
    }

    if (selectedCodexQuestId_ || selectedClassicQuestId_)
        return false;
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
    selectedAmandaQuestId_ = 0;
    selectedCodexQuestId_ = 0;
}
}
