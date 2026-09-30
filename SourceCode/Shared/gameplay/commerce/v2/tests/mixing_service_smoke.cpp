#include "gameplay/commerce/v2/MixingService.h"

#include <cassert>
#include <iostream>

namespace
{
class FakeMixingService final : public gameplay::commerce::IMixingService
{
public:
    gameplay::commerce::MixingResult Process(const gameplay::commerce::MixingRequest& request) override
    {
        gameplay::commerce::MixingResult result;
        result.player = request.player;
        result.operation = request.operation;
        result.code = request.player.Valid() && !request.requestBytes.empty()
                          ? gameplay::commerce::MixingCode::Processed
                          : gameplay::commerce::MixingCode::InvalidPayload;
        return result;
    }
};
}

int main()
{
    FakeMixingService service;
    gameplay::commerce::MixingRequest request;
    request.player = gameplay::PlayerId(10u);
    request.requestBytes.push_back(1u);
    const gameplay::commerce::MixingResult result = service.Process(request);
    assert(result.Processed());
    assert(result.operation == gameplay::commerce::MixingOperation::Reset);
    std::cout << "mixing service smoke: PASS\n";
    return 0;
}
