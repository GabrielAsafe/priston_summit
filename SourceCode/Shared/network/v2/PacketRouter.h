#pragma once

#include <cstdint>
#include <vector>

#include "gameplay/core/v2/GameplayIds.h"
#include "network/v2/PacketSender.h"

namespace gameplay
{
namespace network
{
enum class RouteStatus
{
    Handled = 0,
    UnknownOpcode,
    InvalidPacket,
    Rejected
};

class IProtocolHandler
{
public:
    virtual ~IProtocolHandler() {}
    virtual std::uint32_t Opcode() const = 0;
    virtual RouteStatus Handle(PlayerId player, const PacketView& packet) = 0;
};

class PacketRouter
{
public:
    bool Register(IProtocolHandler& handler)
    {
        for (std::vector<IProtocolHandler*>::const_iterator it = handlers_.begin(); it != handlers_.end(); ++it)
            if ((*it)->Opcode() == handler.Opcode())
                return false;
        handlers_.push_back(&handler);
        return true;
    }

    bool Handles(std::uint32_t opcode) const
    {
        for (std::vector<IProtocolHandler*>::const_iterator it = handlers_.begin(); it != handlers_.end(); ++it)
            if ((*it)->Opcode() == opcode)
                return true;
        return false;
    }

    RouteStatus Route(PlayerId player, const PacketView& packet) const
    {
        for (std::vector<IProtocolHandler*>::const_iterator it = handlers_.begin(); it != handlers_.end(); ++it)
        {
            if ((*it)->Opcode() != packet.opcode)
                continue;

            // Unknown/pre-auth legacy packets must be allowed to fall through
            // to the original login/session switch. Only packets owned by this
            // modern router require an authenticated PlayerId.
            if (!player.Valid() || !packet.Valid())
                return RouteStatus::InvalidPacket;

            return (*it)->Handle(player, packet);
        }
        return RouteStatus::UnknownOpcode;
    }

private:
    std::vector<IProtocolHandler*> handlers_;
};
}
}
