#include "gameplay/commerce/v2/CraftingService.h"

#include <cassert>
#include <iostream>

namespace
{
class FakeCraftingService final : public gameplay::commerce::ICraftingService
{
public:
    gameplay::commerce::CraftResult Craft(const gameplay::commerce::CraftRequest& request) override
    {
        gameplay::commerce::CraftResult result;
        result.player = request.player;
        result.code = request.player.Valid() && !request.requestBytes.empty()
                          ? gameplay::commerce::CraftingCode::Processed
                          : gameplay::commerce::CraftingCode::InvalidPayload;
        return result;
    }

    gameplay::commerce::CraftingAuxiliaryResult ProcessAuxiliary(
        const gameplay::commerce::CraftingAuxiliaryRequest& request) override
    {
        gameplay::commerce::CraftingAuxiliaryResult result;
        result.player = request.player;
        result.operation = request.operation;
        result.code = request.player.Valid() && !request.requestBytes.empty()
                          ? gameplay::commerce::CraftingCode::Processed
                          : gameplay::commerce::CraftingCode::InvalidPayload;
        return result;
    }
};
}

int main()
{
    gameplay::commerce::CraftRequest request;
    request.player = gameplay::PlayerId(7u);
    request.requestBytes.push_back(1u);
    FakeCraftingService service;
    const gameplay::commerce::CraftResult result = service.Craft(request);
    assert(result.Processed());
    assert(result.player == gameplay::PlayerId(7u));

    gameplay::commerce::CraftingAuxiliaryRequest auxiliary;
    auxiliary.player = gameplay::PlayerId(7u);
    auxiliary.operation = gameplay::commerce::CraftingAuxiliaryOperation::Smelting;
    auxiliary.requestBytes.push_back(1u);
    assert(service.ProcessAuxiliary(auxiliary).Processed());
    std::cout << "crafting service smoke: PASS\n";
    return 0;
}
