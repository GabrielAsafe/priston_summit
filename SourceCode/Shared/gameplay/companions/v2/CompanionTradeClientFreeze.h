#pragma once
#include <atomic>
namespace gameplay { namespace companions { namespace v2 {
inline std::atomic<bool>& TradeClientFreezeFlag() {
    static std::atomic<bool> frozen{false};
    return frozen;
}
inline bool TradeClientFrozen() {
    return TradeClientFreezeFlag().load(std::memory_order_acquire);
}
inline void SetTradeClientFrozen(bool active) {
    TradeClientFreezeFlag().store(active,std::memory_order_release);
}
}}}
