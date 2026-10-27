// SUMMIT350I: tests actual POSIX journal/fsync and exact DAT replacement
// with DISPOSABLE opaque bytes, never real player records.
#include "gameplay/companions/v2/CompanionTradeDiskJournal.h"
#include "gameplay/companions/v2/CompanionTradeExactPairBackend.h"
#include "gameplay/companions/v2/CompanionTradeExactRecordStore.h"
#include "gameplay/companions/v2/CompanionTradeJournal.h"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>
using namespace gameplay::companions::v2;
using Bytes=std::vector<std::uint8_t>;
static void Put(const std::string& p,const Bytes& b) {
    std::ofstream f(p,std::ios::binary|std::ios::trunc);
    assert(f.good());f.write(reinterpret_cast<const char*>(b.data()),b.size());
    f.flush();assert(f.good());f.close();assert(!f.fail());
}
static Bytes Get(const std::string& p) {
    Bytes out;assert(CompanionTradeExactRecordStore::Read(p,out));return out;
}
static Bytes B(const char* x) {return Bytes(x,x+std::char_traits<char>::length(x));}
struct Host: ICompanionTradeExactPairHost {
    CompanionTradePrepared plan;
    std::string a,b;
    bool acquired=false;
    bool interruptAfterCompanion=false;
    bool allow=true;
    Host(const CompanionTradePrepared& p,std::string x,std::string y):plan(p),a(std::move(x)),b(std::move(y)){}
    bool AcquireExternalLease(std::uint64_t owner,std::uint64_t companion) override {
        if(!allow || acquired || owner!=plan.offer.authenticatedOwner ||
           companion!=plan.offer.companionCharacter)return false;
        acquired=true;return true;
    }
    void ReleaseExternalLease(std::uint64_t,std::uint64_t) override {acquired=false;}
    bool StillOwnsExternalLease(std::uint64_t owner,std::uint64_t companion) override {
        return acquired && allow && owner==plan.offer.authenticatedOwner &&
               companion==plan.offer.companionCharacter &&
               !(interruptAfterCompanion && Get(b)==plan.companionAfter);
    }
    bool TrustedPathsUnderLease(std::uint64_t,std::uint64_t,std::string& x,std::string& y) override {
        if(!acquired) return false;
        x=a; y=b; return true;
    }
    bool BuildPreparedUnderLease(const CompanionTradeOffer& offer, CompanionTradePrepared& p) override {
        if(!acquired || !SameOffer(offer,plan.offer))return false;
        p=plan;return true;
    }
};
int main(int argc,char**argv) {
    assert(argc==2);
    const std::string dir=argv[1];
    const std::string journalRoot=dir+"/journal";
    assert(mkdir(journalRoot.c_str(),0700)==0);
    const std::string owner=dir+"/owner.dat",companion=dir+"/companion.dat";
    CompanionTradePrepared p;
    p.offer.authenticatedOwner=12345;p.offer.companionCharacter=67890;
    p.offer.tradeNonce=424242;p.offer.offerDigest=8998;p.offer.offerRevision=7;
    p.ownerBefore=B("OWNER_RECORD_PRE_TRANSFER_UNIQUE_ITEM_A");
    p.ownerAfter=B("OWNER_RECORD_AFTER_ITEM_REMOVED_A");
    p.companionBefore=B("COMPANION_RECORD_BEFORE_NO_ITEM_A");
    p.companionAfter=B("COMPANION_RECORD_AFTER_INSERTED_A");
    Put(owner,p.ownerBefore);Put(companion,p.companionBefore);
    // Simulate a process dying after the first physical DAT replacement.
    {
        Host host(p,owner,companion);host.interruptAfterCompanion=true;
        CompanionTradeExactPairBackend backend(host);
        CompanionTradeDiskJournal disk(journalRoot,backend);
        assert(disk.Healthy());
        CompanionTradeJournalCoordinator coordinator(disk);
        assert(coordinator.Commit(p.offer,true)==TradeJournalResult::RecoveryRequired);
        assert(Get(owner)==p.ownerBefore);
        assert(Get(companion)==p.companionAfter);
        assert(disk.PendingForPair(12345,67890).present);
    }
    // New in-memory backend/journal instance, same REAL files on disk.
    {
        Host host(p,owner,companion);
        CompanionTradeExactPairBackend backend(host);
        CompanionTradeDiskJournal disk(journalRoot,backend);
        CompanionTradeJournalCoordinator coordinator(disk);
        assert(coordinator.RecoverPair(12345,67890)==TradeJournalResult::Committed);
        assert(Get(owner)==p.ownerAfter && Get(companion)==p.companionAfter);
        const auto receipt=disk.Receipt(p.offer.tradeNonce);
        assert(receipt.found && receipt.committed);
        assert(!disk.PendingForPair(12345,67890).present);
        // Exact-once request replay cannot mutate the completed images.
        assert(coordinator.Commit(p.offer,true)==TradeJournalResult::AlreadyCommitted);
        assert(Get(owner)==p.ownerAfter && Get(companion)==p.companionAfter);
        CompanionTradePrepared proof;
        assert(disk.CompletedPlan(p.offer.tradeNonce,proof));
        assert(proof.ownerAfter==p.ownerAfter && proof.companionAfter==p.companionAfter);
    }
    // If an unrelated writer changed the source, never publish a new intent.
    p.offer.tradeNonce=424243;
    p.ownerBefore=B("MISMATCHED_STALE_OWNER_RECORD");
    p.companionBefore=B("MISMATCHED_STALE_COMPANION_RECORD");
    {
        Host host(p,owner,companion);
        CompanionTradeExactPairBackend backend(host);
        CompanionTradeDiskJournal disk(journalRoot,backend);
        CompanionTradeJournalCoordinator coordinator(disk);
        assert(coordinator.Commit(p.offer,true)==TradeJournalResult::Rejected);
        assert(!disk.PendingForPair(12345,67890).present);
        assert(Get(owner)!=p.ownerBefore);
    }
    std::cout << "SUMMIT350I_REAL_DISK=PASS (partial DAT, restart recovery, receipt, exact-once, stale baseline)\n";
}
