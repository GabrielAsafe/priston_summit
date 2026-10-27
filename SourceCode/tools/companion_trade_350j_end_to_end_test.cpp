// SUMMIT350J native disk regression: old DAT -> accepted online save ->
// journalled item move -> crash recovery, using ONLY synthetic disposable data.
#include "gameplay/companions/v2/CompanionTradeBaseline350J.h"
#include "gameplay/companions/v2/CompanionTradeDiskJournal.h"
#include "gameplay/companions/v2/CompanionTradeExactPairBackend.h"
#include "gameplay/companions/v2/CompanionTradeJournal.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sys/stat.h>
using namespace gameplay::companions::v2;
using Bytes=std::vector<std::uint8_t>;
static Bytes B(const std::string& x) {return Bytes(x.begin(),x.end());}
static void Put(const std::string& path,const Bytes& b) {
    std::ofstream f(path,std::ios::binary|std::ios::trunc);
    assert(f.good()); f.write(reinterpret_cast<const char*>(b.data()),b.size());
    f.close(); assert(!f.fail());
}
static Bytes Get(const std::string& p) {
    Bytes r;assert(CompanionTradeExactRecordStore::Read(p,r));return r;
}
class TradeHost: public ICompanionTradeExactPairHost {
public:
    std::string owner,companion;
    CompanionTradePrepared plan;
    bool locked=false,permission=true,interrupt=false,uncertain=false;
    TradeHost(const std::string& a,const std::string& b,const CompanionTradePrepared& p)
        :owner(a),companion(b),plan(p) {}
    bool AcquireExternalLease(std::uint64_t o,std::uint64_t c) override {
        if(locked || o!=plan.offer.authenticatedOwner ||
           c!=plan.offer.companionCharacter)return false;
        locked=true;return true;
    }
    void ReleaseExternalLease(std::uint64_t,std::uint64_t) override {locked=false;}
    bool StillOwnsExternalLease(std::uint64_t o,std::uint64_t c) override {
        return locked && o==plan.offer.authenticatedOwner &&
            c==plan.offer.companionCharacter &&
            !(interrupt && Get(companion)==plan.companionAfter);
    }
    bool TrustedPathsUnderLease(std::uint64_t,std::uint64_t,
                                std::string& a,std::string& b) override {
        if(!locked) return false;
        a=owner; b=companion; return true;
    }
    bool BuildPreparedUnderLease(const CompanionTradeOffer& offer,
                                 CompanionTradePrepared& result) override {
        // The runtime validates both inventory images BEFORE this save step.
        if(!locked || !permission || !SameOffer(plan.offer,offer))return false;
        auto baseline=Get(owner);
        const auto status=MaterializeAcceptedOwner350J(owner,baseline,plan.ownerBefore);
        if(status==AcceptedOwnerBaseline350J::Uncertain) uncertain=true;
        if(status!=AcceptedOwnerBaseline350J::Ready)return false;
        result=plan;return true;
    }
};
static CompanionTradePrepared Plan(std::uint64_t nonce) {
    CompanionTradePrepared p{};
    p.offer.authenticatedOwner=1001;
    p.offer.companionCharacter=2002;
    p.offer.tradeNonce=nonce;
    p.offer.offerDigest=5;
    p.offer.offerRevision=6;
    p.ownerBefore=B("Player:slotA0=ITEM_ABC;slotB1=EMPTY;level=22");
    p.ownerAfter=B("Player:slotA0=EMPTY;slotB1=EMPTY;level=22");
    p.companionBefore=B("Companion:slotA0=EMPTY;slotB1=EMPTY;level=9");
    p.companionAfter=B("Companion:slotA0=EMPTY;slotB1=ITEM_ABC;level=9");
    return p;
}
int main(int argc,char**argv) {
    assert(argc==2);
    const std::string dir=argv[1],owner=dir+"/owner.dat",companion=dir+"/companion.dat";
    const std::string journal=dir+"/journal";
    assert(mkdir(journal.c_str(),0700)==0);
    auto p=Plan(1357901);
    Put(owner,B("Player:slotA0=ITEM_ABC;slotB1=EMPTY;level=21"));
    Put(companion,p.companionBefore);
    // If the selected item / target slot was invalid, NO preflight write.
    {
        TradeHost host(owner,companion,p);host.permission=false;
        CompanionTradeExactPairBackend backend(host);
        CompanionTradeDiskJournal disk(journal,backend);
        CompanionTradeJournalCoordinator coordinator(disk);
        assert(coordinator.Commit(p.offer,true)==TradeJournalResult::Rejected);
        assert(Get(owner)!=p.ownerBefore);
    }
    // A valid operation writes the accepted save first. Simulate losing power
    // between companion and player journal applies, then resume next boot.
    {
        TradeHost host(owner,companion,p);host.interrupt=true;
        CompanionTradeExactPairBackend backend(host);
        CompanionTradeDiskJournal disk(journal,backend);
        CompanionTradeJournalCoordinator coordinator(disk);
        assert(coordinator.Commit(p.offer,true)==TradeJournalResult::RecoveryRequired);
        assert(Get(owner)==p.ownerBefore);
        assert(Get(companion)==p.companionAfter);
        assert(disk.PendingForPair(1001,2002).present);
    }
    {
        TradeHost host(owner,companion,p);
        CompanionTradeExactPairBackend backend(host);
        CompanionTradeDiskJournal disk(journal,backend);
        CompanionTradeJournalCoordinator coordinator(disk);
        assert(coordinator.RecoverPair(1001,2002)==TradeJournalResult::Committed);
        assert(Get(owner)==p.ownerAfter);
        assert(Get(companion)==p.companionAfter);
        assert(disk.Receipt(p.offer.tradeNonce).committed);
        assert(coordinator.Commit(p.offer,true)==TradeJournalResult::AlreadyCommitted);
    }
    std::cout<<"SUMMIT350J_DISK=PASS (buffered save materialised; item conserved; crash recovery; receipt; no replay)\n";
}
