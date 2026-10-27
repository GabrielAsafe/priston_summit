#pragma once
// First post-commit save must be identical in inventory to the durable journal
// after-image. All older, delayed saves are refused before legacy validation
// mutates the session or writes a DAT. This is NOT a journal or permission to
// transfer items: only an additional fail-closed replay barrier.
#include "gameplay/companions/v2/CompanionTradeSaveActivityFence.h"
#include "gameplay/companions/v2/CompanionTradeCanonicalInventoryDigest.h"
#include <map>
#include <mutex>
#include <vector>
namespace gameplay { namespace companions { namespace v2 {
class TradeLiveFirstSave350Q final {
public:
    bool Arm(SaveSessionKey key, const TradeSha256Digest& digest) {
        if(!key.Valid() || !HasDigest(digest)) return false;
        std::lock_guard<std::mutex> lock(mutex_);
        if(expected_.count(key)) return false;
        return expected_.emplace(key,digest).second;
    }
    // Called on the assembled complete record before ANY legacy record side effects.
    bool CheckOrReject(SaveSessionKey key,const std::uint8_t* record,std::size_t bytes,
                       const BufferedTradeEnvelopeLayout& layout) {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto it=expected_.find(key);
        if(it==expected_.end()) return true;
        if(!record || !bytes || bytes>layout.maxBytes) return false;
        const std::vector<std::uint8_t> owned(record,record+bytes);
        TradeSha256Digest actual{};
        if(!TradeCanonicalInventoryDigest::Compute(owned,layout,actual) ||
           actual!=it->second) return false;
        return true;
    }
    // Call only after the complete record has been validated and accepted;
    // a failed save must NOT consume the stale-save replay barrier.
    void MarkAccepted(SaveSessionKey key) {
        std::lock_guard<std::mutex> lock(mutex_);expected_.erase(key);
    }
    bool Pending(SaveSessionKey key) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return expected_.count(key)!=0;
    }
    void Forget(SaveSessionKey key) {
        std::lock_guard<std::mutex> lock(mutex_); expected_.erase(key);
    }
private:
    static bool HasDigest(const TradeSha256Digest& h) {
        unsigned char v=0;for(const auto b:h)v|=b;return v!=0;
    }
    mutable std::mutex mutex_;
    std::map<SaveSessionKey,TradeSha256Digest> expected_;
};
inline TradeLiveFirstSave350Q& ServerTradeLiveFirstSave350Q() {
    static TradeLiveFirstSave350Q gate;return gate;
}
}}}
