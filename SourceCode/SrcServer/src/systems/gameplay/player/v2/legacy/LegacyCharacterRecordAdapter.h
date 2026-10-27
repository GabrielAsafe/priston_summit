#pragma once

#include "gameplay/player/v2/ICharacterRepository.h"
#include "gameplay/companions/v2/CompanionTradeInventorySnapshot.h"
#include "gameplay/companions/v2/CompanionTradeBufferedPayloadCandidate.h"
#include "gameplay/companions/v2/CompanionTradeConcreteTransfer.h"
#include <functional>

namespace gameplay
{
namespace player
{
namespace legacy
{
// SUMMIT350M: stage-only reasons; no account/character/item data or DAT bytes.
// This is only a diagnostic value. Preparation remains fail-closed.
enum class TradePairBuildStage350M : int {
    Unset=0, BadArguments=1, OwnerDecode=2, CompanionDecode=3,
    SelectedMissing=4, SelectedAmbiguous=5, ConcreteTransfer=6, Ready=7
};
struct TradePairBuildDiagnostic350M {
    TradePairBuildStage350M stage=TradePairBuildStage350M::Unset;
    int decode=0, transfer=0, plan=0, ownerImage=0, companionImage=0;
    unsigned unsafeFlags350N=0; // 1 missing, 2 duplicate, 4 cursor, 8 position
    int selection350N=0; // status=InvalidSelection: see TradeSelectionReject350N

};
class LegacyCharacterRecordAdapter final : public ICharacterRepository
{
public:
    // Authorized, full-identity offline DAT inventory read. Does not change DAT.
    // This is NOT a transfer, a lock, or permission to commit a trade.
    gameplay::companions::v2::CompanionTradeInventoryReadCode
    TryLoadTradeInventorySnapshot(const CharacterRecordKey& key,
        gameplay::companions::v2::CompanionTradeInventorySnapshot& out) const;
    // Decode a STABLE COPY of a received/save-buffer record (not a live pointer).
    // Verifies envelope, embedded name and encoded item records only. No DB/DAT
    // access and no permission to trade; caller must serialize the copy itself.
    gameplay::companions::v2::BufferedTradeCaptureCode
    TryDecodeTradeBufferedPayloadCandidate(const std::uint8_t* stableCopy,
        std::size_t copyLength, const char* serverCharacterName,
        gameplay::companions::v2::CompanionTradeInventorySnapshot& out) const;
    // REAL legacy sRECORD_ITEM -> paired DAT image transfer. Unlike the
    // read-only snapshot calls, this removes the original encoded instance
    // from one record and inserts the relocated instance into the other.
    // Does NOT commit or authorize unleased online data. The authenticated
    // caller MUST hold both locks, check the full online ledger, and pass the
    // prepared images to the durable journal before any file is modified.
    bool BuildTradePairedRecordImages(
        const std::vector<std::uint8_t>& ownerSavedBytes,
        const std::vector<std::uint8_t>& companionSavedBytes,
        const char* ownerCharacter, const char* companionCharacter,
        const gameplay::companions::v2::CompanionTradeOffer& offer,
        const std::vector<gameplay::companions::v2::TradeSelectedItem>& selected,
        const std::function<bool(const gameplay::companions::v2::CompanionTradeItemRecord&,
                                  gameplay::companions::v2::TradeSide)>& serverItemPermission,
        gameplay::companions::v2::CompanionTradePrepared& prepared,
        TradePairBuildDiagnostic350M* diagnostics=nullptr) const;
    bool TryLoadProgression(const CharacterRecordKey& key,
                            CharacterProgressionRecord& out) const override;
    bool TryLoadSnapshot(const CharacterRecordKey& key,
                         CharacterPersistentSnapshot& out) const override;
};

const LegacyCharacterRecordAdapter& ServerLegacyCharacterRecordAdapter();
}
}
}
