#include "gameplay/companions/v2/CompanionTradeDetachedJobQueue.h"
#include "gameplay/companions/v2/CompanionTradeSaveActivityFence.h"
#include <cassert>
#include <atomic>
#include <cstring>
#include <thread>
#include <vector>
#include <iostream>
using namespace gameplay::companions::v2;

static DetachedTradeJob Make(unsigned owner, unsigned companion, unsigned serial,
                             unsigned generation, unsigned long long id=1) {
    DetachedTradeJob job;
    job.session={serial,generation,12,owner,companion};
    job.acceptedSaveSequence=101;
    job.request.size=sizeof(job.request);
    job.request.code=gameplay::companions::protocol::OPCODE_COMPANION_TRADE_CONFIRM;
    job.request.version=gameplay::companions::protocol::COMPANION_TRADE_CONFIRM_VERSION;
    job.request.side=0;
    job.request.itemIndex=3;
    job.request.itemCode=4;
    job.request.itemKey=5;
    job.request.itemChecksum=6;
    job.request.ownerRevision=7;
    job.request.companionRevision=8;
    job.request.requestId=id;
    std::strcpy(job.request.characterName,"OFFLINE");
    return job;
}
static void AdmissionAndSessionTests() {
    DetachedTradeJobQueue q(2);
    auto a=Make(11,22,33,4,1);
    assert(q.Enqueue(a)==DetachedAdmission::Accepted);
    assert(q.Enqueue(a)==DetachedAdmission::BusyPair); // duplicate is not re-run
    assert(q.Enqueue(Make(22,11,99,1,2))==DetachedAdmission::BusyCharacter);
    assert(q.Enqueue(Make(11,44,99,1,2))==DetachedAdmission::BusyCharacter);
    auto b=Make(55,66,77,4,3);
    assert(q.Enqueue(b)==DetachedAdmission::Accepted);
    assert(q.Enqueue(Make(88,99,100,1,4))==DetachedAdmission::Full);
    assert(q.Outstanding()==2);
    DetachedTradeJob out;
    assert(q.TryNext(out) && out.request.requestId==1);
    a.request.characterName[0]='X'; // queue owns the inbound packet copy
    assert(out.request.characterName[0]=='O');
    assert(q.Complete(out,-74));
    assert(!q.Complete(out,-74));
    auto recycled=out.session;
    ++recycled.generation;
    int status=0;
    assert(!q.TakeForSession(recycled,1,status));
    assert(q.Outstanding()==2); // recycle cannot release an uncertain transaction
    assert(q.TakeForSession(out.session,1,status) && status==-74);
    assert(q.Outstanding()==2); // -74 is still reserved until verified recovery
    assert(q.TryNext(out) && out.request.requestId==3);
    assert(!q.TryNext(out));
    assert(q.Complete(out,-76));
    q.DiscardFinishedForOldSession(out.session);
    assert(q.Outstanding()==1); // uncertain pair is still quarantined
    assert(q.Enqueue(Make(11,22,101,1,9))==DetachedAdmission::BusyPair);
    assert(q.ReleaseAfterVerifiedRecovery(11,22)); // test simulates external proof
    assert(!q.ReleaseAfterVerifiedRecovery(11,22));
    assert(q.Outstanding()==0);
    q.CloseAdmission();
    assert(q.Enqueue(Make(44,55,77,5))==DetachedAdmission::Closed);
}
static void ParallelPairTest() {
    DetachedTradeJobQueue q(12);
    constexpr unsigned count=32;
    std::atomic<int> accepted(0);
    std::vector<std::thread> threads;
    for(unsigned i=0;i<count;++i) threads.emplace_back([&q,&accepted,i] {
        auto request=Make(11,22,i+1,1,i+1);
        if(q.Enqueue(request)==DetachedAdmission::Accepted) ++accepted;
    });
    for(auto& t:threads)t.join();
    assert(accepted==1 && q.Outstanding()==1);
}
static void FenceTests() {
    SaveActivityFence fence;
    const SaveSessionKey original(7,3);
    bool tracked=false;
    assert(fence.BeginMutation(original,tracked) && tracked);
    std::uint64_t token=0;
    assert(!fence.TryReserve(original,token));
    fence.EndMutation(original,tracked);
    assert(fence.BeginReceive(original,0,1));
    fence.EndReceive(original,true,true);
    assert(fence.TryReserve(original,token) && token);
    assert(!fence.BeginMutation(original,tracked));
    assert(fence.SealCommittedForReconnect(original,token));
    assert(fence.ReleaseReservation(original,token));
    assert(!fence.BeginMutation(original,tracked));
    assert(!fence.TryReserve(original,token));
    const SaveSessionKey newGeneration(7,4);
    assert(fence.BeginMutation(newGeneration,tracked));
    fence.EndMutation(newGeneration,tracked);
}
int main() {
    auto bad=Make(11,22,33,4);
    bad.request.itemKey=0;
    DetachedTradeJobQueue q;
    assert(q.Enqueue(bad)==DetachedAdmission::Invalid);
    auto noReceipt=Make(11,22,33,4);noReceipt.acceptedSaveSequence=0;
    assert(q.Enqueue(noReceipt)==DetachedAdmission::Invalid);
    AdmissionAndSessionTests();
    ParallelPairTest();
    FenceTests();
    std::cout << "SUMMIT350A_NATIVE=PASS (mailbox, session generation, reservation, concurrency)\n";
}
