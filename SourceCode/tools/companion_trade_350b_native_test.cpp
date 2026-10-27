#include "gameplay/companions/v2/CompanionTradeDetachedWorker.h"
#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
using namespace gameplay::companions::v2;
namespace protocol = gameplay::companions::protocol;

static DetachedTradeJob Job(unsigned int id=1, unsigned int generation=1) {
    DetachedTradeJob job;
    job.session.serial=321;
    job.session.generation=generation;
    job.session.account=111;
    job.session.owner=222;
    job.session.companion=333;
    job.acceptedSaveSequence=123;
    job.request.size=sizeof(job.request);
    job.request.code=protocol::OPCODE_COMPANION_TRADE_CONFIRM;
    job.request.version=protocol::COMPANION_TRADE_CONFIRM_VERSION;
    job.request.requestId=id;
    job.request.ownerRevision=42;
    job.request.companionRevision=88;
    job.request.side=0;
    job.request.itemIndex=1;
    job.request.itemCode=4;
    job.request.itemKey=5;
    job.request.itemChecksum=6;
    std::strcpy(job.request.characterName,"Companion");
    return job;
}
struct Executor final : IDetachedTradeExecutor {
    std::atomic<int> calls{0};
    std::atomic<bool> started{false};
    int sleepMs=0;
    DetachedTradeOutcome outcome=DetachedTradeOutcome::Refused(-76);
    DetachedTradeOutcome Execute(const DetachedTradeJob& job) override {
        assert(job.session.Valid());
        ++calls; started=true;
        if(sleepMs) std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
        return outcome;
    }
};
static bool WaitFor(DetachedTradeWorker& worker, const DetachedTradeJob& job, int& status) {
    for(int n=0;n<300;++n) {
        if(worker.TakeForSession(job.session,job.request.requestId,status)) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return false;
}
int main() {
    {
        Executor ex;
        DetachedTradeWorker worker(ex,2);
        assert(worker.Enqueue(Job())==DetachedAdmission::Closed);
        assert(worker.Start());
        assert(!worker.Start());
        auto job=Job();
        assert(worker.Enqueue(job)==DetachedAdmission::Accepted);
        assert(worker.Enqueue(job)==DetachedAdmission::BusyPair);
        int status=0;
        assert(WaitFor(worker,job,status) && status==-76);
        assert(worker.Outstanding()==0);
        assert(ex.calls==1);
        worker.Stop();
        assert(worker.Enqueue(job)==DetachedAdmission::Closed);
    }
    {
        Executor ex;
        ex.outcome=DetachedTradeOutcome::Committed();
        DetachedTradeWorker worker(ex);
        assert(worker.Start());
        auto job=Job(2,13);
        assert(worker.Enqueue(job)==DetachedAdmission::Accepted);
        int status=0;
        auto wrong=Job(2,14);
        for(int n=0;n<50;++n) {
            assert(!worker.TakeForSession(wrong.session,2,status));
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        assert(WaitFor(worker,job,status) && status==2);
        assert(worker.Outstanding()==1); // durable gate remains
        assert(worker.ReleaseAfterVerifiedRecovery(222,333));
        assert(worker.Outstanding()==0);
        worker.Stop();
    }
    {
        Executor ex;
        ex.outcome=DetachedTradeOutcome(-60,true); // misclassified error must become UNKNOWN
        DetachedTradeWorker worker(ex);
        assert(worker.Start());
        auto job=Job(3);
        assert(worker.Enqueue(job)==DetachedAdmission::Accepted);
        int status=0;
        assert(WaitFor(worker,job,status) && status==-74);
        assert(worker.Outstanding()==1);
        assert(worker.ReleaseAfterVerifiedRecovery(222,333));
        worker.Stop();
    }
    {
        Executor ex;
        ex.sleepMs=50;
        DetachedTradeWorker worker(ex,1);
        assert(worker.Start());
        auto job=Job(4);
        assert(worker.Enqueue(job)==DetachedAdmission::Accepted);
        auto busy=job; busy.session.owner=999;busy.session.companion=333;
        assert(worker.Enqueue(busy)==DetachedAdmission::BusyCharacter);
        worker.Stop(); // join worker; must not use freed executor/session
        assert(ex.calls==1);
        int status=0;
        assert(worker.TakeForSession(job.session,4,status)&&status==-76);
    }
    {
        Executor ex;
        DetachedTradeWorker worker(ex,2);
        assert(worker.Start());
        std::atomic<int> admitted{0};
        std::vector<std::thread> callers;
        for(int n=0;n<20;++n) callers.emplace_back([&] {
            if(worker.Enqueue(Job(5))==DetachedAdmission::Accepted)++admitted;
        });
        for(auto& t:callers)t.join();
        assert(admitted==1);
        worker.Stop();
    }
    std::cout << "SUMMIT350B_NATIVE=PASS (joined worker, generation, capacity, duplicate, durable hold, failure, concurrency)\n";
}
