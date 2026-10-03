#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ui::v2
{
struct QuestObjectiveSnapshot
{
    std::string name;
    std::string location;
    int current = 0;
    int required = 0;
    bool complete = false;
};

struct QuestRewardSnapshot
{
    std::string name;
    int count = 0;
};

struct QuestCodexSnapshot
{
    bool available = false;
    bool runtimeVerified = false;
    std::string guideId;
    std::string type;
    std::string category;
    std::string source;
    std::string lore;
    bool repeatable = false;
    std::vector<std::string> prerequisites;
    std::vector<std::string> classes;
    std::vector<std::string> factions;
    std::vector<std::string> stages;
    std::vector<std::string> objectives;
    std::vector<std::string> npcs;
    std::vector<std::string> locations;
    std::vector<std::string> questItems;
    std::vector<std::string> rewards;
    std::vector<std::string> notes;
};

struct QuestEntrySnapshot
{
    int id = 0;
    int kind = 0;
    int sectionId = 0;
    std::string title;
    std::string name;
    std::string description;
    int levelMin = 0;
    int levelMax = 0;
    int questType = 0;
    bool party = false;
    bool active = false;
    bool complete = false;
    bool selected = false;
    bool tracked = false;
    std::vector<QuestObjectiveSnapshot> objectives;
    std::vector<QuestRewardSnapshot> rewards;
    int rewardGold = 0;
    std::int64_t rewardExp = 0;
    std::int64_t rewardClanExp = 0;
    int rewardCoin = 0;
    int rewardTCoin = 0;
    QuestCodexSnapshot codex;
};

struct QuestSectionSnapshot
{
    int id = 0;
    std::string title;
    bool progressGroup = false;
    bool finalizedGroup = false;
    bool codexGroup = false;
    std::vector<QuestEntrySnapshot> quests;
};

struct QuestJournalSnapshot
{
    bool valid = false;
    int selectedQuestId = 0;
    int selectedSectionId = 0;
    int trackedQuestId = 0;
    int trackedSectionId = 0;
    std::vector<QuestSectionSnapshot> sections;
};
}
