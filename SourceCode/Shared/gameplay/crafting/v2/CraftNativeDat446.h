#pragma once
// SUMMIT 446: bounded in-memory native DAT after-image constructor.
// Does NOT write disk, acquire a session, create database items or send ACK.
// Host callbacks authenticate recipe, create native output, choose a real slot
// with footprint/collision checks and validate CheckItemForm on every record.
#include "gameplay/crafting/v2/CiStackDatSeal440.h"
#include "gameplay/companions/v2/CompanionTradeBoundedItemCodec.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <set>
#include <tuple>
#include <vector>
#include <memory>

namespace summit446 {
using Byte=std::uint8_t;
using Bytes=std::vector<Byte>;
enum class BuildStatus { Ready, Invalid, BadRecord, Conflict, Missing, Capacity, BadMint };
struct Need {std::uint32_t code,quantity; Need(std::uint32_t c=0,std::uint32_t q=0):code(c),quantity(q){} };
struct IngredientCheck {std::uint32_t code,before,after; IngredientCheck(std::uint32_t c=0,std::uint32_t b=0,std::uint32_t a=0):code(c),before(b),after(a){} };
template<class Record> struct NativeItem {Record item{};Bytes encoded; bool secondary=false;};

inline bool Encode(const Byte* input,std::size_t n,Bytes& output) {
    if(!input||!n||n>65536)return false;
    Bytes candidate(4,0);
    for(std::size_t i=0;i<n;) {
        const bool zero=input[i]==0;
        std::size_t length=1;
        while(i+length<n && length<127 && (input[i+length]==0)==zero)++length;
        candidate.push_back(static_cast<Byte>(length|(zero?128:0)));
        if(!zero)candidate.insert(candidate.end(),input+i,input+i+length);
        i+=length;
    }
    if(candidate.size()>INT32_MAX)return false;
    std::uint32_t len=static_cast<std::uint32_t>(candidate.size());
    for(int i=0;i<4;++i)candidate[i]=static_cast<Byte>(len>>(i*8));
    Bytes decoded(n,0);
    if(gameplay::companions::v2::CompanionTradeBoundedItemCodec::Decode(
         candidate.data(),candidate.size(),decoded.data(),decoded.size())!=
         gameplay::companions::v2::TradeItemDecodeStatus::Ok ||
       std::memcmp(decoded.data(),input,n)!=0)return false;
    output.swap(candidate);return true;
}

// Structured, read-only diagnostic of the FIRST failure in a legacy DAT.
// Never log account, character, path, item keys, checksums, or secret bytes.
enum class DecodeStage454 {
    None, Count, SubStart, DataSize, StoredSize, StoredPayload,
    MissingFrame, FrameLength, CompressedItem, ItemPosition,
    BagOrdinal, ItemCode, ItemHead, ItemChecksum, ItemForm,
    BagSlotConflict, IdentityConflict, TailBytes, OrphanTwoHandMirror
};
inline const char* DecodeStageName454(DecodeStage454 stage) {
    switch(stage) {
    case DecodeStage454::None:return "none";
    case DecodeStage454::Count:return "count";
    case DecodeStage454::SubStart:return "sub_start";
    case DecodeStage454::DataSize:return "data_size";
    case DecodeStage454::StoredSize:return "stored_size";
    case DecodeStage454::StoredPayload:return "stored_payload";
    case DecodeStage454::MissingFrame:return "missing_frame";
    case DecodeStage454::FrameLength:return "frame_length";
    case DecodeStage454::CompressedItem:return "compressed_item";
    case DecodeStage454::ItemPosition:return "item_position";
    case DecodeStage454::BagOrdinal:return "bag_ordinal";
    case DecodeStage454::ItemCode:return "item_code";
    case DecodeStage454::ItemHead:return "item_head";
    case DecodeStage454::ItemChecksum:return "item_checksum";
    case DecodeStage454::ItemForm:return "item_form";
    case DecodeStage454::BagSlotConflict:return "bag_slot_conflict";
    case DecodeStage454::IdentityConflict:return "identity_conflict";
    case DecodeStage454::TailBytes:return "tail_bytes";
    case DecodeStage454::OrphanTwoHandMirror:return "orphan_twohand_mirror";
    }
    return "unknown";
}
struct DecodeDetail454 {
    DecodeStage454 stage=DecodeStage454::None;
    int itemIndex=-1;
    std::size_t offset=0;
    std::size_t frame=0;
    int codecStatus=-1;
    int itemPosition=0;
    int bagOrdinal=0;
};
inline BuildStatus DecodeFail454(DecodeDetail454* detail,DecodeStage454 reason,
                                 int index=-1,std::size_t offset=0,
                                 std::size_t frame=0,int codec=-1,
                                 int position=0,int ordinal=0,
                                 BuildStatus result=BuildStatus::BadRecord) {
    if(detail){
        detail->stage=reason;detail->itemIndex=index;detail->offset=offset;
        detail->frame=frame;detail->codecStatus=codec;
        detail->itemPosition=position;detail->bagOrdinal=ordinal;
    }
    return result;
}
// SUMMIT455A: the legacy SetTwoHandItem() fabricates a DISPLAY-ONLY
// offhand mirror. It has CODE and Price, but no name or native identity.
// It is not an item, must never be debited or reconciled as an owned item,
// and is permitted only when exactly one REAL position-1 item of the same
// CODE exists in the same DAT. Keep its original compressed frame verbatim.
template<class Record>
inline bool IsLegacyTwoHandMirror455(const NativeItem<Record>& e) {
    const auto& info=e.item.sItemInfo;
    return e.item.ItemPosition==2 && info.CODE &&
        !info.ItemHeader.Head && !info.ItemHeader.dwChkSum &&
        !info.ItemName[0];
}

template<class Dat,class Record,class Validate>
BuildStatus Decode(const Dat& data,std::size_t fileBytes,Validate validator,
                   std::vector<NativeItem<Record>>& out,
                   DecodeDetail454* detail=nullptr) {
    if(detail)*detail=DecodeDetail454();
    if(data.ItemCount<0||data.ItemCount>200)
        return DecodeFail454(detail,DecodeStage454::Count);
    if(data.ItemSubStart<0||data.ItemSubStart>data.ItemCount)
        return DecodeFail454(detail,DecodeStage454::SubStart);
    if(data.DataSize<0||static_cast<std::size_t>(data.DataSize)>sizeof(data.Data))
        return DecodeFail454(detail,DecodeStage454::DataSize);
    if(data.size<=0||static_cast<std::size_t>(data.size)>fileBytes)
        return DecodeFail454(detail,DecodeStage454::StoredSize);
    if(static_cast<std::size_t>(data.size)<
       offsetof(Dat,Data)+static_cast<std::size_t>(data.DataSize))
        return DecodeFail454(detail,DecodeStage454::StoredPayload);
    std::vector<NativeItem<Record>> stage;
    std::set<std::tuple<std::uint32_t,std::uint32_t,std::uint32_t>> ids;
    std::set<std::pair<int,int>> slots;
    std::size_t pos=0;
    for(int n=0;n<data.ItemCount;++n) {
        if(pos>static_cast<std::size_t>(data.DataSize)||
           static_cast<std::size_t>(data.DataSize)-pos<4)
            return DecodeFail454(detail,DecodeStage454::MissingFrame,n,pos);
        const Byte* from=data.Data+pos;
        std::uint32_t frame=0;
        for(unsigned k=0;k<4;++k)frame|=static_cast<std::uint32_t>(from[k])<<(8*k);
        if(frame<=4||frame>static_cast<std::size_t>(data.DataSize)-pos)
            return DecodeFail454(detail,DecodeStage454::FrameLength,n,pos,frame);
        NativeItem<Record> entry;
        entry.secondary=n>=data.ItemSubStart;
        entry.encoded.assign(from,from+frame);
        const auto codec=gameplay::companions::v2::CompanionTradeBoundedItemCodec::Decode(
            from,frame,reinterpret_cast<Byte*>(&entry.item),sizeof(Record));
        if(codec!=gameplay::companions::v2::TradeItemDecodeStatus::Ok)
            return DecodeFail454(detail,DecodeStage454::CompressedItem,n,pos,frame,
                                  static_cast<int>(codec));
        const auto& info=entry.item.sItemInfo;
        const int position=entry.item.ItemPosition,ordinal=entry.item.ItemCount;
        if(position<-1||position>16)
            return DecodeFail454(detail,DecodeStage454::ItemPosition,n,pos,frame,-1,position,ordinal);
        if(position==0&&(ordinal<0||ordinal>=100))
            return DecodeFail454(detail,DecodeStage454::BagOrdinal,n,pos,frame,-1,position,ordinal);
        if(!info.CODE)
            return DecodeFail454(detail,DecodeStage454::ItemCode,n,pos,frame,-1,position,ordinal);
        const bool mirror455=IsLegacyTwoHandMirror455(entry);
        // A display mirror is never a bag material and has no item signature.
        // No other unsigned item is accepted by the Craft DAT parser.
        if(mirror455 && (ordinal<0||ordinal>=100))
            return DecodeFail454(detail,DecodeStage454::BagOrdinal,n,pos,frame,-1,position,ordinal);
        if(!info.ItemHeader.Head && !mirror455)
            return DecodeFail454(detail,DecodeStage454::ItemHead,n,pos,frame,-1,position,ordinal);
        if(!info.ItemHeader.dwChkSum && !mirror455)
            return DecodeFail454(detail,DecodeStage454::ItemChecksum,n,pos,frame,-1,position,ordinal);
        if(!mirror455 && !validator(info))
            return DecodeFail454(detail,DecodeStage454::ItemForm,n,pos,frame,-1,position,ordinal);
        if(position==0 && !slots.insert({entry.secondary?2:1,ordinal}).second)
            return DecodeFail454(detail,DecodeStage454::BagSlotConflict,n,pos,frame,-1,position,ordinal,BuildStatus::Conflict);
        if(!mirror455 && !ids.insert(std::make_tuple(static_cast<std::uint32_t>(info.CODE),
                 static_cast<std::uint32_t>(info.ItemHeader.Head),
                 static_cast<std::uint32_t>(info.ItemHeader.dwChkSum))).second)
            return DecodeFail454(detail,DecodeStage454::IdentityConflict,n,pos,frame,-1,position,ordinal,BuildStatus::Conflict);
        stage.push_back(std::move(entry));pos+=frame;
    }
    if(pos!=static_cast<std::size_t>(data.DataSize))
        return DecodeFail454(detail,DecodeStage454::TailBytes,data.ItemCount,pos);
    // A CODE-only record can be a native two-hand display mirror only when
    // exactly one signed/equipped (position 1) item with the SAME code exists.
    // A second mirror claiming that real item, or a mirror with no parent,
    // is malformed: fail CLOSED, do not fabricate or modify the DAT.
    std::set<std::size_t> claimed;
    for(std::size_t n=0;n<stage.size();++n) {
        const auto& ghost=stage[n];
        if(!IsLegacyTwoHandMirror455(ghost))continue;
        std::size_t parent=stage.size();
        for(std::size_t j=0;j<stage.size();++j) {
            const auto& real=stage[j];
            if(real.item.ItemPosition==1 &&
               real.item.sItemInfo.CODE==ghost.item.sItemInfo.CODE &&
               !IsLegacyTwoHandMirror455(real)) {
                if(parent!=stage.size())
                    return DecodeFail454(detail,DecodeStage454::OrphanTwoHandMirror,
                        static_cast<int>(n),0,ghost.encoded.size(),-1,
                        ghost.item.ItemPosition,ghost.item.ItemCount);
                parent=j;
            }
        }
        if(parent==stage.size() || !claimed.insert(parent).second)
            return DecodeFail454(detail,DecodeStage454::OrphanTwoHandMirror,
                static_cast<int>(n),0,ghost.encoded.size(),-1,
                ghost.item.ItemPosition,ghost.item.ItemCount);
        // The mirror is an occupied physical index in its native A/B array.
        // Its ordinal cannot overlap ANY other record in the same array.
        for(std::size_t j=0;j<stage.size();++j)if(j!=n &&
            stage[j].secondary==ghost.secondary && stage[j].item.ItemPosition!=-1 &&
            stage[j].item.ItemCount==ghost.item.ItemCount)
            return DecodeFail454(detail,DecodeStage454::OrphanTwoHandMirror,
                static_cast<int>(n),0,ghost.encoded.size(),-1,
                ghost.item.ItemPosition,ghost.item.ItemCount);
    }
    out.swap(stage);return BuildStatus::Ready;
}

// Mint callback: bool create(ItemInfo&, requestedCode) — calls real native
// CreateItem; verifies output CODE and CheckItemForm; no client input trusted.
// Placement callback: bool place(const vector<NativeItem<Record>>&, bool& secondary,
//                               int& index,int& x,int& y) — collision and footprint.
template<class Dat,class Record,class Validate,class Mint,class Place>
BuildStatus Build(const Dat& source,std::size_t fileBytes,const std::vector<Need>& needs,
                  std::uint32_t outputCode,const char* outputName,
                  const summit_ci440::Key& key, Validate validator,Mint mint,
                  Place place,Dat& result,std::vector<IngredientCheck>& conservation) {
    if(!outputCode||!outputName||!outputName[0]||!summit_ci440::ValidKey(key)||
       needs.empty()||needs.size()>16)return BuildStatus::Invalid;
    std::set<std::uint32_t> codeSet;
    for(const Need& n:needs)if(!summit_ci440::IsNativeCi(n.code)||!n.quantity||
       n.quantity>10000||!codeSet.insert(n.code).second)return BuildStatus::Invalid;
    std::vector<NativeItem<Record>> old;
    auto st=Decode<Dat,Record>(source,fileBytes,validator,old);
    if(st!=BuildStatus::Ready)return st;
    std::vector<NativeItem<Record>> stage=old;
    std::vector<IngredientCheck> counts;
    for(const Need& n:needs) {
        std::uint32_t available=0;
        for(const auto& e:stage)if(e.item.sItemInfo.CODE==n.code&&e.item.ItemPosition==0){
            auto q=summit_ci440::Read(e.item.sItemInfo,key);
            if(q.status!=summit_ci440::Status::LegacyOne&&
               q.status!=summit_ci440::Status::VerifiedStack)return BuildStatus::BadRecord;
            if(available>UINT32_MAX-q.quantity)return BuildStatus::Invalid;
            available+=q.quantity;
        }
        if(available<n.quantity)return BuildStatus::Missing;
        counts.push_back({n.code,available,available-n.quantity});
    }
    for(const Need& need:needs) {
        std::uint32_t left=need.quantity;
        // A then B, native record order; preserve all unrelated encoded bytes.
        for(auto& e:stage) {
            if(!left||e.item.sItemInfo.CODE!=need.code||e.item.ItemPosition!=0)continue;
            const auto original=summit_ci440::Read(e.item.sItemInfo,key);
            const unsigned remove=std::min(left,original.quantity);
            const unsigned after=original.quantity-remove;
            if(after) {
                if(summit_ci440::Seal(e.item.sItemInfo,after,key,true,true,true)!=
                     summit_ci440::Status::Sealed)return BuildStatus::BadRecord;
                if(!validator(e.item.sItemInfo)||!
                    Encode(reinterpret_cast<const Byte*>(&e.item),sizeof(Record),e.encoded))
                    return BuildStatus::BadRecord;
            } else e.encoded.clear();
            left-=remove;
        }
        if(left)return BuildStatus::Conflict;
    }
    stage.erase(std::remove_if(stage.begin(),stage.end(),[](const NativeItem<Record>& e){
        return e.encoded.empty();}),stage.end());
    if(stage.size()>=200)return BuildStatus::Capacity;
    NativeItem<Record> crafted;
    if(!mint(crafted.item.sItemInfo,outputName)||
       crafted.item.sItemInfo.CODE!=outputCode||
       !crafted.item.sItemInfo.ItemHeader.Head||
       !crafted.item.sItemInfo.ItemHeader.dwChkSum||
       !validator(crafted.item.sItemInfo))return BuildStatus::BadMint;
    for(const auto& e:old)
      if(e.item.sItemInfo.CODE==crafted.item.sItemInfo.CODE&&
         e.item.sItemInfo.ItemHeader.Head==crafted.item.sItemInfo.ItemHeader.Head&&
         e.item.sItemInfo.ItemHeader.dwChkSum==crafted.item.sItemInfo.ItemHeader.dwChkSum)
        return BuildStatus::Conflict;
    bool secondary=false;int index=-1,x=-1,y=-1;
    if(!place(stage,crafted.item.sItemInfo,secondary,index,x,y)||index<0||index>=100||x<0||y<0)
        return BuildStatus::Capacity;
    for(const auto& e:stage)if(e.secondary==secondary&&e.item.ItemCount==index)
        return BuildStatus::Conflict;
    crafted.secondary=secondary;
    crafted.item.ItemCount=index;
    crafted.item.ItemPosition=0;
    crafted.item.x=x;crafted.item.y=y;
    if(!Encode(reinterpret_cast<const Byte*>(&crafted.item),sizeof(Record),crafted.encoded))
        return BuildStatus::BadMint;
    stage.push_back(std::move(crafted));
    std::stable_partition(stage.begin(),stage.end(),[](const NativeItem<Record>& e){return !e.secondary;});
    std::size_t total=0;std::size_t primary=0;
    for(const auto& e:stage) {
        if(total>sizeof(source.Data)-e.encoded.size())return BuildStatus::Capacity;
        total+=e.encoded.size();
        if(!e.secondary)++primary;
    }
    std::unique_ptr<Dat> next(new Dat(source));
    std::memset(next->Data,0,sizeof(next->Data));
    std::size_t offset=0;
    for(const auto& e:stage){
        std::memcpy(next->Data+offset,e.encoded.data(),e.encoded.size());
        offset+=e.encoded.size();
    }
    next->ItemCount=static_cast<int>(stage.size());
    next->ItemSubStart=static_cast<int>(primary);
    next->DataSize=static_cast<int>(total);
    next->size=static_cast<int>(offsetof(Dat,Data)+total);
    std::vector<NativeItem<Record>> check;
    if(Decode<Dat,Record>(*next,sizeof(Dat),validator,check)!=BuildStatus::Ready||
       check.size()!=stage.size())return BuildStatus::BadRecord;
    // Full conservation check after native encode/decode; no credit to SQLite.
    for(auto& expected:counts) {
        std::uint32_t actual=0;
        for(const auto& e:check)if(e.item.sItemInfo.CODE==expected.code&&
                                       e.item.ItemPosition==0) {
            auto q=summit_ci440::Read(e.item.sItemInfo,key);
            if(q.status!=summit_ci440::Status::LegacyOne&&
               q.status!=summit_ci440::Status::VerifiedStack)return BuildStatus::BadRecord;
            if(actual>UINT32_MAX-q.quantity)return BuildStatus::Invalid;
            actual+=q.quantity;
        }
        if(actual!=expected.after)return BuildStatus::Conflict;
    }
    conservation.swap(counts);
    result=*next;return BuildStatus::Ready;
}
} // namespace summit446
