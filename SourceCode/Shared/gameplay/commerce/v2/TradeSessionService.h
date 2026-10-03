#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "gameplay/commerce/v2/TradeSession.h"

namespace gameplay
{
namespace commerce
{
class TradeReplayGuardService final : public ITradeReplayGuard
{
public:
    explicit TradeReplayGuardService(std::uint32_t duplicateWindowMs = 8000u)
        : duplicateWindowMs_(duplicateWindowMs) {}

    TradeCommitGuardResult Check(
        PlayerHandle sender,
        const TradeCommitFingerprint& fingerprint,
        std::uint32_t nowMs) const override
    {
        TradeCommitGuardResult result;
        result.tradeId = BuildTradeId(fingerprint);
        if(!sender.Valid())
        {
            result.code = TradeCommitGuardCode::SenderNotFound;
            return result;
        }

        const Session* session = Find(sender);
        if(!session)
        {
            result.code = TradeCommitGuardCode::Fresh;
            return result;
        }

        const std::uint32_t elapsed = nowMs - session->lastCommittedAt;
        result.code = elapsed < duplicateWindowMs_ && session->lastTradeId == result.tradeId
                          ? TradeCommitGuardCode::Duplicate
                          : TradeCommitGuardCode::Fresh;
        return result;
    }

    void MarkCommitted(
        PlayerHandle sender,
        const TradeCommitFingerprint& fingerprint,
        std::uint32_t nowMs) override
    {
        if(!sender.Valid())
            return;

        PruneStaleGeneration(sender);
        Session* session = FindMutable(sender);
        if(!session)
        {
            Session created;
            created.sender = sender;
            sessions_.push_back(created);
            session = &sessions_.back();
        }

        session->lastCommittedAt = nowMs;
        session->lastTradeId = BuildTradeId(fingerprint);
    }

    std::size_t Clear(PlayerHandle sender) override
    {
        const std::size_t before = sessions_.size();
        sessions_.erase(std::remove_if(sessions_.begin(), sessions_.end(),
            [sender](const Session& session)
            {
                return session.sender.id == sender.id &&
                       session.sender.generation == sender.generation;
            }), sessions_.end());
        return before - sessions_.size();
    }

private:
    struct Session
    {
        PlayerHandle sender;
        std::uint32_t lastCommittedAt;
        TradeId lastTradeId;
        Session() : sender(), lastCommittedAt(0u), lastTradeId() {}
    };

    const Session* Find(PlayerHandle sender) const
    {
        for(std::vector<Session>::const_iterator it = sessions_.begin(); it != sessions_.end(); ++it)
            if(it->sender.id == sender.id && it->sender.generation == sender.generation)
                return &*it;
        return 0;
    }

    Session* FindMutable(PlayerHandle sender)
    {
        for(std::vector<Session>::iterator it = sessions_.begin(); it != sessions_.end(); ++it)
            if(it->sender.id == sender.id && it->sender.generation == sender.generation)
                return &*it;
        return 0;
    }

    void PruneStaleGeneration(PlayerHandle sender)
    {
        sessions_.erase(std::remove_if(sessions_.begin(), sessions_.end(),
            [sender](const Session& session)
            {
                return session.sender.id == sender.id &&
                       session.sender.generation != sender.generation;
            }), sessions_.end());
    }

    std::uint32_t duplicateWindowMs_;
    std::vector<Session> sessions_;
};
}
}
