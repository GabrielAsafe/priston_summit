#pragma once

#include "gameplay/inventory/v2/ItemCatalog.h"
#include "gameplay/inventory/v2/EquipmentPolicy.h"
#include "gameplay/inventory/v2/ItemFootprint.h"

namespace gameplay
{
namespace inventory
{
namespace legacy
{
class LegacyItemCatalogAdapter final : public IItemCatalog, public IEquipmentCatalog, public IItemFootprintCatalog
{
public:
    bool TryGetDefinition(ItemId item, ItemDefinition& out) const override;
    bool TryGetDefinitionByContentKey(const std::string& contentKey, ItemDefinition& out) const override;
    bool TryGetEquipment(ItemId item, EquipmentDefinition& out) const override;
    bool TryGetFootprint(ItemId item, ItemFootprint& out) const override;
};

LegacyItemCatalogAdapter& ServerLegacyItemCatalogAdapter();
}
}
}
