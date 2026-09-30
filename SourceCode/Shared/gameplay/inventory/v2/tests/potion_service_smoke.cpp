#include "gameplay/inventory/v2/PotionService.h"

#include <cassert>

namespace
{
class FakePotionService final : public gameplay::inventory::IPotionService
{
public:
    gameplay::inventory::PotionUseResult ConsumeAndApply(
        const gameplay::inventory::PotionUseRequest& request) override
    {
        gameplay::inventory::PotionUseResult result;
        if(request.target.Valid() && !request.accountName.empty() && !request.characterName.empty())
        {
            result.code = gameplay::inventory::PotionUseCode::Ok;
            result.consumed = true;
            result.restoreAmount = 100;
            result.beforeValue = 50;
            result.afterValue = 150;
            result.maxValue = 200;
        }
        return result;
    }
};
}

int main()
{
    FakePotionService service;
    gameplay::inventory::PotionUseRequest request(
        "account", "character", gameplay::EntityId(7u), gameplay::inventory::PotionResource::Life);
    assert(service.ConsumeAndApply(request).Ok());
    request.target = gameplay::EntityId();
    assert(!service.ConsumeAndApply(request).Ok());
    return 0;
}
