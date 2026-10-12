#pragma once
// Acknowledgement is a semantic server result, not proof from packet send.
// All values are fixed-size and ABI-neutral; transport uses smTRANS_COMMAND.
namespace gameplay { namespace commerce { namespace premium_wire {
static const int ResultOpcode = 0x48479F72;
static const int CoinScope = 0;
static const int TimeScope = 1;
enum Code {
    Purchased = 0,
    InvalidRequest = 1,
    ReplayedRequest = 2,
    UnavailableProduct = 3,
    WalletUnavailable = 4,
    InsufficientFunds = 5,
    DeliveryRejected = 6,
    TransactionRejected = 7
};
inline bool UnsupportedLegacyNoOp(const char* code) {
    if (!code || !*code) return true;
    return *code == 'P' || *code == 'p' || *code == 'G' || *code == 'g';
}
}}}
