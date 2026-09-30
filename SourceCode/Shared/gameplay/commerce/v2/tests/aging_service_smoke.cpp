#include "gameplay/commerce/v2/AgingService.h"

#include <cassert>
#include <iostream>

namespace
{
class FakeAgingService final : public gameplay::commerce::IAgingService
{
public:
    gameplay::commerce::AgingResult Process(const gameplay::commerce::AgingRequest& request) override
    {
        gameplay::commerce::AgingResult result;
        result.player = request.player;
        result.operation = request.operation;
        result.code = request.player.Valid() && !request.requestBytes.empty()
                          ? gameplay::commerce::AgingCode::Processed
                          : gameplay::commerce::AgingCode::InvalidPayload;
        return result;
    }
};
}

int main()
{
    FakeAgingService service;
    gameplay::commerce::AgingRequest request;
    request.player = gameplay::PlayerId(9u);
    request.operation = gameplay::commerce::AgingOperation::Upgrade;
    request.requestBytes.push_back(1u);
    const gameplay::commerce::AgingResult result = service.Process(request);
    assert(result.Processed());
    assert(result.operation == gameplay::commerce::AgingOperation::Upgrade);
    std::cout << "aging service smoke: PASS\n";
    return 0;
}
