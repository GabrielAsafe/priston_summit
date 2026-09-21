#pragma once

#include <string>

namespace gameplay
{
namespace reputation
{
struct ReputationDefinition
{
    std::string key;
    std::string displayNameKey;
    bool hasBounds;
    int minValue;
    int maxValue;

    ReputationDefinition()
        : hasBounds(false), minValue(0), maxValue(0)
    {
    }
};

struct ReputationCatalogEntry
{
    std::string key;
    std::string displayNameKey;
    bool hasBounds;
    int minValue;
    int maxValue;

    ReputationCatalogEntry()
        : hasBounds(false), minValue(0), maxValue(0)
    {
    }
};

struct ReputationSnapshot
{
    std::string key;
    std::string displayNameKey;
    int value;
    bool hasBounds;
    int minValue;
    int maxValue;

    ReputationSnapshot()
        : value(0), hasBounds(false), minValue(0), maxValue(0)
    {
    }
};
}
}
