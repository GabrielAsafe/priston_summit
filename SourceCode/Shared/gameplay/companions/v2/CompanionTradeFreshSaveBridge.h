#pragma once
// Authenticated live-save synchronization glue. It binds the existing trade
// sync state machine to the server's ACTUALLY accepted legacy save sequence.
// It is an on-server coordination primitive, NOT a DAT writer or item grant.
// A production caller still must authenticate packets, freeze client state,
// validate captured record/ledger, and perform durable paired persistence.
#include "gameplay/companions/v2/CompanionTradeSaveActivityFence.h"
#include "gameplay/companions/v2/CompanionTradeLiveSyncProtocol.h"
#include <cstdint>
#include <utility>

namespace gameplay { namespace companions { namespace v2 {

enum class TradeFreshSaveCode : std::uint8_t {
    Ready, InvalidSession, NoValidatedSave, NoNewSave,
    IncorrectProof, NotReserved, WrongPhase, RetryOrRecovery
};

class TradeFreshSaveBridge final {
public:
    explicit TradeFreshSaveBridge(SaveActivityFence& fence)
        : fence_(fence) {}

    TradeFreshSaveCode Begin(SaveSessionKey session, TradeSyncIdentity id,
            TradeSyncPairKey pair, std::uint64_t now, std::uint64_t deadline) {
        if (!session.Valid() || id.playerSerial != session.serial ||
            id.connectGeneration != session.generation || !id.Valid())
            return TradeFreshSaveCode::InvalidSession;
        std::uint64_t seq = 0;
        if (!fence_.TryAcceptedSaveSequence(session, seq))
            return TradeFreshSaveCode::NoValidatedSave;
        if (sync_.Begin(id, std::move(pair), now, deadline, seq) != TradeSyncCode::Ok)
            return TradeFreshSaveCode::RetryOrRecovery;
        session_ = session;
        lastAcceptedAtBegin_ = seq;
        return TradeFreshSaveCode::Ready;
    }

    TradeFreshSaveCode Quiesced(const TradeSyncIdentity& id, std::uint64_t now) {
        return sync_.ClientQuiesced(id, now) == TradeSyncCode::Ok ?
            TradeFreshSaveCode::Ready : TradeFreshSaveCode::WrongPhase;
    }

    // Proof MUST be recomputed from a saved record that passed the server's
    // validator. A wire ACK or a client-supplied digest is never a proof.
    TradeFreshSaveCode ObserveAccepted(const TradeSyncRecordProof& proof,
                                       std::uint64_t now) {
        std::uint64_t sequence=0;
        if (!fence_.TryAcceptedSaveSequence(session_,sequence))
            return TradeFreshSaveCode::NoValidatedSave;
        if (sequence<=lastAcceptedAtBegin_)
            return TradeFreshSaveCode::NoNewSave;
        if (proof.saveSequence!=sequence)
            return TradeFreshSaveCode::IncorrectProof;
        return sync_.AcceptedSave(proof,now)==TradeSyncCode::Ok ?
            TradeFreshSaveCode::Ready : TradeFreshSaveCode::IncorrectProof;
    }

    // Verify MUST run under the lease and must calculate a new record+ledger
    // proof from trusted server state, not reuse proof sent over the network.
    // It should also ensure all *other* inventory mutation paths are fenced.
    template<class RecomputeProof>
    TradeFreshSaveCode CaptureUnderLease(const TradeSyncIdentity& id,
               std::uint64_t now, RecomputeProof recompute) {
        if (sync_.Phase()!=TradeSyncPhase::AwaitingServerCapture)
            return TradeFreshSaveCode::WrongPhase;
        SaveTradeReservationLease lease(fence_,session_);
        if (!lease.Acquired()) return TradeFreshSaveCode::NotReserved;
        std::uint64_t sequence=0;
        if (!lease.AcceptedSaveSequence(sequence) || sequence<=lastAcceptedAtBegin_)
            return TradeFreshSaveCode::NotReserved;
        TradeSyncRecordProof verified{};
        if (!recompute(verified) || verified.saveSequence!=sequence)
            return TradeFreshSaveCode::IncorrectProof;
        // CapturedUnderLease verifies equality to the accepted record proof.
        return sync_.CapturedUnderLease(id,verified,now)==TradeSyncCode::Ok ?
            TradeFreshSaveCode::Ready : TradeFreshSaveCode::IncorrectProof;
    }

    TradeSyncPhase Phase() const { return sync_.Phase(); }
    const TradeSyncExchange& Sync() const { return sync_; }
private:
    SaveActivityFence& fence_;
    SaveSessionKey session_{};
    std::uint64_t lastAcceptedAtBegin_=0;
    TradeSyncExchange sync_{};
};
}}}
