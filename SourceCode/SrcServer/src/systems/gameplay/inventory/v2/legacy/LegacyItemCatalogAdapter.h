#pragma once

#include "gameplay/inventory/v2/ItemCatalog.h"

namespace gameplay
{
namespace inventory
{
namespace legacy
{
class LegacyItemCatalogAdapter final : public IItemCatalog
{
public:
    bool TryGetDefinition(ItemId item, ItemDefinition& out) const override;
    bool TryGetDefinitionByContentKey(const std::string& contentKey, ItemDefinition& out) const override;
};

LegacyItemCatalogAdapter& ServerLegacyItemCatalogAdapter();
}
}
}
