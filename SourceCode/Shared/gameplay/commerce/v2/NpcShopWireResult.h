#pragma once

// NPC shop response in the EXISTING TRANS_BUY_SHOPITEM opcode.
// ItemCount > 0 is a server-confirmed purchase; negative counts are
// explicit rejections. Zero is reserved/invalid. Never grant on NACK.
namespace gameplay { namespace commerce {
enum class NpcShopRejection : int {
    InvalidRequest = 1,
    InvalidItem = 2,
    InsufficientFunds = 3,
    InventoryRejected = 4,
    TransactionRejected = 5
};
constexpr int EncodeNpcShopRejection(NpcShopRejection reason) {
    return -static_cast<int>(reason);
}
constexpr bool IsNpcShopRejection(int count) { return count < 0; }
constexpr int DecodeNpcShopRejection(int count) { return count < 0 ? -count : 0; }
}}
