#pragma once

#include <windows.h>
#include "items.h"

#ifndef MAX_COINSHOP_ITEMS
#define MAX_COINSHOP_ITEMS 15
#endif

#ifndef MAX_QUEST_PACKET_INFO
#define MAX_QUEST_PACKET_INFO 58
#endif

struct smTRANS_SHOPDATA
{
    int size;
    int code;
    int Section;
    int Count;
    struct sCOMPRESSED_SHOPITEM { BYTE Data[512]; int Price; }; sCOMPRESSED_SHOPITEM sCompItem[MAX_COINSHOP_ITEMS];
};

struct smTRANS_BUYSHOP
{
    int size;
    int code;
    int Section;
    int ItemIndex;
    char szItemCode[32];
};

struct smQUEST_PACKET_INFO
{
    int QuestID;
    int QuestKind;
    char QuestName[128];
    bool QuestActive;
};

struct smTRANS_QUEST_INFO
{
    int size;
    int code;
    int SpoilerNum;
    int QuestCount;
    smQUEST_PACKET_INFO QuestInfo[MAX_QUEST_PACKET_INFO];
};

constexpr int QUEST_FULL_ENTRY_MAX = 10;

struct smTRANS_QUEST_FULL
{
    int size;
    int code;
    int QuestID;
    int QuestKind;
    char QuestName[128];
    int QuestType;
    int PartyFlag;
    char QuestTitle[128];
    char QuestDesc[1024];
    int LevelMin;
    int LevelMax;
    int MonsterCount;
    char MonsterNames[QUEST_FULL_ENTRY_MAX][64];
    int MonsterCounts[QUEST_FULL_ENTRY_MAX];
    int MonsterKilled[QUEST_FULL_ENTRY_MAX];
    int MonsterMaps[QUEST_FULL_ENTRY_MAX];
    int RewardCount;
    char RewardNames[QUEST_FULL_ENTRY_MAX][64];
    int RewardCounts[QUEST_FULL_ENTRY_MAX];
    int RewardGold;
    __int64 RewardExp;
    __int64 RewardClanExp;
    int RewardCoin;
    int RewardTCoin;
};


