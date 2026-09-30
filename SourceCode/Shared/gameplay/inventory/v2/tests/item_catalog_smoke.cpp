#include "gameplay/inventory/v2/ItemCatalog.h"

#include <cassert>
#include <iostream>
#include <map>

namespace
{
class FakeItemCatalog final : public gameplay::inventory::IItemCatalog
{
public:
    bool TryGetDefinition(gameplay::ItemId item, gameplay::inventory::ItemDefinition& out) const override
    {
        const auto found = items.find(item.value);
        if(found == items.end()) return false;
        out = found->second;
        return true;
    }

    bool TryGetDefinitionByContentKey(const std::string& contentKey, gameplay::inventory::ItemDefinition& out) const override
    {
        for(const auto& pair : items)
            if(pair.second.contentKey == contentKey)
            {
                out = pair.second;
                return true;
            }
        return false;
    }

    std::map<std::uint32_t, gameplay::inventory::ItemDefinition> items;
};
}

int main()
{
    using namespace gameplay;
    using namespace gameplay::inventory;
    FakeItemCatalog catalog;
    catalog.items.emplace(0x04010100u,
        ItemDefinition(ItemId(0x04010100u), "PL101", ItemCategory::Consumable, true, true, "Potion"));
    ItemDefinition definition;
    assert(catalog.TryGetDefinition(ItemId(0x04010100u), definition));
    assert(definition.Known());
    assert(definition.stackable);
    assert(definition.defaultGrantable);
    assert(definition.displayName == "Potion");
    assert(definition.category == ItemCategory::Consumable);
    assert(catalog.TryGetDefinitionByContentKey("PL101", definition));
    assert(definition.id.value == 0x04010100u);
    assert(!catalog.TryGetDefinition(ItemId(0x99999999u), definition));
    assert(!catalog.TryGetDefinitionByContentKey("missing", definition));
    std::cout << "item catalog smoke: PASS\n";
}
