#include "gameplay/companions/v2/CompanionTradeAdmission350C.h"
#include "gameplay/companions/v2/CompanionTradeResultDispatch350C.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>
using namespace gameplay::companions::v2;
using namespace gameplay::companions;
int main(){
    protocol::CompanionTradeConfirmPacket packet{};
    packet.size=sizeof(packet);packet.code=protocol::OPCODE_COMPANION_TRADE_CONFIRM;
    packet.version=protocol::COMPANION_TRADE_CONFIRM_VERSION;
    packet.side=0; packet.itemIndex=2;packet.itemCode=123;packet.itemKey=456;
    packet.itemChecksum=789; packet.ownerRevision=1;packet.companionRevision=2;
    packet.requestId=10;std::strcpy(packet.characterName,"offline");
    const std::uint8_t original[]={1,2,3,4,5,6,7,8};
    std::vector<std::uint8_t> save(original,original+8);
    AcceptedTradeSource350C src;
    src.account="account";src.owner="online";src.companion="offline";
    src.session.serial=101;src.session.generation=202;
    src.session.account=800;src.session.owner=TradeParticipantId("account","online");
    src.session.companion=TradeParticipantId("account","offline");
    src.acceptedSaveSequence=11;src.leaseToken=12;src.tradeNonce=13;
    src.completeRecord=save.data();src.allocatedRecordBytes=save.size();
    src.acknowledgedRecordBytes=save.size();
    src.acknowledgedDigest=TradeSha256::Hash(save.data(),save.size());
    DetachedTradeJob job;
    assert(CaptureAcceptedTrade350C(src,packet,job)==TradeCaptureCode350C::Captured);
    save[0]=99;src.account="changed";packet.itemKey=999;
    assert(job.acceptedOwnerRecord[0]==1 && job.accountName=="account" && job.request.itemKey==456);
    src.account="account";packet.itemKey=456;
    assert(CaptureAcceptedTrade350C(src,packet,job)==TradeCaptureCode350C::DigestMismatch);
    save[0]=1;
    src.allocatedRecordBytes=5;
    assert(CaptureAcceptedTrade350C(src,packet,job)==TradeCaptureCode350C::IncorrectLength);
    src.allocatedRecordBytes=8;
    src.acceptedSaveSequence=0;
    assert(CaptureAcceptedTrade350C(src,packet,job)==TradeCaptureCode350C::UntrustedSave);
    src.acceptedSaveSequence=11;
    src.session.generation=0; // first accepted legacy connection is valid
    assert(CaptureAcceptedTrade350C(src,packet,job)==TradeCaptureCode350C::Captured);
    assert(job.session.generation==0 && job.session.serial==101);
    ResolvedTradeSession350C first{};
    first.session=job.session;first.account=job.accountName;
    first.owner=job.ownerName;first.companion=job.companionName;
    first.connected=true;first.staleSaveWritesSealed=true;
    assert(DecideReply350C(job,2,first)==TradeReplyDecision350C::SendCommitted);
    first.session.generation=1;
    assert(DecideReply350C(job,2,first)==TradeReplyDecision350C::Drop);
    src.session.serial=0;
    assert(CaptureAcceptedTrade350C(src,packet,job)==TradeCaptureCode350C::InvalidSession);
    src.session.serial=101;src.session.generation=202;
    packet.characterName[0]='x';
    assert(CaptureAcceptedTrade350C(src,packet,job)==TradeCaptureCode350C::InvalidIdentity);
    std::strcpy(packet.characterName,"offline");
    assert(CaptureAcceptedTrade350C(src,packet,job)==TradeCaptureCode350C::Captured);
    TradeParticipantGate350B gate;
    DetachedTradeJobQueue queue(1);
    assert(AdmitCapturedTrade350C(gate,queue,src,packet)==DetachedAdmission::Accepted);
    assert(gate.IsReserved(job.session.owner));
    assert(AdmitCapturedTrade350C(gate,queue,src,packet)==DetachedAdmission::BusyCharacter);
    DetachedTradeJob queued;
    assert(queue.TryNext(queued) && queued.acceptedOwnerRecord==job.acceptedOwnerRecord);
    assert(queue.Complete(queued,-60));
    int status=0;
    assert(queue.TryTakeCompletion(queued,status) && status==-60);
    assert(gate.ReleaseVerified(job.session.owner,job.session.companion));
    assert(!gate.IsReserved(job.session.owner));
    ResolvedTradeSession350C live{};
    live.session=job.session;live.account=job.accountName;live.owner=job.ownerName;
    live.companion=job.companionName;live.connected=true;
    assert(DecideReply350C(job,2,live)==TradeReplyDecision350C::Drop);
    live.staleSaveWritesSealed=true;
    assert(DecideReply350C(job,2,live)==TradeReplyDecision350C::SendCommitted);
    assert(DecideReply350C(job,-74,live)==TradeReplyDecision350C::SendRecoveryRequired);
    assert(DecideReply350C(job,-60,live)==TradeReplyDecision350C::SendRefusal);
    assert(DecideReply350C(job,-75,live)==TradeReplyDecision350C::Drop);
    ++live.session.generation;
    assert(DecideReply350C(job,2,live)==TradeReplyDecision350C::Drop);
    --live.session.generation;
    live.companion="other";
    assert(DecideReply350C(job,2,live)==TradeReplyDecision350C::Drop);
    std::puts("SUMMIT350C_NATIVE=PASS (bounded immutable record, digest, session, quarantine)");
}
