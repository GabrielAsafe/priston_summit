#pragma once

#include <string>

namespace gameplay
{
namespace reputation
{
struct ReputationChangedEvent
{
    std::string playerKey;
    std::string reputationKey;
    int oldValue;
    int newValue;
    std::string sourceKey;

    ReputationChangedEvent() : oldValue(0), newValue(0) {}
};
}
}
