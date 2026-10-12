#pragma once

// Portable admission policy for Summit texture streaming.
// CPU preparation and completed GPU uploads have independent limits; the
// old global in-flight-only limit made GPU backlog block new critical jobs.
#include <cstdint>

namespace summit { namespace rendering { namespace assets { namespace v3 {

enum TextureQueueBlockReason
{
    TEXTURE_QUEUE_ACCEPT = 0,
    TEXTURE_QUEUE_CPU_FULL,
    TEXTURE_QUEUE_GPU_READY_FULL,
    TEXTURE_QUEUE_TOTAL_FULL
};

struct TextureQueuePressure
{
    std::uint32_t preparing;
    std::uint32_t ready;
    std::uint32_t outstanding;
};

inline TextureQueueBlockReason TextureQueueCanAdmit(
    const TextureQueuePressure &pressure, std::uint32_t normalLimit, int priority)
{
    // Priority 0 means HIGH; priority >= 2 is speculative LOW/prewarm.
    // Keep an emergency lane for HIGH without admitting unbounded work.
    if (normalLimit < 1u) normalLimit = 1u;
    if (normalLimit > 16u) normalLimit = 16u;
    const std::uint32_t reserve = priority <= 0 ? 2u : 0u;
    const std::uint32_t preparingLimit = normalLimit + reserve;
    const std::uint32_t readyLimit = normalLimit * 2u + reserve;
    const std::uint32_t totalLimit = normalLimit * 3u + reserve * 2u;
    if (pressure.preparing >= preparingLimit)
        return TEXTURE_QUEUE_CPU_FULL;
    if (pressure.ready >= readyLimit)
        return TEXTURE_QUEUE_GPU_READY_FULL;
    if (pressure.outstanding >= totalLimit)
        return TEXTURE_QUEUE_TOTAL_FULL;
    return TEXTURE_QUEUE_ACCEPT;
}

}}}} // namespace summit::rendering::assets::v3
