#include "gameplay/companions/v2/CompanionTradeDetachedJobQueue.h"
#include "gameplay/companions/v2/CompanionTradeParticipantGate350B.h"
#include "gameplay/companions/v2/CompanionTradeSaveActivityFence.h"
#include <cassert>
#include <cstring>
#include <iostream>
using namespace gameplay::companions::v2;
namespace protocol=gameplay::companions::protocol;
int main() {
    SaveActivityFence fence;
    TradeParticipantGate350B gate;
    const SaveSessionKey key(150,7);
    assert(fence.BeginReceive(key,0,1));
    fence.EndReceive(key,true,true);
    std::uint64_t seq=0;
    assert(fence.TryAcceptedSaveSequence(key,seq) && seq);
    std::uint64_t token=0;
    assert(fence.TryReserve(key,token) && token);
    assert(gate.TryReserve(101,202));
    DetachedTradeJob j;
    j.session.serial=150;j.session.generation=7;
    j.session.account=1;j.session.owner=101;j.session.companion=202;
    j.acceptedSaveSequence=seq;j.leaseToken=token;j.tradeNonce=88;
    j.accountName="account";j.ownerName="owner";j.companionName="companion";
    j.acceptedOwnerRecord={1,2,3};
    j.request.size=sizeof(j.request);
    j.request.code=protocol::OPCODE_COMPANION_TRADE_CONFIRM;
    j.request.version=protocol::COMPANION_TRADE_CONFIRM_VERSION;
    j.request.requestId=88;j.request.side=0;j.request.itemIndex=1;
    j.request.itemCode=2;j.request.itemKey=3;j.request.itemChecksum=4;
    j.request.ownerRevision=5;j.request.companionRevision=6;
    std::strcpy(j.request.characterName,"companion");
    DetachedTradeJobQueue q(2);
    assert(q.Enqueue(j)==DetachedAdmission::Accepted);
    assert(q.Outstanding()==1);
    assert(!gate.TryReserve(101,202));
    DetachedTradeJob running;
    assert(q.TryNext(running));
    assert(fence.SealCommittedForReconnect(key,token));
    assert(q.Complete(running,2));
    int status=0;
    assert(q.TakeForSession(j.session,j.request.requestId,status) && status==2);
    assert(q.Outstanding()==1); // retained until verified and old session gone
    assert(gate.IsReserved(101));
    bool tracked=false;
    assert(!fence.BeginMutation(key,tracked));
    assert(q.ReleaseAfterVerifiedRecovery(101,202));
    assert(gate.ReleaseVerified(101,202));
    assert(!gate.IsReserved(101));
    assert(q.Outstanding()==0);
    assert(!fence.BeginMutation(key,tracked)); // old session stays SEALED
    std::cout << "SUMMIT350H_LIFETIME=PASS (durable hold, safe release, stale-save quarantine)\n";
}
