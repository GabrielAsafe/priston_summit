#pragma once
// Wire delivery for an ALREADY committed inventory image, limited to the
// authenticated requesting client. A packet is never permission to mint items:
// the server must seal the pair journal BEFORE sending any chunk.
#include "gameplay/companions/v2/CompanionTradeSha256.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace gameplay { namespace companions { namespace v2 {

static const std::size_t kTradeDeliveryChunkData=3000;
static const std::size_t kTradeDeliveryMaxRecord=65536;
struct TradeDeliveryChunk {
    std::int32_t size,code,version;
    std::uint64_t requestId,txnId;
    std::uint32_t total,offset,length;
    std::uint8_t sha256[32], beforeInventorySha256[32];
    char player[32],companion[32];
    std::uint8_t data[kTradeDeliveryChunkData];
};

enum class TradeDeliveryPacketStatus {Incomplete,Complete,Rejected};
class TradeDeliveryReceiver final {
public:
    TradeDeliveryPacketStatus Accept(const TradeDeliveryChunk& p,
        std::int32_t opcode,std::int32_t version,std::uint64_t requestId,
        const char* authenticatedPlayer,const char* selectedCompanion,
        std::vector<std::uint8_t>& complete) {
        if(state_==2 || !requestId || !authenticatedPlayer || !selectedCompanion ||
           !authenticatedPlayer[0] || !selectedCompanion[0] ||
           p.size!=static_cast<std::int32_t>(sizeof(TradeDeliveryChunk)) ||
           p.code!=opcode || p.version!=version || p.requestId!=requestId ||
           !p.txnId || !p.total || p.total>kTradeDeliveryMaxRecord ||
           !p.length || p.length>kTradeDeliveryChunkData ||
           p.offset>p.total || p.length>p.total-p.offset ||
           std::memchr(p.player,0,sizeof(p.player))==nullptr ||
           std::memchr(p.companion,0,sizeof(p.companion))==nullptr ||
           std::strcmp(p.player,authenticatedPlayer)!=0 ||
           std::strcmp(p.companion,selectedCompanion)!=0) return Reject();
        if(state_==0) {
            if(p.offset!=0) return Reject();
            total_=p.total; txnId_=p.txnId;
            std::copy(p.sha256,p.sha256+32,hash_.begin());
            std::copy(p.beforeInventorySha256,p.beforeInventorySha256+32,before_.begin());
            image_.reserve(total_);state_=1;
        }
        if(p.total!=total_ || p.txnId!=txnId_ ||
           !std::equal(p.sha256,p.sha256+32,hash_.begin()) ||
           !std::equal(p.beforeInventorySha256,p.beforeInventorySha256+32,before_.begin()) ||
           p.offset!=image_.size()) return Reject();
        image_.insert(image_.end(),p.data,p.data+p.length);
        if(image_.size()!=total_) return TradeDeliveryPacketStatus::Incomplete;
        if(TradeSha256::Hash(image_.data(),image_.size())!=hash_) return Reject();
        complete=image_;state_=2;
        return TradeDeliveryPacketStatus::Complete;
    }
    void Reset(){state_=0;total_=0;txnId_=0;image_.clear();hash_.fill(0);before_.fill(0);}
    std::uint64_t TransactionId() const {return txnId_;}
    const std::array<std::uint8_t,32>& BeforeInventoryDigest() const{return before_;}
private:
    TradeDeliveryPacketStatus Reject(){Reset();return TradeDeliveryPacketStatus::Rejected;}
    int state_=0;
    std::uint32_t total_=0;
    std::uint64_t txnId_=0;
    std::array<std::uint8_t,32> hash_{},before_{};
    std::vector<std::uint8_t> image_;
};

template<class Send>
bool SendCommittedImage(const std::vector<std::uint8_t>& image,
    std::int32_t opcode,std::int32_t version,
    std::uint64_t requestId,std::uint64_t txnId,
    const std::array<std::uint8_t,32>& beforeInventoryDigest,
    const char* authenticatedPlayer,const char* companion,
    Send send) {
    if(image.empty() || image.size()>kTradeDeliveryMaxRecord ||
       !requestId || !txnId || !authenticatedPlayer || !companion ||
       std::strlen(authenticatedPlayer)>=32 || std::strlen(companion)>=32)
        return false;
    const auto hash=TradeSha256::Hash(image.data(),image.size());
    for(std::size_t offset=0;offset<image.size();offset+=kTradeDeliveryChunkData) {
        TradeDeliveryChunk p{};
        p.size=static_cast<std::int32_t>(sizeof(p));p.code=opcode;p.version=version;
        p.requestId=requestId;p.txnId=txnId;
        p.total=static_cast<std::uint32_t>(image.size());
        p.offset=static_cast<std::uint32_t>(offset);
        p.length=static_cast<std::uint32_t>(std::min(kTradeDeliveryChunkData,image.size()-offset));
        std::copy(hash.begin(),hash.end(),p.sha256);
        std::copy(beforeInventoryDigest.begin(),beforeInventoryDigest.end(),p.beforeInventorySha256);
        std::memcpy(p.player,authenticatedPlayer,std::strlen(authenticatedPlayer));
        std::memcpy(p.companion,companion,std::strlen(companion));
        std::memcpy(p.data,image.data()+offset,p.length);
        if(!send(p)) return false;
    }
    return true;
}
}}}
