#pragma once

#include <algorithm>
#include <vector>

#include "gameplay/commerce/v2/PersonalShopSession.h"

namespace gameplay
{
namespace commerce
{
class PersonalShopSessionService final : public IPersonalShopSessionService
{
public:
    explicit PersonalShopSessionService(std::size_t maxEntries = 16u,
                                        std::uint32_t cooldownMs = 500u)
        : maxEntries_(maxEntries), cooldownMs_(cooldownMs) {}

    bool CooldownActive(PlayerHandle buyer, std::uint32_t nowMs) const override
    {
        if(!buyer.Valid())
            return false;
        const Session* session = Find(buyer);
        return session && session->nextAllowedAt > nowMs;
    }

    PersonalShopSessionRecordResult RecordPrepared(
        PlayerHandle buyer,
        std::uint32_t nowMs,
        const PersonalShopSessionFingerprint& fingerprint) override
    {
        PersonalShopSessionRecordResult result;
        if(!buyer.Valid() || !fingerprint.itemCode || maxEntries_ == 0u)
            return result;

        PruneStaleGeneration(buyer);
        Session* session = FindMutable(buyer);
        if(!session)
        {
            Session created;
            created.buyer = buyer;
            sessions_.push_back(created);
            session = &sessions_.back();
        }

        session->nextAllowedAt = nowMs + cooldownMs_;
        if(std::find(session->prepared.begin(), session->prepared.end(), fingerprint) !=
           session->prepared.end())
        {
            result.code = PersonalShopSessionRecordCode::Duplicate;
            return result;
        }

        if(session->prepared.size() < maxEntries_)
        {
            session->prepared.push_back(fingerprint);
        }
        else
        {
            session->prepared[session->nextSlot % maxEntries_] = fingerprint;
        }
        ++session->nextSlot;
        result.code = PersonalShopSessionRecordCode::Recorded;
        return result;
    }

    std::size_t Clear(PlayerHandle buyer) override
    {
        const std::size_t before = sessions_.size();
        sessions_.erase(std::remove_if(sessions_.begin(), sessions_.end(),
            [buyer](const Session& session)
            {
                return session.buyer.id == buyer.id &&
                       session.buyer.generation == buyer.generation;
            }), sessions_.end());
        return before - sessions_.size();
    }

private:
    struct Session
    {
        PlayerHandle buyer;
        std::uint32_t nextAllowedAt;
        std::size_t nextSlot;
        std::vector<PersonalShopSessionFingerprint> prepared;
        Session() : buyer(), nextAllowedAt(0u), nextSlot(0u), prepared() {}
    };

    const Session* Find(PlayerHandle buyer) const
    {
        for(std::vector<Session>::const_iterator it = sessions_.begin(); it != sessions_.end(); ++it)
            if(it->buyer.id == buyer.id && it->buyer.generation == buyer.generation)
                return &*it;
        return 0;
    }

    Session* FindMutable(PlayerHandle buyer)
    {
        for(std::vector<Session>::iterator it = sessions_.begin(); it != sessions_.end(); ++it)
            if(it->buyer.id == buyer.id && it->buyer.generation == buyer.generation)
                return &*it;
        return 0;
    }

    void PruneStaleGeneration(PlayerHandle buyer)
    {
        sessions_.erase(std::remove_if(sessions_.begin(), sessions_.end(),
            [buyer](const Session& session)
            {
                return session.buyer.id == buyer.id &&
                       session.buyer.generation != buyer.generation;
            }), sessions_.end());
    }

    std::size_t maxEntries_;
    std::uint32_t cooldownMs_;
    std::vector<Session> sessions_;
};
}
}
