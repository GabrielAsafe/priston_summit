#pragma once
// Local integration between a VERIFIED durable-delivery gate and the staged
// two-bag legacy inventory. This is NOT a network handler or a trade commit.
// Host callbacks MUST ensure the actual client is frozen, verify that its
// live inventory still corresponds to the accepted before-image, and install
// both bag arrays as ONE non-failing operation followed by a fresh save.
#include "gameplay/companions/v2/CompanionTradeClientDeliveryGate.h"
#include "gameplay/companions/v2/CompanionTradeLegacyInventoryStage.h"

namespace gameplay { namespace companions { namespace v2 {

struct CompanionTradeClientApplyAdapter final {
    template<class LegacyItem, class DecodeSnapshot, class DecodeItem,
             class StillFrozenAndCurrent, class InstallBothBags>
    static TradeDeliveryResult ApplyCommitted(
        TradeClientDeliveryGate& gate,
        const BufferedTradeEnvelopeLayout& layout,
        const char* authenticatedCharacter,
        std::size_t slotsPerBag, int maxItems,
        DecodeSnapshot decodeSnapshot, DecodeItem decodeItem,
        StillFrozenAndCurrent liveCheck,
        InstallBothBags install)
    {
        return gate.Apply([&](const std::vector<std::uint8_t>& committed) {
            if (!liveCheck()) return false;
            CompanionTradeInventorySnapshot snapshot;
            if (BufferedTradePayloadPolicy::DecodeBounded(
                  committed.data(),committed.size(),layout,
                  authenticatedCharacter,maxItems,decodeSnapshot,snapshot) !=
                BufferedTradeCaptureCode::CandidateReadOnly ||
                !snapshot.TradeEligible()) return false;
            TradeClientStagedBags<LegacyItem> staged;
            if (CompanionTradeLegacyInventoryStage::Prepare(
                snapshot,slotsPerBag,decodeItem,staged) !=
                    TradeClientStageStatus::Ready) return false;
            // Check AGAIN immediately before publication. The caller must
            // hold the real client inventory freeze across this check and
            // installation. A bool callback is not proof of a mutex.
            if (!liveCheck()) return false;
            return install(staged);
        });
    }
};

}}} // gameplay::companions::v2
