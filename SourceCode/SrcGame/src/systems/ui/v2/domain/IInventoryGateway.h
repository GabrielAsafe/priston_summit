#pragma once

#include "systems/ui/v2/domain/CharacterUIModels.h"

namespace ui::v2
{
class IInventoryGateway
{
public:
    virtual ~IInventoryGateway() = default;
    [[nodiscard]] virtual InventorySnapshot GetSnapshot() const = 0;
    // SUMMIT427A read-only view of inactive bag. Not an authority for crafting.
    [[nodiscard]] virtual std::vector<InventoryItemSnapshot> GetInactiveBagForCrafting427() const { return {}; }
    [[nodiscard]] virtual std::vector<DetailLineSnapshot> GetItemDetails(int itemIndex) const { (void)itemIndex; return {}; }

    virtual bool BeginDrag(int itemIndex) = 0;
    virtual bool DropToBag(int column, int row) = 0;
    virtual bool DropToQuest(int column, int row) = 0;
    virtual bool DropToEquipment(int equipmentSlot) = 0;
    virtual bool CancelDrag() = 0;
    virtual bool ActivateItem(int itemIndex) = 0;
    virtual bool SelectBagPage(int page) = 0;
    virtual bool SelectWeaponSet(int set) = 0;
    virtual bool SetQuestItemsVisible(bool visible) = 0;
    virtual bool OpenPersonalShopSales() = 0;
    virtual bool ToggleLock() = 0;
    virtual bool ToggleCostumeMode() = 0;
};
}
