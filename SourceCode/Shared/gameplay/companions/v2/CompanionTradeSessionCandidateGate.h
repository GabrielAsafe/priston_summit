#pragma once
// Development integration seam: a lease-bound READ-ONLY capture of the full
// player record that was last accepted by the legacy save server.
// A valid capture is NEVER sufficient to execute an item transfer: the data
// was originally sent by the client and not all legacy mutation paths are
// covered by SaveActivityFence. A separate authenticated live-synchronization
// barrier, pairwise durable commit and client refresh are still mandatory.
#include "gameplay/companions/v2/CompanionTradeSaveActivityFence.h"
#include "gameplay/companions/v2/CompanionTradeBufferedPayloadCandidate.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gameplay { namespace companions { namespace v2 {

enum class SessionCandidateStatus {
    NoReservation, SessionMismatch, LedgerUnavailable, CopyUnavailable,
    UnsafeRecord, StaleLedger, IdentityMismatch, CallbackRejected, CandidateOnly
};
struct SessionCandidate {
    OnlineTradeIdentitySnapshot online;
    CompanionTradeInventorySnapshot record;
    std::vector<std::uint8_t> stableRecordBytes;
    SessionCandidateStatus status = SessionCandidateStatus::NoReservation;
    bool CanCommitTrade() const { return false; }
};

struct SessionCandidateGate {
    // readLedger and copyRecord execute entirely while a reservation is held.
    // copyRecord MUST serialize access to lpRecordDataBuff and copy the full,
    // already validated record; passing the pointer through is forbidden.
    // sameSession must check the live authenticated registry -> rsPLAYINFO
    // mapping (including the connection generation), not only the character.
    // A failed callback leaves out unchanged.
    template<class ReadLedger, class CopyRecord, class Decode, class SameSession,
             class WhileLeased>
    static SessionCandidateStatus CaptureWithLease(
        SaveActivityFence& fence, SaveSessionKey session,
        const char* expectedName, const BufferedTradeEnvelopeLayout& layout,
        int maxItems, std::uint32_t ledgerCapacity,
        ReadLedger readLedger, CopyRecord copyRecord, Decode decode,
        SameSession sameSession, WhileLeased whileLeased, SessionCandidate& out)
    {
        SaveTradeReservationLease lease(fence, session);
        if (!lease.Acquired()) return SessionCandidateStatus::NoReservation;
        if (!expectedName || !*expectedName || maxItems <= 0 || !ledgerCapacity)
            return SessionCandidateStatus::SessionMismatch;

        OnlineTradeIdentitySnapshot first, second;
        if (!readLedger(first) || !first.session.Valid())
            return SessionCandidateStatus::LedgerUnavailable;
        if (!sameSession(session, first.session))
            return SessionCandidateStatus::SessionMismatch;
        const std::uint64_t firstRevision = first.revision;
        if (!firstRevision ||
            !OnlineTradeIdentityPolicy::Finalize(first, ledgerCapacity) ||
            first.revision != firstRevision)
            return SessionCandidateStatus::LedgerUnavailable;

        std::vector<std::uint8_t> stable;
        if (!copyRecord(stable) || stable.empty() || stable.size() > layout.maxBytes)
            return SessionCandidateStatus::CopyUnavailable;
        CompanionTradeInventorySnapshot record;
        const BufferedTradeCaptureCode decoded = BufferedTradePayloadPolicy::DecodeBounded(
            stable.data(), stable.size(), layout, expectedName, maxItems, decode, record);
        if (decoded != BufferedTradeCaptureCode::CandidateReadOnly)
            return SessionCandidateStatus::UnsafeRecord;

        if (!readLedger(second) || !second.session.Valid() ||
            !sameSession(session, second.session))
            return SessionCandidateStatus::SessionMismatch;
        const std::uint64_t secondRevision = second.revision;
        if (!secondRevision ||
            !OnlineTradeIdentityPolicy::Finalize(second, ledgerCapacity) ||
            second.revision != secondRevision ||
            !OnlineTradeIdentityPolicy::ExactlyMatches(first, second))
            return SessionCandidateStatus::StaleLedger;
        const auto verified = OnlineTradePersistedPolicy::Compare(second, record, ledgerCapacity);
        if (verified.code != OnlineTradePersistedCode::MatchReadOnly)
            return SessionCandidateStatus::IdentityMismatch;

        SessionCandidate candidate;
        candidate.online = second;
        candidate.record = std::move(record);
        candidate.stableRecordBytes = std::move(stable);
        candidate.status = SessionCandidateStatus::CandidateOnly;
        // Invoked with the exclusive lease STILL held. The callback must
        // provide its own authenticated freshness and durable-commit checks;
        // read-only candidate status does not grant permission to write.
        if (!whileLeased(candidate))
            return SessionCandidateStatus::CallbackRejected;
        out = std::move(candidate);
        return SessionCandidateStatus::CandidateOnly;
    }
    // Convenience read-only inspection. The lease ends on return: never use
    // a returned copy as the source of an UNLOCKED commit.
    template<class ReadLedger, class CopyRecord, class Decode, class SameSession>
    static SessionCandidateStatus Capture(
        SaveActivityFence& fence, SaveSessionKey session,
        const char* expectedName, const BufferedTradeEnvelopeLayout& layout,
        int maxItems, std::uint32_t ledgerCapacity,
        ReadLedger readLedger, CopyRecord copyRecord, Decode decode,
        SameSession sameSession, SessionCandidate& out)
    {
        return CaptureWithLease(fence, session, expectedName, layout,
            maxItems, ledgerCapacity, readLedger, copyRecord, decode,
            sameSession, [](const SessionCandidate&){ return true; }, out);
    }
};
}}}
