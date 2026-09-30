#pragma once

#include <string>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace inventory
{
enum class ItemCategory
{
    Unknown,
    General,
    Equipment,
    Consumable
};

struct ItemDefinition
{
    ItemId id;
    std::string contentKey;
    std::string displayName;
    ItemCategory category;
    bool stackable;
    bool defaultGrantable;

    ItemDefinition()
        : id(), contentKey(), displayName(), category(ItemCategory::Unknown), stackable(false), defaultGrantable(false) {}

    ItemDefinition(ItemId idValue,
                   const std::string& contentKeyValue,
                   ItemCategory categoryValue,
                   bool stackableValue,
                   bool defaultGrantableValue = false,
                   const std::string& displayNameValue = std::string())
        : id(idValue), contentKey(contentKeyValue), displayName(displayNameValue), category(categoryValue), stackable(stackableValue),
          defaultGrantable(defaultGrantableValue) {}

    bool Known() const { return id.Valid() && category != ItemCategory::Unknown; }
};

class IItemCatalog
{
public:
    virtual ~IItemCatalog() {}
    virtual bool TryGetDefinition(ItemId item, ItemDefinition& out) const = 0;
    virtual bool TryGetDefinitionByContentKey(const std::string& contentKey, ItemDefinition& out) const = 0;
};
}
}
