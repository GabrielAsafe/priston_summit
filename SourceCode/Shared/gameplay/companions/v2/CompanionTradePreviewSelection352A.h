#pragma once
// SUMMIT352A: presentation-only explanation of legacy preview restrictions.
// The server remains the final authority for any Companion item transfer.
#include <cstdint>
namespace gameplay { namespace companions { namespace v2 {
enum class TradePreviewBlock352A {
    None, Equipped, UnknownPosition, IncompleteIdentity, PotionStack
};
inline TradePreviewBlock352A TradePreviewBlockReason352A(
    int position, std::uint32_t code, std::uint32_t key, std::uint32_t checksum) {
    if(position<0 || position>16) return TradePreviewBlock352A::UnknownPosition;
    if(position!=0) return TradePreviewBlock352A::Equipped;
    // SUMMIT353: potion stack semantics take priority over missing instance
    // keys. Potions often have no key by design; "SEM ID" was misleading.
    if((code&0xff000000u)==0x04000000u) return TradePreviewBlock352A::PotionStack;
    if(!code || !key || !checksum) return TradePreviewBlock352A::IncompleteIdentity;
    return TradePreviewBlock352A::None;
}
inline const char* TradePreviewBlockLabel352A(TradePreviewBlock352A reason) {
    switch(reason) {
        case TradePreviewBlock352A::Equipped: return "EQUIP";
        case TradePreviewBlock352A::UnknownPosition: return "POS?";
        case TradePreviewBlock352A::IncompleteIdentity: return "SEM ID";
        case TradePreviewBlock352A::PotionStack: return "POCAO";
        default: return "";
    }
}
}}}
