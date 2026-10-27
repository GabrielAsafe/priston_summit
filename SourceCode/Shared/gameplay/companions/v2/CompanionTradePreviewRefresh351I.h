#pragma once
#include <cstdint>
namespace gameplay { namespace companions { namespace v2 {
// One refresh per authenticated successful trade, after the post-save gate
// releases the client. A commit alone must never refresh a stale preview.
inline bool TradePreviewRefreshDue351I(bool matchingOpenPreview,bool received,
    int status,std::uint64_t requestId,std::uint64_t lastRefreshed,bool frozen) {
    return matchingOpenPreview && received && status==1 && requestId!=0 &&
           requestId!=lastRefreshed && !frozen;
}
}}}
