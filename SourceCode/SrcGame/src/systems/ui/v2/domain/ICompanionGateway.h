#pragma once

#include <string>
#include "systems/ui/v2/domain/CompanionUIModels.h"

namespace ui::v2
{
class ICompanionGateway
{
public:
    virtual ~ICompanionGateway() = default;
    virtual const CompanionUiSnapshot& Snapshot() const = 0;
    virtual const CompanionRuntimeSnapshot& RuntimeSnapshot() const = 0;
    // Default preserves existing gateway fakes, without fabricating inventory.
    virtual const CompanionInventorySnapshot& InventorySnapshot() const
    {
        static const CompanionInventorySnapshot absent{};
        return absent;
    }
    virtual const CompanionTradePreviewPage& TradePreview(int side) const
    {
        static const CompanionTradePreviewPage empty{};
        (void)side;
        return empty;
    }
    virtual const CompanionTradeAttemptResult& TradeAttemptResult() const
    { static const CompanionTradeAttemptResult absent{}; return absent; }
    // A request, not a client-side item mutation. No success without server receipt.
    virtual bool RequestTradeAttempt(const std::string& characterName, int side,
        int recordIndex, std::uint32_t itemCode, std::uint32_t itemKey,
        std::uint32_t itemChecksum, std::uint64_t ownerRevision,
        std::uint64_t companionRevision)
    { (void)characterName; (void)side; (void)recordIndex; (void)itemCode;
      (void)itemKey; (void)itemChecksum; (void)ownerRevision; (void)companionRevision; return false; }
    // Run pending trade IO on the game thread; test gateways default to no-op.
    virtual void PumpTradeExecution() {}
    virtual void RequestRefresh() = 0;
    virtual void Execute(CompanionUiAction action, const std::string& characterName = {}, int value = 0) = 0;
};
}
