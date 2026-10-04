#include "gameplay/commerce/v2/AgingStoneKindPolicy.h"

#include <cassert>

int main()
{
    gameplay::commerce::AgingStoneKindConfig config;
    for(int i = 0; i < 9; ++i)
        config.stoneCodes[i] = static_cast<unsigned int>(100 + i);

    assert(gameplay::commerce::AgingStoneKindPolicy::Classify(0u, config) == 0);
    assert(gameplay::commerce::AgingStoneKindPolicy::Classify(99u, config) == 0);
    for(int i = 0; i < 9; ++i)
        assert(gameplay::commerce::AgingStoneKindPolicy::Classify(static_cast<unsigned int>(100 + i), config) == i + 1);

    return 0;
}
