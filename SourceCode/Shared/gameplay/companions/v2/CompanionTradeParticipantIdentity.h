#pragma once
// Stable account-scoped, server-only journal participant identity. Never trust
// participant identities received in a client packet.
#include <cstdint>
namespace gameplay { namespace companions { namespace v2 {
inline std::uint64_t TradeParticipantId(const char* account,const char* name) {
    std::uint64_t h=UINT64_C(14695981039346656037);
    for(const char* p=account;p && *p;++p) {
        const unsigned char c=static_cast<unsigned char>(*p);
        h^=(c>='A' && c<='Z') ? static_cast<unsigned char>(c+32) : c;
        h*=UINT64_C(1099511628211);
    }
    h^=':';h*=UINT64_C(1099511628211);
    for(const char* p=name;p && *p;++p) {
        const unsigned char c=static_cast<unsigned char>(*p);
        h^=(c>='A' && c<='Z') ? static_cast<unsigned char>(c+32) : c;
        h*=UINT64_C(1099511628211);
    }
    return h ? h : 1;
}
}}}
