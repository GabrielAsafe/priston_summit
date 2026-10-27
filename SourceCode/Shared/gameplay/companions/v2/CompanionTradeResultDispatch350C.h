#pragma once
// SUMMIT 350C: pure session/durable-status validation before reply delivery.
// NOT an installed runtime pump: caller MUST be under the world's lock and
// must independently resolve actual account/player/companion before sending.
#include "gameplay/companions/v2/CompanionTradeDetachedJobQueue.h"
#include <string>

namespace gameplay { namespace companions { namespace v2 {

struct ResolvedTradeSession350C {
    DetachedTradeSession session;
    std::string account, owner, companion;
    bool connected = false;
    bool staleSaveWritesSealed = false;
};

enum class TradeReplyDecision350C { Drop, SendRefusal, SendCommitted, SendRecoveryRequired };

inline TradeReplyDecision350C DecideReply350C(
    const DetachedTradeJob& completed, int status,
    const ResolvedTradeSession350C& live)
{
    if(!live.connected || !completed.session.SameConnection(live.session) ||
       completed.session.companion!=live.session.companion ||
       completed.accountName!=live.account || completed.ownerName!=live.owner ||
       completed.companionName!=live.companion ||
       !ValidTradeConfirmRequest(completed.request) ||
       completed.request.characterName!=live.companion)
        return TradeReplyDecision350C::Drop;
    // Never report any possibly durable result while a stale connection can
    // save its OLD player inventory. Recovering the journal is separate.
    if(status==2 || status==3)
        return live.staleSaveWritesSealed ? TradeReplyDecision350C::SendCommitted
                                        : TradeReplyDecision350C::Drop;
    if(status==-74)
        return live.staleSaveWritesSealed ? TradeReplyDecision350C::SendRecoveryRequired
                                        : TradeReplyDecision350C::Drop;
    if(status<0 && status!=-75)
        return TradeReplyDecision350C::SendRefusal;
    return TradeReplyDecision350C::Drop;
}

}}}
