#include "gameplay/companions/v2/CompanionTradeNormalizedRecord350D.h"
#include "gameplay/companions/v2/CompanionTradeAdmission350C.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>
using namespace gameplay::companions::v2;
using namespace gameplay::companions;

static void write32(std::vector<std::uint8_t>& b, std::int32_t v) {
    std::memcpy(b.data(),&v,4);
}
int main() {
    // Synthetic fixture with independently allocated 128B, variable DAT write
    // length of max(60, 80), GameSave at [16,32), inventory [48,60).
    std::vector<std::uint8_t> live(128,0x3a);
    write32(live,60);
    TradeLegacyRecordLayout350D layout{};
    layout.declaredSizeOffset=0;
    layout.minimumWriteBytes=80;
    layout.gameSaveOffset=16;
    layout.gameSaveBytes=16;
    layout.inventoryEndOffset=60;
    TradeNormalizedRecord350D captured;
    const auto normalize=[](std::uint8_t* bytes,std::size_t length)->bool {
        if(length!=16) return false;
        bytes[0]=0xa5; bytes[15]=0x5a; return true;
    };
    assert(BuildNormalizedRecord350D(live.data(),live.size(),layout,normalize,captured)==
           TradeRecordNormalizeCode350D::Ready);
    assert(captured.declaredBytes==60 && captured.exactWriteBytes==80);
    assert(captured.bytes.size()==80 && captured.bytes[16]==0xa5 &&
           captured.bytes[31]==0x5a && captured.bytes[48]==0x3a);
    const auto digest=captured.digest;
    live[16]=0;live[48]=0; // detached bytes must not follow source mutations
    assert(captured.bytes[16]==0xa5 && captured.bytes[48]==0x3a);
    assert(digest==TradeSha256::Hash(captured.bytes.data(),captured.bytes.size()));
    const auto prior=captured.bytes;
    assert(BuildNormalizedRecord350D(nullptr,128,layout,normalize,captured)==
           TradeRecordNormalizeCode350D::MissingSource);
    assert(BuildNormalizedRecord350D(live.data(),3,layout,normalize,captured)==
           TradeRecordNormalizeCode350D::UnprovenCapacity);
    assert(BuildNormalizedRecord350D(live.data(),64,layout,normalize,captured)==
           TradeRecordNormalizeCode350D::InvalidLayout);
    assert(captured.bytes==prior);
    layout.gameSaveBytes=0;
    assert(BuildNormalizedRecord350D(live.data(),128,layout,normalize,captured)==
           TradeRecordNormalizeCode350D::InvalidLayout);
    layout.gameSaveBytes=16;
    write32(live,129);
    assert(BuildNormalizedRecord350D(live.data(),128,layout,normalize,captured)==
           TradeRecordNormalizeCode350D::InvalidDeclaredSize);
    write32(live,59);
    assert(BuildNormalizedRecord350D(live.data(),128,layout,normalize,captured)==
           TradeRecordNormalizeCode350D::InvalidDeclaredSize);
    write32(live,60);
    assert(BuildNormalizedRecord350D(live.data(),128,layout,
        [](std::uint8_t*,std::size_t){return false;},captured)==
           TradeRecordNormalizeCode350D::FailedServerNormalization);
    assert(captured.bytes==prior);
    // Explicit outside-region writes must be detected; don't let a bad
    // server adapter mutate unrelated state in the authoritative save image.
    assert(BuildNormalizedRecord350D(live.data(),128,layout,
        [](std::uint8_t* buf,std::size_t){buf[-1]=0;return true;},captured)==
           TradeRecordNormalizeCode350D::InventoryModified);
    assert(captured.bytes==prior);
    // The resulting normalized bytes are consumable by 350C as a detached
    // save image AFTER a separate receipt/lease producer validates identity.
    protocol::CompanionTradeConfirmPacket packet{};
    packet.size=sizeof(packet);packet.code=protocol::OPCODE_COMPANION_TRADE_CONFIRM;
    packet.version=protocol::COMPANION_TRADE_CONFIRM_VERSION;
    packet.side=0;packet.itemIndex=1;packet.itemCode=123;
    packet.itemKey=234;packet.itemChecksum=345;
    packet.ownerRevision=3;packet.companionRevision=4;packet.requestId=99;
    std::strcpy(packet.characterName,"companion");
    AcceptedTradeSource350C src;
    src.account="account";src.owner="owner";src.companion="companion";
    src.session.serial=34;src.session.generation=23;
    src.session.account=13;
    src.session.owner=TradeParticipantId("account","owner");
    src.session.companion=TradeParticipantId("account","companion");
    src.acceptedSaveSequence=7;src.leaseToken=8;src.tradeNonce=9;
    src.completeRecord=captured.bytes.data();
    src.allocatedRecordBytes=captured.bytes.size();
    src.acknowledgedRecordBytes=captured.bytes.size();
    src.acknowledgedDigest=captured.digest;
    DetachedTradeJob job;
    assert(CaptureAcceptedTrade350C(src,packet,job)==TradeCaptureCode350C::Captured);
    assert(job.acceptedOwnerRecord==captured.bytes);
    // A race between digest establishment and capture must be rejected.
    ++captured.bytes[48];
    assert(CaptureAcceptedTrade350C(src,packet,job)==TradeCaptureCode350C::DigestMismatch);
    std::puts("SUMMIT350D_NATIVE=PASS (exact-length normalized snapshot, immutable copy, invalid sizes, digest)");
}
