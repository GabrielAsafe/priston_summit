#pragma once

#include <cstdint>
#include <vector>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace commerce
{
enum class CraftingCode
{
    Processed,
    InvalidPlayer,
    InvalidPayload,
    InventoryReservationRejected,
    RollbackFailed,
    Rejected
};

struct CraftRequest
{
    PlayerId player;
    std::vector<std::uint8_t> requestBytes;

    CraftRequest() : player(), requestBytes() {}
};

struct CraftResult
{
    CraftingCode code;
    PlayerId player;

    CraftResult() : code(CraftingCode::InvalidPlayer), player() {}
    bool Processed() const { return code == CraftingCode::Processed; }
};


enum class CraftingAuxiliaryOperation
{
    MakeLinkCore,
    UseLinkCore,
    Wing,
    SkinChange,
    Smelting,
    Manufacture
};

struct CraftingAuxiliaryRequest
{
    PlayerId player;
    CraftingAuxiliaryOperation operation;
    std::vector<std::uint8_t> requestBytes;

    CraftingAuxiliaryRequest()
        : player(), operation(CraftingAuxiliaryOperation::MakeLinkCore), requestBytes() {}
};

struct CraftingAuxiliaryResult
{
    CraftingCode code;
    PlayerId player;
    CraftingAuxiliaryOperation operation;

    CraftingAuxiliaryResult()
        : code(CraftingCode::InvalidPlayer), player(), operation(CraftingAuxiliaryOperation::MakeLinkCore) {}
    bool Processed() const { return code == CraftingCode::Processed; }
};

class ICraftingService
{
public:
    virtual ~ICraftingService() {}
    virtual CraftResult Craft(const CraftRequest& request) = 0;
    virtual CraftingAuxiliaryResult ProcessAuxiliary(const CraftingAuxiliaryRequest& request) = 0;
};
}
}
