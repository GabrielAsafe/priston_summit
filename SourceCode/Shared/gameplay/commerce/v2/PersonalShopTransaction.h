#pragma once

#include "gameplay/commerce/v2/PersonalShopService.h"

namespace gameplay
{
namespace commerce
{
struct PersonalShopTransactionRequest
{
    PersonalShopTradeRequest trade;
    bool potion;

    PersonalShopTransactionRequest(const PersonalShopTradeRequest& tradeValue = PersonalShopTradeRequest(),
                                   bool potionValue = false)
        : trade(tradeValue), potion(potionValue) {}
};

struct PersonalShopTransactionResult
{
    PersonalShopTradeResult trade;
    bool potionTransferFailed;

    PersonalShopTransactionResult() : trade(), potionTransferFailed(false) {}
};

class IPersonalShopTransactionService
{
public:
    virtual ~IPersonalShopTransactionService() {}
    virtual PersonalShopTransactionResult Commit(const PersonalShopTransactionRequest& request) = 0;
};
}
}
