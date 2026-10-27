#pragma once
// SUMMIT 350C: immutable, bounded, authenticated-source capture. NOT a trade engine.
// Must be called only after the legacy save subsystem has acknowledged and
// independently validated the EXACT owner record bytes, save epoch and session.
// No rsPLAYINFO, sockets, filesystem, mutable memory alias or threading here.
#include "gameplay/companions/v2/CompanionTradeDetachedJobQueue.h"
#include "gameplay/companions/v2/CompanionTradeSha256.h"
#include "gameplay/companions/v2/CompanionTradeParticipantIdentity.h"
#include "gameplay/companions/v2/CompanionTradeParticipantGate350B.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

namespace gameplay { namespace companions { namespace v2 {

struct AcceptedTradeSource350C {
    // ALL fields must originate from an authenticated server-side state, not
    // a packet, preview, or unacknowledged client-side record.
    DetachedTradeSession session;
    std::string account, owner, companion;
    std::uint64_t acceptedSaveSequence = 0;
    std::uint64_t leaseToken = 0;
    std::uint64_t tradeNonce = 0;
    const std::uint8_t* completeRecord = nullptr;
    std::size_t allocatedRecordBytes = 0;
    std::size_t acknowledgedRecordBytes = 0;
    TradeSha256Digest acknowledgedDigest{};
};

enum class TradeCaptureCode350C {
    Captured, InvalidSession, InvalidIdentity, UntrustedSave,
    IncorrectLength, DigestMismatch
};

// Complete immutable bytes; never accept a length taken solely from the
// in-buffer legacy `size` member, which cannot prove allocation capacity.
// The save subsystem must provide independently bounded allocatedRecordBytes.
inline TradeCaptureCode350C CaptureAcceptedTrade350C(
    const AcceptedTradeSource350C& source,
    const protocol::CompanionTradeConfirmPacket& packet,
    DetachedTradeJob& output)
{
    // Generation zero is valid on the first legacy connection (SaveSessionKey).
    // Only serial and server-authenticated participant IDs must be non-zero.
    // SameConnection still matches the exact generation during result delivery.
    if(!source.session.Valid() ||
       !source.session.account || !source.session.owner ||
       !source.session.companion)
        return TradeCaptureCode350C::InvalidSession;
    if(!ValidTradeConfirmRequest(packet) || source.account.empty() ||
       source.owner.empty() || source.companion.empty() ||
       source.account.size()>31 || source.owner.size()>31 ||
       source.companion.size()>31 || source.owner==source.companion ||
       packet.characterName[0]==0 ||
       source.companion!=std::string(packet.characterName))
        return TradeCaptureCode350C::InvalidIdentity;
    // Reject mismatched identity hashes. TradeParticipantId normalizes to
    // legacy character semantics, but the caller must independently confirm
    // the companion belongs to this account and is OFFLINE.
    if(TradeParticipantId(source.account.c_str(),source.owner.c_str()) != source.session.owner ||
       TradeParticipantId(source.account.c_str(),source.companion.c_str()) != source.session.companion)
        return TradeCaptureCode350C::InvalidIdentity;
    if(!source.acceptedSaveSequence || !source.leaseToken || !source.tradeNonce ||
       !source.completeRecord) return TradeCaptureCode350C::UntrustedSave;
    const std::size_t maximum=8u*1024u*1024u;
    if(source.acknowledgedRecordBytes==0 ||
       source.acknowledgedRecordBytes>maximum ||
       source.acknowledgedRecordBytes>source.allocatedRecordBytes ||
       source.allocatedRecordBytes>maximum)
        return TradeCaptureCode350C::IncorrectLength;
    const auto actual=TradeSha256::Hash(source.completeRecord,source.acknowledgedRecordBytes);
    if(actual!=source.acknowledgedDigest)
        return TradeCaptureCode350C::DigestMismatch;
    DetachedTradeJob candidate;
    candidate.session=source.session;
    candidate.accountName=source.account;
    candidate.ownerName=source.owner;
    candidate.companionName=source.companion;
    candidate.acceptedSaveSequence=source.acceptedSaveSequence;
    candidate.leaseToken=source.leaseToken;
    candidate.tradeNonce=source.tradeNonce;
    candidate.request=packet;
    candidate.acceptedOwnerRecord.assign(source.completeRecord,
        source.completeRecord+source.acknowledgedRecordBytes);
    // Defensive check for a data race during capture. The producer must
    // still own the save buffer and its mutation fence throughout this call.
    if(TradeSha256::Hash(candidate.acceptedOwnerRecord.data(),
                        candidate.acceptedOwnerRecord.size())!=source.acknowledgedDigest)
        return TradeCaptureCode350C::DigestMismatch;
    output=std::move(candidate); // never mutate output on a rejected capture
    return TradeCaptureCode350C::Captured;
}


// Atomic-looking admission at the QUEUE boundary: reserve both participant
// names against concurrent login, then admit one owned immutable job. The
// process-local gate is NOT a substitute for SaveActivityFence or the durable
// journal; caller must hold its own mutation fence for the full handoff.
inline DetachedAdmission AdmitCapturedTrade350C(
    TradeParticipantGate350B& gate, DetachedTradeJobQueue& queue,
    const AcceptedTradeSource350C& source,
    const protocol::CompanionTradeConfirmPacket& packet)
{
    DetachedTradeJob job;
    if(CaptureAcceptedTrade350C(source,packet,job)!=TradeCaptureCode350C::Captured)
        return DetachedAdmission::Invalid;
    if(!gate.TryReserve(job.session.owner,job.session.companion))
        return DetachedAdmission::BusyCharacter;
    try {
        const auto result=queue.Enqueue(job);
        if(result != DetachedAdmission::Accepted)
            (void)gate.ReleaseVerified(job.session.owner,job.session.companion);
        return result;
    } catch(...) {
        (void)gate.ReleaseVerified(job.session.owner,job.session.companion);
        throw;
    }
}

}}} // namespace gameplay::companions::v2
