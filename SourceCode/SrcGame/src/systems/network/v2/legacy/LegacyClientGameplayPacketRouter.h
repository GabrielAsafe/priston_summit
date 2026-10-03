#pragma once

#include <cstddef>
#include <cstdint>

#include "gameplay/companions/CompanionProtocol.h"
#include "gameplay/covenants/v2/CovenantUIProtocol.h"
#include "systems/ui/v2/legacy/LegacyCompanionGateway.h"
#include "systems/ui/v2/legacy/LegacyCovenantGateway.h"

namespace gameplay
{
namespace companions
{
namespace client_visual
{
void ApplyPacket(const protocol::CompanionVisualStatePacket& packet);
}
}

namespace network
{
namespace legacy
{
// AMBER receive-side adapter. Returns true when the opcode belongs to a V2
// feature path, including malformed packets, so frozen RED dispatch cannot
// accidentally re-interpret a modern packet as a legacy opcode.
inline bool RouteClientGameplayPacket(std::uint32_t opcode, const void* data, std::size_t size)
{
    using namespace gameplay;

    switch (opcode)
    {
    case static_cast<std::uint32_t>(covenants::protocol::OPCODE_COVENANT_UI_STATE):
    {
        if (data && size == sizeof(covenants::protocol::CovenantUIStatePacket))
        {
            const covenants::protocol::CovenantUIStatePacket* packet =
                static_cast<const covenants::protocol::CovenantUIStatePacket*>(data);
            if (packet->size == static_cast<std::int32_t>(sizeof(*packet)) &&
                packet->version == covenants::protocol::COVENANT_UI_PROTOCOL_VERSION)
                ui::v2::ApplyLegacyCovenantStatePacket(*packet);
        }
        return true;
    }
    case static_cast<std::uint32_t>(companions::protocol::OPCODE_COMPANION_UI_STATE):
    {
        if (data && size == sizeof(companions::protocol::CompanionUIStatePacket))
        {
            const companions::protocol::CompanionUIStatePacket* packet =
                static_cast<const companions::protocol::CompanionUIStatePacket*>(data);
            if (packet->size == static_cast<std::int32_t>(sizeof(*packet)) &&
                packet->version == companions::protocol::COMPANION_UI_PROTOCOL_VERSION)
                ui::v2::ApplyLegacyCompanionStatePacket(*packet);
        }
        return true;
    }
    case static_cast<std::uint32_t>(companions::protocol::OPCODE_COMPANION_RUNTIME_STATE):
    {
        if (data && size == sizeof(companions::protocol::CompanionRuntimeStatePacket))
        {
            const companions::protocol::CompanionRuntimeStatePacket* packet =
                static_cast<const companions::protocol::CompanionRuntimeStatePacket*>(data);
            if (packet->size == static_cast<std::int32_t>(sizeof(*packet)) &&
                packet->version == companions::protocol::COMPANION_RUNTIME_PROTOCOL_VERSION)
                ui::v2::ApplyLegacyCompanionRuntimePacket(*packet);
        }
        return true;
    }
    case static_cast<std::uint32_t>(companions::protocol::OPCODE_COMPANION_VISUAL_STATE):
    {
        if (data && size == sizeof(companions::protocol::CompanionVisualStatePacket))
        {
            const companions::protocol::CompanionVisualStatePacket* packet =
                static_cast<const companions::protocol::CompanionVisualStatePacket*>(data);
            if (packet->size == static_cast<std::int32_t>(sizeof(*packet)) &&
                packet->version == companions::protocol::COMPANION_VISUAL_PROTOCOL_VERSION &&
                packet->entryCount >= 0 &&
                packet->entryCount <= companions::protocol::COMPANION_VISUAL_MAX_ACTIVE)
                companions::client_visual::ApplyPacket(*packet);
        }
        return true;
    }
    default:
        return false;
    }
}
}
}
}
