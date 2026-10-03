#pragma once

#include <cstddef>
#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace commerce
{
struct PersonalShopSessionFingerprint
{
    std::uint32_t itemCode;
    std::uint32_t head;
    std::uint32_t checksum;
    std::int64_t goldPrice;

    PersonalShopSessionFingerprint(std::uint32_t itemCodeValue = 0u,
                                   std::uint32_t headValue = 0u,
                                   std::uint32_t checksumValue = 0u,
                                   std::int64_t goldPriceValue = 0)
        : itemCode(itemCodeValue), head(headValue), checksum(checksumValue), goldPrice(goldPriceValue) {}
};

inline bool operator==(const PersonalShopSessionFingerprint& lhs,
                       const PersonalShopSessionFingerprint& rhs)
{
    return lhs.itemCode == rhs.itemCode && lhs.head == rhs.head &&
           lhs.checksum == rhs.checksum && lhs.goldPrice == rhs.goldPrice;
}

enum class PersonalShopSessionRecordCode
{
    Recorded,
    Duplicate,
    InvalidRequest
};

struct PersonalShopSessionRecordResult
{
    PersonalShopSessionRecordCode code;
    PersonalShopSessionRecordResult() : code(PersonalShopSessionRecordCode::InvalidRequest) {}
    bool Accepted() const
    {
        return code == PersonalShopSessionRecordCode::Recorded ||
               code == PersonalShopSessionRecordCode::Duplicate;
    }
};

class IPersonalShopSessionService
{
public:
    virtual ~IPersonalShopSessionService() {}
    virtual bool CooldownActive(PlayerHandle buyer, std::uint32_t nowMs) const = 0;
    virtual PersonalShopSessionRecordResult RecordPrepared(
        PlayerHandle buyer,
        std::uint32_t nowMs,
        const PersonalShopSessionFingerprint& fingerprint) = 0;
    virtual std::size_t Clear(PlayerHandle buyer) = 0;
};
}
}
