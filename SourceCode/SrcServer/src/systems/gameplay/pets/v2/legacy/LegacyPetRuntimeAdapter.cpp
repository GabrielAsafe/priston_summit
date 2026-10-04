#include "systems/gameplay/pets/v2/legacy/LegacyPetRuntimeAdapter.h"

#include "systems/network/protocol/server/opcodes.h"
#include "systems/network/v2/legacy/LegacyPacketSender.h"

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

int rsSendAGameServer(smWINSOCK* lpsmSock, void* data);
int rsSendAGameServer_PrimeItem3(smWINSOCK* lpsmSock, void* data);

namespace gameplay
{
namespace pets
{
namespace legacy
{
LegacyPetRuntimeAdapter::LegacyPetRuntimeAdapter(
    const player::legacy::LegacyOnlinePlayerRegistry& players,
    network::IPacketSender& packets)
    : players_(players),
      packets_(packets)
{
}

PetRuntimeSnapshot LegacyPetRuntimeAdapter::Snapshot(PlayerId playerId) const
{
    rsPLAYINFO* player = players_.Resolve(playerId);
    if(!player)
        return PetRuntimeSnapshot();

    return PetRuntimeSnapshot(
        true,
        static_cast<std::uint32_t>(player->Bl_RNo > 0 ? player->Bl_RNo : 0),
        static_cast<std::uint32_t>(player->dwTime_PrimeItem_PhenixPet),
        static_cast<std::uint32_t>(player->dwTime_PrimeItem_HelpPet));
}

PetSyncStatus LegacyPetRuntimeAdapter::ApplyAreaSync(PlayerId playerId, const PetAreaSync& sync)
{
    rsPLAYINFO* player = players_.Resolve(playerId);
    if(!player)
        return PetSyncStatus::InvalidPlayer;

    const std::uint32_t mask =
        static_cast<std::uint32_t>(player->dwObjectSerial) *
        (static_cast<std::uint32_t>(player->dwCode_Name) << 3) ^ sync.e;
    const std::uint32_t w = sync.w ^ mask;
    const std::uint32_t l = sync.l ^ mask;
    const std::uint32_t s = sync.s ^ mask;
    if(s != static_cast<std::uint32_t>(w + l + sync.opcode))
        return PetSyncStatus::InvalidChecksum;

    switch(sync.kind)
    {
    case PetRuntimeKind::PcBang:
        player->Bl_RNo = static_cast<int>(w);
        break;
    case PetRuntimeKind::Phoenix:
        player->dwTime_PrimeItem_PhenixPet = static_cast<DWORD>(w);
        break;
    case PetRuntimeKind::Help:
        player->dwTime_PrimeItem_HelpPet = static_cast<DWORD>(w);
        break;
    }
    return PetSyncStatus::Applied;
}

PetPublishStatus LegacyPetRuntimeAdapter::PublishAreaSync(PlayerId playerId, PetRuntimeKind kind)
{
    rsPLAYINFO* player = players_.Resolve(playerId);
    if(!player)
        return PetPublishStatus::InvalidPlayer;

    DWORD* nextSetTime = 0;
    DWORD value = 0;
    DWORD opcode = 0;
    bool enabled = false;

    switch(kind)
    {
    case PetRuntimeKind::PcBang:
        nextSetTime = &player->dwPCBang_Pet_NextSetTime;
        value = static_cast<DWORD>(player->Bl_RNo);
        opcode = OpCode::OPCODE_PCBANG_PET;
        enabled = rsHasPCBangGameplayEntitlement(player) != 0;
        break;
    case PetRuntimeKind::Phoenix:
        nextSetTime = &player->dwPCBang_Pet_NextSetTime;
        value = player->dwTime_PrimeItem_PhenixPet;
        opcode = OpCode::OPCODE_PHENIX_PET;
        enabled = value > 0;
        break;
    case PetRuntimeKind::Help:
        nextSetTime = &player->dwHelp_Pet_NextSetTime;
        value = player->dwTime_PrimeItem_HelpPet;
        opcode = OpCode::OPCODE_HELP_PET;
        enabled = value > 0;
        break;
    }

    if(!player->dwObjectSerial || !nextSetTime || *nextSetTime >= dwPlayServTime || !enabled)
        return PetPublishStatus::Skipped;

    smTRANS_COMMAND command;
    ZeroMemory(&command, sizeof(command));
    if(!*nextSetTime)
    {
        command.code = opcode;
        command.size = sizeof(command);
        (void)packets_.Send(
            playerId,
            network::PacketView(&command, static_cast<std::size_t>(command.size),
                                static_cast<std::uint32_t>(command.code)));
    }

    const DWORD codeKey = dwPlayServTime & 0x3FFFFFFF;
    *nextSetTime = dwPlayServTime + 1000 * 17;
    const DWORD maskKey =
        player->dwObjectSerial * (player->dwCode_Name << 3) ^ codeKey;

    command.code = opcode;
    command.size = sizeof(command);
    command.WParam = value ^ maskKey;
    command.LParam = *nextSetTime ^ maskKey;
    command.SParam = (value + *nextSetTime + command.code) ^ maskKey;
    command.EParam = codeKey;

    if(kind == PetRuntimeKind::Help)
        rsSendAGameServer_PrimeItem3(player->lpsmSock, &command);
    else
        rsSendAGameServer(player->lpsmSock, &command);

    return PetPublishStatus::Published;
}

LegacyPetRuntimeAdapter& ServerLegacyPetRuntimeAdapter()
{
    static LegacyPetRuntimeAdapter adapter(
        player::legacy::ServerLegacyOnlinePlayerRegistry(),
        network::legacy::ServerLegacyPacketSender());
    return adapter;
}
}
}
}
