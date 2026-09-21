#pragma once

#include "systems/ui/v2/domain/IInventoryGateway.h"

namespace ui::v2::legacy
{
class LegacyInventoryGateway final : public IInventoryGateway
{
public:
    [[nodiscard]] InventorySnapshot GetSnapshot() const override;
    [[nodiscard]] std::vector<DetailLineSnapshot> GetItemDetails(int itemIndex) const override;

    bool BeginDrag(int itemIndex) override;
    bool DropToBag(int column, int row) override;
    bool DropToQuest(int column, int row) override;
    bool DropToEquipment(int equipmentSlot) override;
    bool CancelDrag() override;
    bool ActivateItem(int itemIndex) override;
    bool SelectBagPage(int page) override;
    bool SelectWeaponSet(int set) override;
    bool SetQuestItemsVisible(bool visible) override;
    bool OpenPersonalShopSales() override;
    bool ToggleLock() override;
    bool ToggleCostumeMode() override;

private:
    bool DropAtLegacyPoint(int x, int y);
};
}
