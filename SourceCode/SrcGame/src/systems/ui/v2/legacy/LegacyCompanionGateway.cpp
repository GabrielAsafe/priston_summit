#include "systems/ui/v2/legacy/LegacyCompanionGateway.h"

#include <algorithm>
#include <cstring>

#include "systems/network/client/netplay.h"
#include "systems/network/transport/legacy/smwsock.h"

namespace ui::v2
{
namespace
{
using namespace gameplay::companions::protocol;

std::int32_t ToWireAction(CompanionUiAction action)
{
    switch (action)
    {
        case CompanionUiAction::Refresh: return COMPANION_UI_REFRESH;
        case CompanionUiAction::Select: return COMPANION_UI_SELECT;
        case CompanionUiAction::Summon: return COMPANION_UI_SUMMON;
        case CompanionUiAction::Dismiss: return COMPANION_UI_DISMISS;
        case CompanionUiAction::DismissAll: return COMPANION_UI_DISMISS_ALL;
        case CompanionUiAction::SetMode: return COMPANION_UI_SET_MODE;
        case CompanionUiAction::EquipRightNext: return COMPANION_UI_EQUIP_RIGHT_NEXT;
        case CompanionUiAction::EquipRightPrev: return COMPANION_UI_EQUIP_RIGHT_PREV;
        case CompanionUiAction::EquipLeftNext: return COMPANION_UI_EQUIP_LEFT_NEXT;
        case CompanionUiAction::EquipLeftPrev: return COMPANION_UI_EQUIP_LEFT_PREV;
        case CompanionUiAction::ResetEquipment: return COMPANION_UI_RESET_EQUIPMENT;
        case CompanionUiAction::ResyncData: return COMPANION_UI_RESYNC_DAT;
        case CompanionUiAction::SetTactic: return COMPANION_UI_SET_TACTIC;
        case CompanionUiAction::SetRetreatHp: return COMPANION_UI_SET_RETREAT_HP;
        case CompanionUiAction::SetPreferredRange: return COMPANION_UI_SET_PREFERRED_RANGE;
    }
    return COMPANION_UI_REFRESH;
}
}

LegacyCompanionGateway& GetLegacyCompanionGateway()
{
    static LegacyCompanionGateway gateway;
    return gateway;
}

void LegacyCompanionGateway::RequestRefresh()
{
    Execute(CompanionUiAction::Refresh);
}

void LegacyCompanionGateway::Execute(CompanionUiAction action, const std::string& characterName, int value)
{
    using namespace gameplay::companions::protocol;
    if (!smWsockServer)
        return;

    CompanionUIActionPacket packet{};
    packet.size = sizeof(packet);
    packet.code = OPCODE_COMPANION_UI_ACTION;
    packet.version = COMPANION_UI_PROTOCOL_VERSION;
    packet.action = ToWireAction(action);
    packet.value = value;
    if (!characterName.empty())
        std::strncpy(packet.characterName, characterName.c_str(), sizeof(packet.characterName) - 1);

    smWsockServer->Send(reinterpret_cast<char*>(&packet), packet.size, TRUE);
}

void LegacyCompanionGateway::ApplyState(const gameplay::companions::protocol::CompanionUIStatePacket& packet)
{
    using namespace gameplay::companions::protocol;
    if (packet.version != COMPANION_UI_PROTOCOL_VERSION)
        return;

    snapshot_ = CompanionUiSnapshot{};
    snapshot_.valid = true;
    snapshot_.mode = static_cast<CompanionUiMode>(std::clamp(packet.mode, 0, 2));
    snapshot_.activeCount = packet.activeCount;
    snapshot_.maxActive = packet.maxActive;
    snapshot_.lastResult = packet.lastResult;
    snapshot_.entryCount = std::clamp(packet.entryCount, 0, static_cast<int>(CompanionUiMaxCharacters));

    for (int i = 0; i < snapshot_.entryCount; ++i)
    {
        const auto& source = packet.entries[i];
        auto& target = snapshot_.entries[static_cast<std::size_t>(i)];
        target.name = source.characterName;
        target.level = source.level;
        target.jobCode = source.jobCode;
        target.active = source.active != 0;
        target.selected = source.selected != 0;
        target.currentCharacter = source.currentCharacter != 0;
        target.rightItemCode = source.rightItemCode;
        target.leftItemCode = source.leftItemCode;
        target.rightItemName = source.rightItemName;
        target.leftItemName = source.leftItemName;
        target.life = source.life;
        target.maxLife = source.maxLife;
        target.mana = source.mana;
        target.maxMana = source.maxMana;
        target.dead = source.dead != 0;
        target.targetName = source.targetName;
        target.mode = static_cast<CompanionUiMode>(std::clamp(source.mode, 0, 2));
        target.tactic = static_cast<CompanionUiTactic>(std::clamp(source.tactic, 0, 3));
        target.retreatHpPercent = std::clamp(source.retreatHpPercent, 0, 90);
        target.preferredRange = std::clamp(source.preferredRange, 0, 600);
    }
}

void LegacyCompanionGateway::ApplyRuntimeState(const gameplay::companions::protocol::CompanionRuntimeStatePacket& packet)
{
    using namespace gameplay::companions::protocol;
    if (packet.version != COMPANION_RUNTIME_PROTOCOL_VERSION)
        return;

    runtimeSnapshot_ = CompanionRuntimeSnapshot{};
    runtimeSnapshot_.valid = true;
    runtimeSnapshot_.entryCount = std::clamp(packet.entryCount, 0, COMPANION_RUNTIME_MAX_ACTIVE);
    for (int i = 0; i < runtimeSnapshot_.entryCount; ++i)
    {
        const auto& source = packet.entries[i];
        auto& target = runtimeSnapshot_.entries[static_cast<std::size_t>(i)];
        target.name = source.characterName;
        target.objectSerial = source.objectSerial;
        target.level = source.level;
        target.jobCode = source.jobCode;
        target.life = source.life;
        target.maxLife = source.maxLife;
        target.mana = source.mana;
        target.maxMana = source.maxMana;
        target.dead = source.dead != 0;
        target.mode = static_cast<CompanionUiMode>(std::clamp(source.mode, 0, 2));
        target.tactic = static_cast<CompanionUiTactic>(std::clamp(source.tactic, 0, 3));
        target.casting = source.casting != 0;
        target.targetName = source.targetName;
    }
}

void LegacyCompanionGateway::Reset()
{
    snapshot_ = CompanionUiSnapshot{};
    runtimeSnapshot_ = CompanionRuntimeSnapshot{};
}

void ApplyLegacyCompanionStatePacket(const gameplay::companions::protocol::CompanionUIStatePacket& packet)
{
    GetLegacyCompanionGateway().ApplyState(packet);
}

void ApplyLegacyCompanionRuntimePacket(const gameplay::companions::protocol::CompanionRuntimeStatePacket& packet)
{
    GetLegacyCompanionGateway().ApplyRuntimeState(packet);
}
}
