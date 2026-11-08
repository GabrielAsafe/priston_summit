#pragma once
// SUMMIT442A — non-authoritative CLIENT presentation of native CI quantity.
// Does not verify HMAC: only the SERVER with its secret may authorize quantity,
// merges, splits, trades, crafting debits or DAT writes. Never ship secret.
#include <cstdint>

namespace summit_ci442 {
constexpr std::uint32_t kMagic = 0x31534943u; // SUMMIT440A, "CIS1"
constexpr std::uint32_t kFirstCi = 0x080B8000u;
constexpr std::uint32_t kLastCi = 0x080BC300u;
constexpr unsigned kMaxQuantity = 999u;
inline bool IsNativeCi(std::uint32_t code) {
    return code>=kFirstCi && code<=kLastCi && ((code-kFirstCi)&255u)==0u;
}
enum class State : unsigned char { NotCi, LegacyOne, StackHintUnverified, Invalid };
struct Reading {
    State state = State::NotCi;
    unsigned quantityHint = 0;
    Reading(State s=State::NotCi,unsigned q=0):state(s),quantityHint(q){}
    bool IsCi() const { return state!=State::NotCi; }
    bool IsSuspicious() const { return state==State::Invalid; }
    // A legacy client must not mutate sealed stacks; only a server operation
    // preserving/re-signing the item may do so.
    bool PreventLegacyMutation() const { return state==State::StackHintUnverified || IsSuspicious(); }
};
template<class Info> inline Reading Inspect(const Info& info) {
    const auto code=static_cast<std::uint32_t>(info.CODE);
    if(!IsNativeCi(code)) return {};
    if(!info.ItemHeader.Head || !info.ItemHeader.dwChkSum)
        return {State::Invalid,0};
    if(static_cast<std::uint32_t>(info.dwTemp[0])==kMagic) {
        if(info.PotionCount<1 || info.PotionCount>static_cast<int>(kMaxQuantity))
            return {State::Invalid,0};
        // The eight signature DWORDs may not be all zero. The client cannot
        // verify MAC and treats this as a DISPLAY HINT, never an authority.
        std::uint32_t any=0;
        for(unsigned i=1;i<9;++i)any|=static_cast<std::uint32_t>(info.dwTemp[i]);
        if(!any)return {State::Invalid,0};
        return {State::StackHintUnverified,static_cast<unsigned>(info.PotionCount)};
    }
    for(unsigned i=0;i<9;++i) if(info.dwTemp[i]!=0)
        return {State::Invalid,0};
    if(info.PotionCount<0 || info.PotionCount>1)
        return {State::Invalid,0};
    return {State::LegacyOne,1};
}
} // namespace summit_ci442
