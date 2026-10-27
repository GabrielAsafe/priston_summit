#pragma once
// Canonical, domain-separated SHA-256 for an inventory inside a complete
// legacy character record. This is NOT a full-record digest, a journal,
// a signature, or proof of server acceptance. Use it to reconcile a fresh
// authenticated save after delivery of a journaled trade.
#include "gameplay/companions/v2/CompanionTradeBufferedPayloadCandidate.h"
#include "gameplay/companions/v2/CompanionTradeSha256.h"
#include <vector>
#include <cstdint>
#include <cstring>
namespace gameplay { namespace companions { namespace v2 {
struct TradeCanonicalInventoryDigest {
    static bool Compute(const std::vector<std::uint8_t>& bytes,
        const BufferedTradeEnvelopeLayout& layout, TradeSha256Digest& result) {
        std::int32_t count=0, primary=0, size=0, recordSize=0;
        if(!Read(bytes,layout.itemCountOffset,count) || !Read(bytes,layout.primaryOffset,primary) ||
            !Read(bytes,layout.dataSizeOffset,size) || !Read(bytes,layout.sizeOffset,recordSize) ||
            count<0 || primary<0 || primary>count || size<0 || recordSize<0 ||
            layout.dataOffset>bytes.size() || layout.maxBytes<bytes.size() ||
            static_cast<std::size_t>(size)>bytes.size()-layout.dataOffset ||
            static_cast<std::size_t>(recordSize)>bytes.size() ||
            static_cast<std::size_t>(recordSize)<layout.dataOffset+static_cast<std::size_t>(size))
            return false;
        std::vector<std::uint8_t> canonical;
        canonical.reserve(32+static_cast<std::size_t>(size));
        const char tag[]="SUMMIT-TRADE-INVENTORY-V1";
        canonical.insert(canonical.end(),tag,tag+sizeof(tag));
        AddLE32(canonical,static_cast<std::uint32_t>(count));
        AddLE32(canonical,static_cast<std::uint32_t>(primary));
        AddLE32(canonical,static_cast<std::uint32_t>(size));
        canonical.insert(canonical.end(), bytes.begin()+layout.dataOffset,
                         bytes.begin()+layout.dataOffset+static_cast<std::size_t>(size));
        result=TradeSha256::Hash(canonical.data(),canonical.size());
        return true;
    }
private:
    static bool Read(const std::vector<std::uint8_t>& v,std::size_t p,std::int32_t& out) {
        if(p>v.size() || v.size()-p<4) return false;
        std::memcpy(&out,v.data()+p,4);return true;
    }
    static void AddLE32(std::vector<std::uint8_t>& v,std::uint32_t value) {
        for(int i=0;i<4;++i) v.push_back(static_cast<std::uint8_t>(value>>(8*i)));
    }
};
}}}
