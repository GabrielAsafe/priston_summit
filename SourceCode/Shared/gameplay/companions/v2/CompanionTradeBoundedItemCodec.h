#pragma once
// Strict decoder for the historical Priston zero-run/literal item codec.
// Unlike legacy DecodeCompress, the input AND output bounds are explicit.
// This is a decoder, NOT an authorization to transfer or persist an item.
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

namespace gameplay { namespace companions { namespace v2 {

enum class TradeItemDecodeStatus : std::uint8_t {
    Ok, InvalidArgument, InvalidFrame, InvalidRun, TruncatedLiteral,
    OverlongOutput, IncompleteOutput
};

struct CompanionTradeBoundedItemCodec final {
    static TradeItemDecodeStatus Decode(const std::uint8_t* source,
        std::size_t sourceLength, std::uint8_t* destination,
        std::size_t destinationLength)
    {
        if (!source || !destination || sourceLength <= 4u ||
            destinationLength == 0u ||
            sourceLength > static_cast<std::size_t>(INT32_MAX) ||
            destinationLength > static_cast<std::size_t>(INT32_MAX))
            return TradeItemDecodeStatus::InvalidArgument;
        // The legacy encoder stores the length as a little-endian signed int.
        const std::uint32_t framed =
            static_cast<std::uint32_t>(source[0]) |
            (static_cast<std::uint32_t>(source[1]) << 8) |
            (static_cast<std::uint32_t>(source[2]) << 16) |
            (static_cast<std::uint32_t>(source[3]) << 24);
        if (framed != sourceLength)
            return TradeItemDecodeStatus::InvalidFrame;
        std::size_t input = 4u, output = 0u;
        // Decode into a private local only; callers should never pass an
        // authoritative inventory slot as destination until validation passes.
        while (input < sourceLength) {
            const std::uint8_t command = source[input++];
            const std::size_t run = command & 0x7fu;
            if (!run) return TradeItemDecodeStatus::InvalidRun;
            if (run > destinationLength - output)
                return TradeItemDecodeStatus::OverlongOutput;
            if (command & 0x80u) {
                std::memset(destination + output, 0, run);
            } else {
                if (run > sourceLength - input)
                    return TradeItemDecodeStatus::TruncatedLiteral;
                std::memcpy(destination + output, source + input, run);
                input += run;
            }
            output += run;
        }
        return output == destinationLength ? TradeItemDecodeStatus::Ok :
            TradeItemDecodeStatus::IncompleteOutput;
    }
};

}}} // gameplay::companions::v2
