#pragma once

#include "systems/ui/v2/domain/ICompanionGateway.h"
#include "gameplay/companions/CompanionProtocol.h"
#include "gameplay/companions/v2/CompanionTradeDeliveryWire.h"
#include <mutex>
#include <vector>

namespace ui::v2
{
class LegacyCompanionGateway final : public ICompanionGateway
{
public:
    const CompanionUiSnapshot& Snapshot() const override { return snapshot_; }
    const CompanionRuntimeSnapshot& RuntimeSnapshot() const override { return runtimeSnapshot_; }
    const CompanionInventorySnapshot& InventorySnapshot() const override { return inventorySnapshot_; }
    const CompanionTradePreviewPage& TradePreview(int side) const override
    { return tradePages_[side == 1 ? 1 : 0]; }
    const CompanionTradeAttemptResult& TradeAttemptResult() const override { return tradeAttempt_; }
    bool RequestTradeAttempt(const std::string& characterName, int side, int recordIndex,
        std::uint32_t itemCode, std::uint32_t itemKey, std::uint32_t itemChecksum,
        std::uint64_t ownerRevision, std::uint64_t companionRevision) override;
    void ApplyTradeResult(const gameplay::companions::protocol::CompanionTradeResultPacket& packet);
    void ApplyCommittedDeliveryChunk(const gameplay::companions::v2::TradeDeliveryChunk& packet);
    void PumpTradeExecution() override;
    void RequestRefresh() override;
    void Execute(CompanionUiAction action, const std::string& characterName = {}, int value = 0) override;

    void ApplyState(const gameplay::companions::protocol::CompanionUIStatePacket& packet);
    void ApplyRuntimeState(const gameplay::companions::protocol::CompanionRuntimeStatePacket& packet);
    void ApplyInventoryPage(const gameplay::companions::protocol::CompanionInventoryPagePacket& packet);
    void ApplyTradePage(const gameplay::companions::protocol::CompanionTradePagePacket& packet);
    void Reset();

private:
    CompanionUiSnapshot snapshot_{};
    CompanionRuntimeSnapshot runtimeSnapshot_{};
    CompanionInventorySnapshot inventorySnapshot_{};
    CompanionTradePreviewPage tradePages_[2]{};
    CompanionTradeAttemptResult tradeAttempt_{};
    std::uint64_t tradeRequestId_ = 0;
    bool tradeSaveScheduled_ = false;
    bool tradeSavingBeforeConfirm_ = false;
    bool tradeSavingAfterApply_ = false;
    bool tradeLiveAwaitingServer350Q_ = false;
    bool tradeLivePostSaveScheduled350Q_ = false;
    bool tradeCommitted_ = false;
    // Retry transient live image/apply-ACK/postsave failures on the global UI
    // pump; never unfreeze before an authenticated positive post-save receipt.
    bool tradeLiveApplyAckPending350X_ = false;
    unsigned long tradeLiveApplyLastAttempt350X_ = 0;
    unsigned long tradeLivePostSaveAttempt350X_ = 0;
    unsigned long tradeLiveAckRetryTick350X_ = 0;
    // Client handshake deadline is a DISPLAY / PRE-CONFIRM guard only.
    // Never assume a sent CONFIRM has not committed just because it timed out.
    unsigned long tradeHandshakeTick_ = 0;
    bool tradeConfirmSent_ = false;
    bool tradeWaitReported_ = false;
    gameplay::companions::protocol::CompanionTradeConfirmPacket pendingTradeConfirm_{};
    gameplay::companions::v2::TradeSha256Digest beforeInventoryDigest_{};
    gameplay::companions::v2::TradeSha256Digest deliveredBeforeDigest_{};
    std::mutex tradeDeliveryMutex_;
    gameplay::companions::v2::TradeDeliveryReceiver tradeReceiver_;
    std::vector<std::uint8_t> pendingTradePlayerImage_;
    std::uint64_t pendingTradeTxnId_ = 0;
};

LegacyCompanionGateway& GetLegacyCompanionGateway();
void ApplyLegacyCompanionStatePacket(const gameplay::companions::protocol::CompanionUIStatePacket& packet);
void ApplyLegacyCompanionRuntimePacket(const gameplay::companions::protocol::CompanionRuntimeStatePacket& packet);
void ApplyLegacyCompanionInventoryPacket(const gameplay::companions::protocol::CompanionInventoryPagePacket& packet);
void ApplyLegacyCompanionTradePagePacket(const gameplay::companions::protocol::CompanionTradePagePacket& packet);
void ApplyLegacyCompanionTradeResultPacket(const gameplay::companions::protocol::CompanionTradeResultPacket& packet);
}
