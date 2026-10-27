#pragma once
// Server-side proof construction from a stable save copy and the ONLINE
// identity ledger. The caller MUST independently authenticate the session,
// accept the save through the legacy validator, and keep the save reservation
// through durable commit. This header performs no IO and cannot grant items.
#include "gameplay/companions/v2/CompanionTradeSessionCandidateGate.h"
#include "gameplay/companions/v2/CompanionTradeLiveSyncProtocol.h"
#include "gameplay/companions/v2/CompanionTradeSha256.h"
#include <vector>
#include <cstdint>

namespace gameplay { namespace companions { namespace v2 {
class TradeRecordProofBuilder final {
    static void Append32(std::vector<std::uint8_t>& out,std::uint32_t v) {
        for(int i=0;i<4;++i) out.push_back(static_cast<std::uint8_t>(v>>(i*8)));
    }
public:
    static bool Build(const SessionCandidate& captured,
                      const TradeSyncIdentity& authenticatedSession,
                      std::uint64_t serverAcceptedSequence,
                      std::uint32_t onlineCapacity,
                      TradeSyncRecordProof& out) {
        if (captured.status!=SessionCandidateStatus::CandidateOnly ||
            !authenticatedSession.Valid() || !serverAcceptedSequence ||
            !onlineCapacity || captured.stableRecordBytes.empty() ||
            captured.online.entries.size()>onlineCapacity ||
            captured.online.session.id.value!=authenticatedSession.playerSerial ||
            captured.online.session.generation!=authenticatedSession.connectGeneration ||
            !captured.record.TradeEligible()) return false;
        OnlineTradeIdentitySnapshot ledger=captured.online;
        if (!OnlineTradeIdentityPolicy::Finalize(ledger,onlineCapacity) ||
            !OnlineTradeIdentityPolicy::ExactlyMatches(ledger,captured.online)) return false;
        if (OnlineTradePersistedPolicy::Compare(ledger,captured.record,onlineCapacity).code !=
            OnlineTradePersistedCode::MatchReadOnly) return false;
        std::vector<std::uint8_t> canonical;
        canonical.reserve(20 + ledger.entries.size()*16);
        // Protocol domain separator (SUMMIT trade player-identity ledger v1).
        Append32(canonical,0x31534454u);
        Append32(canonical,authenticatedSession.playerSerial);
        Append32(canonical,authenticatedSession.connectGeneration);
        Append32(canonical,onlineCapacity);
        Append32(canonical,static_cast<std::uint32_t>(ledger.entries.size()));
        for (const auto& entry : ledger.entries) {
            Append32(canonical,entry.slot);
            Append32(canonical,entry.identity.item.value);
            Append32(canonical,entry.identity.key);
            Append32(canonical,entry.identity.checksum);
        }
        TradeSyncRecordProof proof{};
        proof.identity=authenticatedSession;
        proof.saveSequence=serverAcceptedSequence;
        proof.ledgerRevision=ledger.revision;
        proof.recordSha256=TradeSha256::Hash(captured.stableRecordBytes.data(),
                                               captured.stableRecordBytes.size());
        proof.identitySha256=TradeSha256::Hash(canonical.data(),canonical.size());
        if (!proof.Valid()) return false;
        out=proof;
        return true;
    }
};
}}}
