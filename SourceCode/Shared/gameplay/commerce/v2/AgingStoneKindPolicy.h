#pragma once

#include <cstdint>

namespace gameplay
{
namespace commerce
{
struct AgingStoneKindConfig
{
    std::uint32_t stoneCodes[9];

    AgingStoneKindConfig()
    {
        for(int i = 0; i < 9; ++i)
            stoneCodes[i] = 0u;
    }
};

class AgingStoneKindPolicy
{
public:
    static int Classify(std::uint32_t stoneCode, const AgingStoneKindConfig& config)
    {
        if(stoneCode == 0u)
            return 0;

        for(int i = 0; i < 9; ++i)
        {
            if(config.stoneCodes[i] != 0u && config.stoneCodes[i] == stoneCode)
                return i + 1;
        }
        return 0;
    }
};
}
}
