#include "systems/social/party/v2/legacy/LegacyPartyCommandsAdapter.h"

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

namespace gameplay
{
namespace social
{
namespace party
{
namespace legacy
{
LegacyPartyCommandsAdapter::LegacyPartyCommandsAdapter(
    const player::legacy::LegacyOnlinePlayerRegistry& players)
    : players_(players)
{
}

bool LegacyPartyCommandsAdapter::Join(PlayerId requesterId, PlayerId targetId) const
{
    rsPLAYINFO* requester = players_.Resolve(requesterId);
    rsPLAYINFO* target = players_.Resolve(targetId);
    if(!requester || !target)
        return false;

    if(target->lpPartyMaster)
    {
        if(requester->lpPartyMaster)
            return false;
        return requester->JoinParty(target->lpPartyMaster) == TRUE;
    }

    if(requester->lpPartyMaster)
        return target->JoinParty(requester->lpPartyMaster) == TRUE;

    return target->CreateParty(requester) == TRUE;
}


bool LegacyPartyCommandsAdapter::TransferLeadership(PlayerId requesterId, PlayerId targetId) const
{
    rsPLAYINFO* requester = players_.Resolve(requesterId);
    rsPLAYINFO* target = players_.Resolve(targetId);
    if(!requester || !target || !requester->dwPartyInfo || !requester->lpPartyMaster)
        return false;
    return requester->LeavePartyMaster(target) == TRUE;
}

bool LegacyPartyCommandsAdapter::Release(PlayerId requesterId) const
{
    rsPLAYINFO* requester = players_.Resolve(requesterId);
    if(!requester || !requester->dwPartyInfo || !requester->lpPartyMaster)
        return false;
    return requester->ReleaseParty() == TRUE;
}

bool LegacyPartyCommandsAdapter::Kick(PlayerId requesterId, PlayerId targetId) const
{
    rsPLAYINFO* requester = players_.Resolve(requesterId);
    rsPLAYINFO* target = players_.Resolve(targetId);
    if(!requester || !target || !requester->dwPartyInfo || !requester->lpPartyMaster)
        return false;
    return target->SecedeParty() == TRUE;
}

bool LegacyPartyCommandsAdapter::Leave(PlayerId requesterId) const
{
    rsPLAYINFO* requester = players_.Resolve(requesterId);
    if(!requester || !requester->dwPartyInfo || !requester->lpPartyMaster)
        return false;
    return requester->SecedeParty() == TRUE;
}

bool LegacyPartyCommandsAdapter::ClearMembership(PlayerId memberId) const
{
    rsPLAYINFO* member = players_.Resolve(memberId);
    if(!member)
        return false;
    member->dwPartyInfo = PARTY_STATE_NONE;
    member->lpPartyMaster = 0;
    return true;
}

bool LegacyPartyCommandsAdapter::SynchronizeMembership(
    PlayerId receiverId, const PartyMembershipSync& sync) const
{
    rsPLAYINFO* receiver = players_.Resolve(receiverId);
    if(!receiver || sync.members.size() >= static_cast<std::size_t>(PARTY_PLAYER_MAX))
        return false;

    int levelAverage = 0;
    if(receiver->dwPartyInfo == PARTY_STATE_MASTER)
    {
        DWORD incomingCode = 0;
        DWORD currentCode = 0;
        for(std::vector<PlayerId>::const_iterator it = sync.members.begin(); it != sync.members.end(); ++it)
            incomingCode += static_cast<DWORD>(it->value);

        int currentCount = 0;
        for(int index = 0; index < PARTY_PLAYER_MAX; ++index)
        {
            if(receiver->lpPartyPlayers[index])
            {
                currentCode += receiver->lpPartyPlayers[index]->dwObjectSerial;
                ++currentCount;
            }
        }

        if(currentCount == static_cast<int>(sync.members.size()) && incomingCode == currentCode)
            return true;

        for(int index = 0; index < PARTY_PLAYER_MAX; ++index)
        {
            if(receiver->lpPartyPlayers[index] && receiver->lpPartyPlayers[index]->lpsmSock)
            {
                receiver->lpPartyPlayers[index]->dwPartyInfo = PARTY_STATE_NONE;
                receiver->lpPartyPlayers[index]->lpPartyMaster = 0;
            }
        }
    }

    ZeroMemory(receiver->lpPartyPlayers, sizeof(rsPLAYINFO*) * PARTY_PLAYER_MAX);

    for(std::size_t index = 0; index < sync.members.size(); ++index)
    {
        const PlayerId memberId = sync.members[index];
        if(memberId.value == receiver->dwObjectSerial)
        {
            receiver->dwPartyInfo = PARTY_STATE_MASTER;
            receiver->lpPartyMaster = receiver;
            receiver->lpPartyPlayers[index] = receiver;
            levelAverage += receiver->smCharInfo.Level;
            continue;
        }

        rsPLAYINFO* member = players_.Resolve(memberId);
        if(member)
        {
            receiver->lpPartyPlayers[index] = member;
            member->dwPartyInfo = PARTY_STATE_MEMBER;
            member->lpPartyMaster = receiver;
            levelAverage += member->smCharInfo.Level;
        }
    }

    receiver->PartyUserCount = static_cast<int>(sync.members.size());
    if(receiver->PartyUserCount)
        receiver->PartyLevelAverage = levelAverage / receiver->PartyUserCount;
    return true;
}

const LegacyPartyCommandsAdapter& ServerLegacyPartyCommandsAdapter()
{
    static LegacyPartyCommandsAdapter adapter(
        player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
}
