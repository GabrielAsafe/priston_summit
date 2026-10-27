#include "gameplay/companions/v2/CompanionTradeDetachedJournalWorker350B.h"
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <cstring>
using namespace gameplay::companions::v2;
namespace protocol=gameplay::companions::protocol;

struct DiskSim {
    CompanionTradePrepared pending;
    bool hasIntent=false, hasReceipt=false, healthy=true;
    int owner=10, companion=20;
    int failAt=0; // 1 seal intent, 2 companion replace, 3 owner replace, 4 receipt
    int replaceOwner=0,replaceCompanion=0, prepares=0;
};
struct FakePort final : ICompanionTradeJournalPort {
    DiskSim& disk;
    explicit FakePort(DiskSim& d):disk(d){}
    bool LockPair(std::uint64_t,std::uint64_t) override {return disk.healthy;}
    void UnlockPair(std::uint64_t,std::uint64_t) override {}
    CompanionTradeReceipt Receipt(std::uint64_t nonce) override {
        CompanionTradeReceipt r;
        if(disk.hasReceipt && disk.pending.offer.tradeNonce==nonce) {
            r.found=true;r.committed=true;
            r.offerDigest=disk.pending.offer.offerDigest;
            r.offerRevision=disk.pending.offer.offerRevision;
            r.authenticatedOwner=disk.pending.offer.authenticatedOwner;
            r.companionCharacter=disk.pending.offer.companionCharacter;
        }
        return r;
    }
    CompanionTradePending PendingForPair(std::uint64_t,std::uint64_t) override {
        CompanionTradePending p;
        p.present=disk.hasIntent;
        if(p.present) p.plan=disk.pending;
        return p;
    }
    bool Prepare(const CompanionTradeOffer& o, CompanionTradePrepared& p) override {
        ++disk.prepares;
        p.offer=o;
        p.ownerBefore={10};p.ownerAfter={11};
        p.companionBefore={20};p.companionAfter={21};
        return true;
    }
    bool SealIntent(const CompanionTradePrepared& p) override {
        if(disk.failAt==1){disk.healthy=false;return false;}
        disk.pending=p;disk.hasIntent=true;return true;
    }
    TradeApplyResult ApplyExact(TradeParticipant who,const CompanionTradePrepared& p) override {
        if(!disk.hasIntent || p.offer.tradeNonce!=disk.pending.offer.tradeNonce)
            return TradeApplyResult::IoUncertain;
        if(who==TradeParticipant::OwnerOnline) {
            if(disk.failAt==3)return TradeApplyResult::IoUncertain;
            if(disk.owner==11)return TradeApplyResult::AlreadyApplied;
            if(disk.owner!=10)return TradeApplyResult::Conflict;
            disk.owner=11;++disk.replaceOwner;
            return TradeApplyResult::AppliedDurably;
        }
        if(disk.failAt==2)return TradeApplyResult::IoUncertain;
        if(disk.companion==21)return TradeApplyResult::AlreadyApplied;
        if(disk.companion!=20)return TradeApplyResult::Conflict;
        disk.companion=21;++disk.replaceCompanion;
        return TradeApplyResult::AppliedDurably;
    }
    bool SealReceipt(const CompanionTradePrepared&) override {
        if(disk.failAt==4)return false;
        disk.hasReceipt=true;return true;
    }
    bool RemoveIntent(const CompanionTradePrepared&) override {
        disk.hasIntent=false;return true;
    }
};
struct Ctx final : IDetachedJournalContext {
    DiskSim& d;
    FakePort port;
    explicit Ctx(DiskSim& disk):d(disk),port(disk){}
    ICompanionTradeJournalPort& Port() override{return port;}
    bool Healthy() const override{return d.healthy;}
};
struct Factory final:IDetachedJournalContextFactory {
    DiskSim& disk;
    explicit Factory(DiskSim& d):disk(d){}
    std::unique_ptr<IDetachedJournalContext> Open(const DetachedTradeJob&) override {
        return std::unique_ptr<IDetachedJournalContext>(new Ctx(disk));
    }
};
static DetachedTradeJob ValidJob() {
    DetachedTradeJob j;
    j.session.serial=7;j.session.generation=9;
    j.session.account=123;j.session.owner=111;j.session.companion=222;
    j.acceptedSaveSequence=15;j.leaseToken=16;j.tradeNonce=17;
    j.accountName="acct";j.ownerName="player";j.companionName="companion";
    j.acceptedOwnerRecord={11,22,33};
    j.request.size=sizeof(j.request);
    j.request.code=protocol::OPCODE_COMPANION_TRADE_CONFIRM;
    j.request.version=protocol::COMPANION_TRADE_CONFIRM_VERSION;
    j.request.requestId=17;j.request.ownerRevision=18;
    j.request.companionRevision=19;j.request.side=0;
    j.request.itemIndex=1;j.request.itemCode=1;
    j.request.itemKey=2;j.request.itemChecksum=3;
    std::strcpy(j.request.characterName,"companion");
    return j;
}
int main(){
    for(int fail=0;fail<=4;++fail){
        DiskSim disk;disk.failAt=fail;
        Factory factory(disk);
        DetachedJournalExecutor350B executor(factory);
        const DetachedTradeJob job=ValidJob();
        const auto first=executor.Execute(job);
        if(!fail){
            assert(first.status==2 && first.durableOrUncertain);
            assert(disk.owner==11 && disk.companion==21 && disk.hasReceipt);
            assert(disk.prepares==1);
            assert(!disk.hasIntent);
        } else {
            assert(first.status==-74 && first.durableOrUncertain);
            if(fail==1)assert(!disk.hasIntent);
            else assert(disk.hasIntent);
            // Simulated server restart with retained disk intent & receipt;
            // do NOT fabricate rollback after first-DAT write.
            disk.healthy=true;disk.failAt=0;
            if(disk.hasIntent){
                auto recovered=executor.Execute(job);
                assert(recovered.status==3);
                assert(!disk.hasIntent && disk.hasReceipt);
                assert(disk.owner==11 && disk.companion==21);
                assert(disk.prepares==1); // recovery cannot rebuild newer images
                assert(disk.replaceOwner==1 && disk.replaceCompanion==1);
            }
        }
    }
    DiskSim disk;
    Factory factory(disk);
    DetachedJournalExecutor350B exec(factory);
    auto invalid=ValidJob();invalid.acceptedOwnerRecord.clear();
    assert(exec.Execute(invalid).status==-76);
    assert(disk.prepares==0);
    // SUMMIT350H: a pending intent for DIFFERENT nonce is recovered for
    // safety but never acknowledged as this request's success.
    {
        DiskSim previous; previous.hasIntent=true;
        auto incoming=ValidJob();
        previous.pending.offer.authenticatedOwner=incoming.session.owner;
        previous.pending.offer.companionCharacter=incoming.session.companion;
        previous.pending.offer.tradeNonce=999;
        previous.pending.offer.offerDigest=101;
        previous.pending.offer.offerRevision=1;
        previous.pending.ownerBefore={10};previous.pending.ownerAfter={11};
        previous.pending.companionBefore={20};previous.pending.companionAfter={21};
        Factory priorFactory(previous);
        DetachedJournalExecutor350B separate(priorFactory);
        auto result=separate.Execute(incoming);
        assert(result.status==-74 && result.durableOrUncertain);
        assert(!previous.hasIntent && previous.hasReceipt);
        assert(previous.owner==11 && previous.companion==21);
    }
    // A corrupt or uncertain intent cannot be labelled a simple pre-intent refusal.
    {
        DiskSim uncertain; uncertain.hasIntent=true;uncertain.failAt=2;
        auto incoming=ValidJob();
        uncertain.pending.offer.authenticatedOwner=incoming.session.owner;
        uncertain.pending.offer.companionCharacter=incoming.session.companion;
        uncertain.pending.offer.tradeNonce=999;
        uncertain.pending.offer.offerDigest=101;
        uncertain.pending.offer.offerRevision=1;
        uncertain.pending.ownerBefore={10};uncertain.pending.ownerAfter={11};
        uncertain.pending.companionBefore={20};uncertain.pending.companionAfter={21};
        Factory priorFactory(uncertain);
        DetachedJournalExecutor350B separate(priorFactory);
        auto result=separate.Execute(incoming);
        assert(result.status==-74 && result.durableOrUncertain);
        assert(uncertain.hasIntent);
    }
    std::cout << "SUMMIT350B_JOURNAL=PASS (two-sided apply, intent/first/second/receipt faults, restart recovery, exact-once)\n";
}
