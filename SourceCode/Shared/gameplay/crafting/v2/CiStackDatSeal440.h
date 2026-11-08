#pragma once
// SUMMIT 440A — versioned CI stack metadata stored in native sITEMINFO bytes.
// This is a CODEC, NOT a gameplay integration. Server-only HMAC key is provided
// by the caller; NEVER ship it to the client or put it in Content/SQLite.
// NO live DAT writes; existing legacy items remain quantity 1.
#include "gameplay/companions/v2/CompanionTradeSha256.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace summit_ci440 {
using Byte = std::uint8_t;
using Key = std::array<Byte,32>;
constexpr std::uint32_t kMagic = 0x31534943u; // bytes "CIS1" on little-endian x86
constexpr unsigned kMaxQuantity = 999;
constexpr std::uint32_t kFirstCi = 0x080B8000u; // CI101, verified registry 436A
constexpr std::uint32_t kLastCi = 0x080BC300u;  // CI168, 68 contiguous native codes

inline bool IsNativeCi(std::uint32_t code) {
    return code>=kFirstCi && code<=kLastCi && ((code-kFirstCi)&255u)==0u;
}
inline bool ValidKey(const Key& key) {
    Byte any=0;for(Byte b:key)any|=b;return any!=0;
}
inline void U32(std::vector<Byte>& out,std::uint32_t v) {
    for(unsigned i=0;i<4;++i)out.push_back(static_cast<Byte>(v>>(8*i)));
}
inline gameplay::companions::v2::TradeSha256Digest Hmac(const Key& key,const std::vector<Byte>& message) {
    Key inner{},outer{};
    for(unsigned i=0;i<32;++i){inner[i]=key[i]^0x36u;outer[i]=key[i]^0x5cu;}
    std::vector<Byte> first(64,0x36u),second(64,0x5cu);
    for(unsigned i=0;i<32;++i){first[i]=inner[i];second[i]=outer[i];}
    first.insert(first.end(),message.begin(),message.end());
    auto middle=gameplay::companions::v2::TradeSha256::Hash(first.data(),first.size());
    second.insert(second.end(),middle.begin(),middle.end());
    return gameplay::companions::v2::TradeSha256::Hash(second.data(),second.size());
}
// Explicitly bind the native identity and all significant legacy variant fields.
// Other item attributes must additionally pass legacy CheckItemForm at call sites.
// Use stable DWORDs; do NOT hash compiler padding, pointers, or the seal itself.
template<class Info>
inline std::vector<Byte> Message(const Info& item,std::uint32_t qty) {
    static const Byte domain[]={'S','U','M','M','I','T','-','C','I','-','D','A','T','-','S','T','A','C','K','-','v','1'};
    std::vector<Byte> msg(domain,domain+sizeof(domain));
    U32(msg,static_cast<std::uint32_t>(item.CODE));
    U32(msg,static_cast<std::uint32_t>(item.ItemHeader.Head));
    U32(msg,static_cast<std::uint32_t>(item.ItemHeader.dwChkSum));
    U32(msg,static_cast<std::uint32_t>(item.ItemHeader.dwVersion));
    U32(msg,static_cast<std::uint32_t>(item.ItemHeader.dwTime));
    U32(msg,static_cast<std::uint32_t>(item.dwCreateTime));
    U32(msg,static_cast<std::uint32_t>(item.UniqueItem));
    U32(msg,static_cast<std::uint32_t>(item.ItemKindCode));
    U32(msg,static_cast<std::uint32_t>(item.ItemKindMask));
    U32(msg,static_cast<std::uint32_t>(item.JobCodeMask));
    U32(msg,static_cast<std::uint32_t>(item.BackUpKey));
    U32(msg,static_cast<std::uint32_t>(item.BackUpChkSum));
    U32(msg,qty);
    return msg;
}

enum class Status {
    LegacyOne, VerifiedStack, NotCi, NoKey, InvalidIdentity,
    UnsignedQuantity, BadQuantity, ReservedFieldsOccupied, BadMac,
    NotAuthorised, Sealed
};
struct Reading {Status status;std::uint32_t quantity; Reading(Status s=Status::NotCi,std::uint32_t q=0):status(s),quantity(q){} };

template<class Info>
inline Reading Read(const Info& item,const Key& key) {
    if(!IsNativeCi(static_cast<std::uint32_t>(item.CODE)))return {Status::NotCi,0};
    if(!ValidKey(key))return {Status::NoKey,0};
    if(!item.ItemHeader.Head || !item.ItemHeader.dwChkSum)return {Status::InvalidIdentity,0};
    if(item.dwTemp[0]!=kMagic) {
        for(unsigned i=0;i<9;++i)if(item.dwTemp[i]!=0)return {Status::ReservedFieldsOccupied,0};
        if(item.PotionCount<0 || item.PotionCount>1)return {Status::UnsignedQuantity,0};
        return {Status::LegacyOne,1};
    }
    if(item.PotionCount<1 || item.PotionCount>static_cast<int>(kMaxQuantity))
        return {Status::BadQuantity,0};
    const auto bytes=Message(item,static_cast<std::uint32_t>(item.PotionCount));
    const auto mac=Hmac(key,bytes);
    Byte mismatch=0;
    for(unsigned i=0;i<8;++i) {
        std::uint32_t word=0;
        for(unsigned b=0;b<4;++b)word|=static_cast<std::uint32_t>(mac[i*4+b])<<(8*b);
        const std::uint32_t diff=word ^ static_cast<std::uint32_t>(item.dwTemp[i+1]);
        mismatch|=static_cast<Byte>((diff|static_cast<std::uint32_t>(0-diff))>>24);
    }
    if(mismatch)return {Status::BadMac,0};
    return {Status::VerifiedStack,static_cast<std::uint32_t>(item.PotionCount)};
}

// Host MUST have called CheckItemForm and proven the authenticated server
// inventory/session and exclusive save lease. Flags cannot acquire locks.
// Re-sealing a stack preserves identity; split must mint a distinct native ID.
template<class Info>
inline Status Seal(Info& item,std::uint32_t quantity,const Key& key,
                   bool legacyItemVerified,bool serverOwnsItem,bool leaseHeld) {
    if(!legacyItemVerified||!serverOwnsItem||!leaseHeld)return Status::NotAuthorised;
    if(quantity<1||quantity>kMaxQuantity)return Status::BadQuantity;
    const Reading existing=Read(item,key);
    if(existing.status!=Status::LegacyOne && existing.status!=Status::VerifiedStack)
        return existing.status;
    Info copy=item;
    copy.PotionCount=static_cast<int>(quantity);
    copy.dwTemp[0]=kMagic;
    const auto msg=Message(copy,quantity),mac=Hmac(key,msg);
    for(unsigned i=0;i<8;++i) {
        std::uint32_t word=0;
        for(unsigned b=0;b<4;++b)word|=static_cast<std::uint32_t>(mac[i*4+b])<<(8*b);
        copy.dwTemp[i+1]=word;
    }
    if(Read(copy,key).status!=Status::VerifiedStack)return Status::BadMac;
    item=copy;
    return Status::Sealed;
}
// For the CLIENT UI only. NOT a server-side authority, no HMAC key needed.
// The server must authenticate and publish the instance before it reaches UI.
template<class Info>
inline std::uint32_t UiHint(const Info& item) {
    if(!IsNativeCi(static_cast<std::uint32_t>(item.CODE)))return 0;
    if(item.dwTemp[0]!=kMagic)return 1; // old one-instance CI DAT
    return item.PotionCount>0 && item.PotionCount<=static_cast<int>(kMaxQuantity) ?
        static_cast<std::uint32_t>(item.PotionCount):0;
}
} // namespace summit_ci440
