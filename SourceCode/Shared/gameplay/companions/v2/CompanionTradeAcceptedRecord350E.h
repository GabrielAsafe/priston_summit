#pragma once
// SUMMIT 350E: immutable evidence captured at the REAL validated save ingress.
// This evidence is an in-memory snapshot, not permission to write a DAT.
// Never retain rsPLAYINFO*, the legacy receive buffer or a socket here.
#include "gameplay/companions/v2/CompanionTradeSaveActivityFence.h"
#include "gameplay/companions/v2/CompanionTradeSha256.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <map>
#include <mutex>
#include <vector>

namespace gameplay { namespace companions { namespace v2 {

struct AcceptedRecordEvidence350E {
    SaveSessionKey session;
    std::vector<std::uint8_t> bytes;
    TradeSha256Digest digest{};
    std::size_t declaredBytes = 0;
    std::size_t exactWriteBytes = 0;
    std::uint64_t generation = 0;
    // SUMMIT350I: independent evidence from a successful legacy DAT write.
    // A later trade still compares these bytes to the actual on-disk DAT.
    std::vector<std::uint8_t> persistedBytes;
    TradeSha256Digest persistedDigest{};
    std::uint64_t persistedSequence = 0;
};

class AcceptedRecordBridge350E final {
public:
    static const std::size_t kMaxSessions = 256u;
    // Input capacity MUST be the real allocation size from the calling site.
    // This class never infers allocated capacity from a network-controlled size.
    // Called only on the successful, final validated save path, before ACK.
    bool Publish(SaveSessionKey key, const void* data, std::size_t capacity,
                 std::size_t minimumWriteBytes) {
        if(!key.Valid() || !data || capacity<4 || capacity>8u*1024u*1024u ||
           minimumWriteBytes<4 || minimumWriteBytes>capacity) {
            Invalidate(key);
            return false;
        }
        std::int32_t declared=0;
        std::memcpy(&declared,data,4);
        if(declared<=0 || static_cast<std::size_t>(declared)>capacity) {
            Invalidate(key);
            return false;
        }
        const std::size_t written=std::max(static_cast<std::size_t>(declared),minimumWriteBytes);
        if(written>capacity) { Invalidate(key);return false; }
        AcceptedRecordEvidence350E next;
        next.session=key;
        next.declaredBytes=static_cast<std::size_t>(declared);
        next.exactWriteBytes=written;
        const auto* source=static_cast<const std::uint8_t*>(data);
        next.bytes.assign(source,source+written);
        next.digest=TradeSha256::Hash(next.bytes.data(),next.bytes.size());
        std::lock_guard<std::mutex> lock(mutex_);
        if(records_.size()>=kMaxSessions && records_.find(key)==records_.end()) {
            auto oldest=records_.begin();
            for(auto it=records_.begin();it!=records_.end();++it)
                if(it->second.generation<oldest->second.generation) oldest=it;
            records_.erase(oldest);
        }
        ++version_;
        if(!version_) ++version_;
        next.generation=version_;
        records_[key]=std::move(next);
        return true;
    }
    // Called ONLY after fwrite and fclose both succeeded in the actual
    // rsRecordMemoryBuffToFile path, while its flush scope is active. We
    // reject unrelated changes outside the only permitted GameSaveInfo
    // subrecord; an older or untracked save must never authorize a trade.
    bool PublishPersisted(SaveSessionKey key, std::uint64_t acceptedSequence,
                          const void* data, std::size_t bytes,
                          std::size_t allowedOffset, std::size_t allowedSize) {
        if(!key.Valid() || !acceptedSequence || !data || !bytes ||
           bytes>8u*1024u*1024u || allowedOffset>bytes ||
           allowedSize>bytes-allowedOffset) return false;
        const auto* source=static_cast<const std::uint8_t*>(data);
        std::lock_guard<std::mutex> lock(mutex_);
        auto it=records_.find(key);
        if(it==records_.end()) return false;
        AcceptedRecordEvidence350E& previous=it->second;
        previous.persistedBytes.clear();
        previous.persistedSequence=0;
        if(previous.bytes.size()!=bytes || previous.exactWriteBytes!=bytes ||
           !std::equal(previous.bytes.begin(),
                       previous.bytes.begin()+allowedOffset,source) ||
           !std::equal(previous.bytes.begin()+allowedOffset+allowedSize,
                       previous.bytes.end(),source+allowedOffset+allowedSize))
            return false;
        previous.persistedBytes.assign(source,source+bytes);
        previous.persistedDigest=TradeSha256::Hash(
            previous.persistedBytes.data(),previous.persistedBytes.size());
        previous.persistedSequence=acceptedSequence;
        return true;
    }

    // Safe to call under a SaveActivityFence reservation. The caller still
    // MUST compare the accepted save sequence, session and live inventory.
    bool Copy(SaveSessionKey key,AcceptedRecordEvidence350E& result) const {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto it=records_.find(key);
        if(it==records_.end()) return false;
        const auto& stored=it->second;
        if(stored.bytes.empty() || stored.bytes.size()!=stored.exactWriteBytes ||
           TradeSha256::Hash(stored.bytes.data(),stored.bytes.size())!=stored.digest)
            return false;
        if(stored.persistedSequence &&
           (stored.persistedBytes.size()!=stored.exactWriteBytes ||
            TradeSha256::Hash(stored.persistedBytes.data(),
                              stored.persistedBytes.size())!=stored.persistedDigest))
            return false;
        result=stored;
        return true;
    }
    void Invalidate(SaveSessionKey key) {
        if(!key.Valid()) return;
        std::lock_guard<std::mutex> lock(mutex_);
        records_.erase(key);
    }
    std::size_t Size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return records_.size();
    }
private:
    mutable std::mutex mutex_;
    std::map<SaveSessionKey,AcceptedRecordEvidence350E> records_;
    std::uint64_t version_=0;
};

inline AcceptedRecordBridge350E& ServerAcceptedRecordBridge350E() {
    static AcceptedRecordBridge350E bridge;
    return bridge;
}

}}} // namespace
