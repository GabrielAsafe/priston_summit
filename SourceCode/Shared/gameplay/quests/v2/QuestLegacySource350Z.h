#pragma once
// Explicit, collision-free V2 IDs for quests whose *authoritative* runtime
// remains in the legacy character record. These are NOT QuestDB quest IDs.
// 350Z is a read-only source bridge: do not pay rewards or reset state here.
#include <cstddef>
#include <cstdint>
#include "gameplay/quests/v2/QuestCatalog.h"

namespace gameplay { namespace quests { namespace legacy {
enum { kLegacySinKind350Z = 90, kLegacyHaKind350Z = 91 };
static const std::uint32_t kLegacyNamespace350Z = 0xE3500000u;
struct LegacyQuestCode350Z {
    std::uint16_t code;
    int kind;
    const char* source;
    const char* symbol;
};
inline const LegacyQuestCode350Z* LegacyQuestCodes350Z(std::size_t& count) {
    static const LegacyQuestCode350Z codes[] = {
        {0x0001,90,"SinQuest","SIN_QUEST_CODE_CHANGEJOB"},
        {0x0002,90,"SinQuest","SIN_QUEST_CODE_CHANGEJOB2_NPC_M"},
        {0x0003,90,"SinQuest","SIN_QUEST_CODE_CHANGEJOB2_NPC_D"},
        {0x0004,90,"SinQuest","SIN_QUEST_CODE_CHANGEJOB3"},
        {0x0005,90,"SinQuest","SIN_QUEST_CODE_LEVEL30"},
        {0x0006,90,"SinQuest","SIN_QUEST_CODE_LEVEL55"},
        {0x0007,90,"SinQuest","SIN_QUEST_CODE_LEVEL55_2"},
        {0x0008,90,"SinQuest","SIN_QUEST_CODE_LEVEL70"},
        {0x0009,90,"SinQuest","SIN_QUEST_CODE_LEVEL80"},
        {0x000A,90,"SinQuest","SIN_QUEST_CODE_LEVEL85"},
        {0x000B,90,"SinQuest","SIN_QUEST_CODE_LEVEL90"},
        {0x000C,90,"SinQuest","SIN_QUEST_CODE_LEVEL80_2"},
        {0x000D,90,"SinQuest","SIN_QUEST_CODE_LEVEL90_2"},
        {0x0010,90,"SinQuest","SIN_QUEST_CODE_CHANGEJOB4"},
        {0x0020,91,"HaQuest","HAQUEST_CODE_ELEMENTARY_A"},
        {0x0021,91,"HaQuest","HAQUEST_CODE_ELEMENTARY_B"},
        {0x0022,91,"HaQuest","HAQUEST_CODE_ELEMENTARY_C"},
        {0x0023,91,"HaQuest","HAQUEST_CODE_ELEMENTARY_D"},
        {0x0024,91,"HaQuest","HAQUEST_CODE_ELEMENTARY_E"},
        {0x0025,91,"HaQuest","HAQUEST_CODE_ELEMENTARY_F"},
        {0x0026,91,"HaQuest","HAQUEST_CODE_ELEMENTARY_G"},
        {0x0028,91,"HaQuest","HAQUEST_CODE_FURYOFPHANTOM"}
    };
    count = sizeof(codes) / sizeof(codes[0]);
    return codes;
}
inline bool IsLegacyNamespacedQuest350Z(QuestId id) {
    return (id.value & 0xFFFF0000u) == kLegacyNamespace350Z;
}
inline QuestId LegacyVirtualQuestId350Z(std::uint16_t legacyCode) {
    return QuestId(kLegacyNamespace350Z | legacyCode);
}
inline const LegacyQuestCode350Z* FindLegacyQuest350Z(QuestId id) {
    if(!IsLegacyNamespacedQuest350Z(id)) return 0;
    const std::uint16_t code = static_cast<std::uint16_t>(id.value);
    std::size_t count = 0;
    const LegacyQuestCode350Z* all = LegacyQuestCodes350Z(count);
    for(std::size_t i = 0; i < count; ++i)
        if(all[i].code == code) return &all[i];
    return 0;
}
inline bool TryGetLegacyDefinition350Z(QuestId id, QuestDefinition& out) {
    const LegacyQuestCode350Z* source = FindLegacyQuest350Z(id);
    if(!source) return false;
    QuestDefinition definition;
    definition.id = id;
    definition.kind = source->kind;
    definition.type = 0; // Legacy stages live in character record, not QuestDB. 
    definition.partyFlag = 0;
    out = definition;
    return true;
}
}}}
