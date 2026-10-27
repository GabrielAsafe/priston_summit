#pragma once
// An explicit server save RESULT is required to advance a trade handshake.
// Clearing rsRecorder.dwRecordTime alone also occurs on rejected saves/timeouts.
#include <atomic>
namespace gameplay { namespace companions { namespace v2 {
inline std::atomic<int>& TradeClientSaveReceiptState() {
    static std::atomic<int> result{0}; // 0 awaiting, +1 server accepted, -1 rejected
    return result;
}
inline void ArmTradeClientSaveReceipt() {
    TradeClientSaveReceiptState().store(0,std::memory_order_release);
}
inline void OnTradeClientSaveResult(bool accepted) {
    TradeClientSaveReceiptState().store(accepted?1:-1,std::memory_order_release);
}
inline int TradeClientSaveReceiptStatus() {
    return TradeClientSaveReceiptState().load(std::memory_order_acquire);
}
}}}
