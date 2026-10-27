#include "systems/network/v2/legacy/LegacyGameplayPacketRouter.h"

#include "gameplay/companions/CompanionProtocol.h"
#include "gameplay/covenants/v2/CovenantUIProtocol.h"
#include "gameplay/quests/v2/QuestProtocol.h"
#include "systems/gameplay/events/hells_gate/cHellsGate.h"
#include "systems/gameplay/quests/v2/legacy/LegacyQuestCommands.h"
#include "systems/gameplay/quests/v2/legacy/LegacyQuestQueryIngressAdapter.h"
#include "systems/network/protocol/server/opcodes.h"
#include "systems/gameplay/companions/v2/legacy/LegacyCompanionUIIngressAdapter.h"
#include "systems/gameplay/covenants/v2/CovenantUIHandler.h"

namespace gameplay
{
namespace network
{
namespace legacy
{
namespace
{
class CovenantHandler final : public IProtocolHandler
{
public:
    std::uint32_t Opcode() const override { return covenants::protocol::OPCODE_COVENANT_UI_ACTION; }
    RouteStatus Handle(PlayerId player, const PacketView& packet) override
    {
        if (packet.size != sizeof(covenants::protocol::CovenantUIActionPacket))
            return RouteStatus::InvalidPacket;
        const covenants::protocol::CovenantUIActionPacket* action =
            static_cast<const covenants::protocol::CovenantUIActionPacket*>(packet.data);
        covenants::HandleCovenantUIAction(player, *action);
        return RouteStatus::Handled;
    }
};

class CompanionHandler final : public IProtocolHandler
{
public:
    std::uint32_t Opcode() const override { return companions::protocol::OPCODE_COMPANION_UI_ACTION; }
    RouteStatus Handle(PlayerId player, const PacketView& packet) override
    {
        if (packet.size != sizeof(companions::protocol::CompanionUIActionPacket))
            return RouteStatus::InvalidPacket;
        const companions::protocol::CompanionUIActionPacket* action =
            static_cast<const companions::protocol::CompanionUIActionPacket*>(packet.data);
        companions::legacy::ServerLegacyCompanionUIIngressAdapter().HandleAction(player, *action);
        return RouteStatus::Handled;
    }
};

class CompanionTradeConfirmHandler final : public IProtocolHandler
{
public:
    std::uint32_t Opcode() const override { return companions::protocol::OPCODE_COMPANION_TRADE_CONFIRM; }
    RouteStatus Handle(PlayerId player, const PacketView& packet) override
    {
        if(packet.size != sizeof(companions::protocol::CompanionTradeConfirmPacket))
            return RouteStatus::InvalidPacket;
        const auto* request = static_cast<const companions::protocol::CompanionTradeConfirmPacket*>(packet.data);
        companions::legacy::ServerLegacyCompanionUIIngressAdapter().HandleTradeConfirm(player,*request);
        return RouteStatus::Handled;
    }
};

class CompanionTradeApplyAckHandler350Q final : public IProtocolHandler
{
public:
    std::uint32_t Opcode() const override {
        return companions::protocol::OPCODE_COMPANION_TRADE_APPLY_ACK;
    }
    RouteStatus Handle(PlayerId player,const PacketView& packet) override {
        if(packet.size!=sizeof(companions::protocol::CompanionTradeApplyAckPacket))
            return RouteStatus::InvalidPacket;
        const auto* ack=static_cast<const companions::protocol::CompanionTradeApplyAckPacket*>(packet.data);
        companions::legacy::ServerLegacyCompanionUIIngressAdapter().HandleTradeApplyAck350Q(player,*ack);
        return RouteStatus::Handled;
    }
};

class HellsGateJoinHandler final : public IProtocolHandler
{
public:
    std::uint32_t Opcode() const override { return static_cast<std::uint32_t>(OpCode::OPCODE_OPEN_HG); }
    RouteStatus Handle(PlayerId player, const PacketView&) override
    {
        cHg.RequestJoin(player);
        return RouteStatus::Handled;
    }
};

class QuestStartHandler final : public IProtocolHandler
{
public:
    std::uint32_t Opcode() const override { return static_cast<std::uint32_t>(OpCode::OPCODE_START_QUEST); }
    RouteStatus Handle(PlayerId player, const PacketView& packet) override
    {
        if(packet.size != sizeof(quests::protocol::QuestCommandPacket))
            return RouteStatus::InvalidPacket;
        const quests::protocol::QuestCommandPacket* wire =
            static_cast<const quests::protocol::QuestCommandPacket*>(packet.data);
        quests::QuestStartCommand command;
        command.quest = QuestId(static_cast<std::uint32_t>(wire->WParam));
        command.kind = wire->LParam;
        command.type = wire->SParam;
        command.partyFlag = wire->EParam;
        return quests::legacy::ServerLegacyQuestCommands().Start(player, command)
            ? RouteStatus::Handled : RouteStatus::Rejected;
    }
};

class QuestCancelHandler final : public IProtocolHandler
{
public:
    std::uint32_t Opcode() const override { return static_cast<std::uint32_t>(OpCode::OPCODE_CANCEL_QUEST); }
    RouteStatus Handle(PlayerId player, const PacketView& packet) override
    {
        if(packet.size != sizeof(quests::protocol::QuestCommandPacket))
            return RouteStatus::InvalidPacket;
        const quests::protocol::QuestCommandPacket* wire =
            static_cast<const quests::protocol::QuestCommandPacket*>(packet.data);
        quests::QuestCommand command;
        command.quest = QuestId(static_cast<std::uint32_t>(wire->WParam));
        command.kind = wire->LParam;
        command.type = wire->SParam;
        return quests::legacy::ServerLegacyQuestCommands().Cancel(player, command)
            ? RouteStatus::Handled : RouteStatus::Rejected;
    }
};

class QuestRewardHandler final : public IProtocolHandler
{
public:
    std::uint32_t Opcode() const override { return static_cast<std::uint32_t>(OpCode::OPCODE_GET_REWARD); }
    RouteStatus Handle(PlayerId player, const PacketView& packet) override
    {
        if(packet.size != sizeof(quests::protocol::QuestCommandPacket))
            return RouteStatus::InvalidPacket;
        const quests::protocol::QuestCommandPacket* wire =
            static_cast<const quests::protocol::QuestCommandPacket*>(packet.data);
        quests::QuestCommand command;
        command.quest = QuestId(static_cast<std::uint32_t>(wire->WParam));
        command.kind = wire->LParam;
        command.type = wire->SParam;
        return quests::legacy::ServerLegacyQuestCommands().ClaimReward(player, command)
            ? RouteStatus::Handled : RouteStatus::Rejected;
    }
};

class QuestSpoilerHandler final : public IProtocolHandler
{
public:
    std::uint32_t Opcode() const override { return static_cast<std::uint32_t>(OpCode::OPCODE_SPOILER_INFO); }
    RouteStatus Handle(PlayerId player, const PacketView& packet) override
    {
        if(packet.size != sizeof(quests::protocol::QuestCommandPacket))
            return RouteStatus::InvalidPacket;
        const quests::protocol::QuestCommandPacket* wire =
            static_cast<const quests::protocol::QuestCommandPacket*>(packet.data);
        return quests::legacy::ServerLegacyQuestQueryIngressAdapter().SendSpoilerInfo(player, wire->WParam)
            ? RouteStatus::Handled : RouteStatus::Rejected;
    }
};

class QuestInfoHandler final : public IProtocolHandler
{
public:
    std::uint32_t Opcode() const override { return static_cast<std::uint32_t>(OpCode::OPCODE_QUEST_INFO); }
    RouteStatus Handle(PlayerId player, const PacketView& packet) override
    {
        if(packet.size != sizeof(quests::protocol::QuestCommandPacket))
            return RouteStatus::InvalidPacket;
        const quests::protocol::QuestCommandPacket* wire =
            static_cast<const quests::protocol::QuestCommandPacket*>(packet.data);
        return quests::legacy::ServerLegacyQuestQueryIngressAdapter().SendQuestInfo(
            player, wire->WParam, wire->LParam, wire->SParam)
            ? RouteStatus::Handled : RouteStatus::Rejected;
    }
};

PacketRouter& ServerRouter()
{
    static PacketRouter router;
    static CovenantHandler covenant;
    static CompanionHandler companion;
    static HellsGateJoinHandler hellsGateJoin;
    static QuestStartHandler questStart;
    static CompanionTradeConfirmHandler tradeConfirm;
    static CompanionTradeApplyAckHandler350Q liveAck;
    static QuestCancelHandler questCancel;
    static QuestRewardHandler questReward;
    static QuestSpoilerHandler questSpoiler;
    static QuestInfoHandler questInfo;
    static bool initialized = false;
    if (!initialized)
    {
        (void)router.Register(covenant);
        (void)router.Register(companion);
        (void)router.Register(tradeConfirm);
        (void)router.Register(liveAck);
        (void)router.Register(hellsGateJoin);
        (void)router.Register(questStart);
        (void)router.Register(questCancel);
        (void)router.Register(questReward);
        (void)router.Register(questSpoiler);
        (void)router.Register(questInfo);
        initialized = true;
    }
    return router;
}
}

RouteStatus RouteServerGameplayPacket(PlayerId player,
                                      std::uint32_t opcode,
                                      const void* data,
                                      std::size_t size)
{
    return ServerRouter().Route(player, PacketView(data, size, opcode));
}
}
}
}
