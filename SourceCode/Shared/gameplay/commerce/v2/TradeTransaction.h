#pragma once

#include "gameplay/commerce/v2/TradeService.h"

namespace gameplay
{
namespace commerce
{
class ITradeTransactionService
{
public:
    virtual ~ITradeTransactionService() {}
    virtual TradeExactCommitResult CommitExactTrade(const TradeExactCommitRequest& request) = 0;
};
}
}
