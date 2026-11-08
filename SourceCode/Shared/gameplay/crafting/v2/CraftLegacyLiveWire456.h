#pragma once
// SUMMIT456L: single legacy inventory craft operation. No DAT rewrite.
// Use only after sITEMINFO is declared. The protocol is private to a
// matching SUMMIT client/server build (both are 32-bit MinGW executables).
#include <cstdint>
#include <cstring>
namespace summit456l {
static const int kOpcode = 0x4847A456;
static const int kReplyOpcode = 0x4847A457;
static const std::uint32_t kMagic = 0x314C3643u; // C6L1
static const unsigned kMaximumInputs = 16;
enum Step : int { Prepare=1, Ready=2, Commit=3, Applied=4,
                  Denied=5, SaveFailed=6, WaitSave=7 };
// SUMMIT457A: diagnostic reason is authenticated by the existing session and
// token; it is only diagnostic, never an authorisation to move any item.
enum Reason : int { ReasonNone=0, NoSession=1, UiBusy=2, SaveBusy=3,
    RewardInvalid=4, InputMissing=5, InputInvalid=6, ImageInvalid=7,
    RequirementsInvalid=8, StalePrepare=9, RemovedItemPresent=10,
    NoFreeSlot=11, PlacementFailed=12, RewardMissing=13,
    LegacySaveFailed=14 };
inline const char* ReasonName(int code) {
    switch(code) {
    case ReasonNone:return "none";
    case NoSession:return "no_session";
    case UiBusy:return "ui_busy";
    case SaveBusy:return "save_busy";
    case RewardInvalid:return "reward_invalid";
    case InputMissing:return "input_missing";
    case InputInvalid:return "input_invalid";
    case ImageInvalid:return "image_invalid";
    case RequirementsInvalid:return "requirements_invalid";
    case StalePrepare:return "stale_prepare";
    case RemovedItemPresent:return "removed_item_present";
    case NoFreeSlot:return "no_free_slot";
    case PlacementFailed:return "placement_failed";
    case RewardMissing:return "reward_missing";
    case LegacySaveFailed:return "legacy_save_failed";
    } return "unrecognised";
}
struct Identity {
    std::uint32_t code,head,checksum;
    Identity():code(0),head(0),checksum(0) {}
    Identity(std::uint32_t a,std::uint32_t b,std::uint32_t c):code(a),head(b),checksum(c) {}
    bool Valid()const{return code&&head&&checksum;}
    bool operator==(const Identity& o)const {
        return code==o.code&&head==o.head&&checksum==o.checksum;
    }
};
template<class Item> struct Delivery {
    int size,code;
    std::uint32_t magic,token;
    int step,count;
    Identity inputs[kMaximumInputs];
    Item reward;
};
struct Reply {
    int size,code;
    std::uint32_t magic,token;
    int step,reason;
};
inline Reply MakeReply(std::uint32_t token,int step,int reason=ReasonNone) {
    Reply r={};r.size=sizeof(r);r.code=kReplyOpcode;
    r.magic=kMagic;r.token=token;r.step=step;r.reason=reason;return r;
}
template<class Item> inline bool ValidDelivery(const Delivery<Item>& p) {
    if(p.size!=static_cast<int>(sizeof(p))||p.code!=kOpcode||
       p.magic!=kMagic||p.token==0||p.count<=0||
       p.count>static_cast<int>(kMaximumInputs)||
       (p.step!=Prepare&&p.step!=Commit))return false;
    for(int i=0;i<p.count;++i){
        if(!p.inputs[i].Valid())return false;
        for(int j=0;j<i;++j)if(p.inputs[i]==p.inputs[j])return false;
    }
    return true;
}
inline bool ValidReply(const Reply& p) {
    return p.size==sizeof(p)&&p.code==kReplyOpcode&&p.magic==kMagic&&
           p.token!=0&&p.reason>=ReasonNone&&p.reason<=LegacySaveFailed&&
           (p.step==Ready||p.step==Applied||p.step==WaitSave||
                        p.step==Denied||p.step==SaveFailed)&&
           ((p.step==Ready||p.step==Applied)?p.reason==ReasonNone:true)&&
           (p.step!=WaitSave||p.reason==SaveBusy);
}
} // namespace summit456l
