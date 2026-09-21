#pragma once

#include "systems/ui/v2/domain/IInventoryGateway.h"

namespace ui::v2
{
class InventoryViewModel
{
public:
    explicit InventoryViewModel(IInventoryGateway& gateway) : gateway_(gateway) {}

    void Refresh() { snapshot_ = gateway_.GetSnapshot(); }
    [[nodiscard]] const InventorySnapshot& Snapshot() const noexcept { return snapshot_; }
    [[nodiscard]] std::vector<DetailLineSnapshot> DetailsFor(int index) const { return gateway_.GetItemDetails(index); }

    void Select(int index) noexcept { selectedIndex_ = index; }
    [[nodiscard]] int SelectedIndex() const noexcept { return selectedIndex_; }

    [[nodiscard]] const InventoryItemSnapshot* Selected() const noexcept
    {
        for (const auto& item : snapshot_.items)
            if (item.index == selectedIndex_)
                return &item;
        return nullptr;
    }

    bool BeginDrag(int itemIndex)
    {
        selectedIndex_ = itemIndex;
        const bool ok = gateway_.BeginDrag(itemIndex);
        Refresh();
        return ok;
    }

    bool DropToBag(int column, int row)
    {
        const bool ok = gateway_.DropToBag(column, row);
        Refresh();
        return ok;
    }

    bool DropToQuest(int column, int row)
    {
        const bool ok = gateway_.DropToQuest(column, row);
        Refresh();
        return ok;
    }

    bool DropToEquipment(int slot)
    {
        const bool ok = gateway_.DropToEquipment(slot);
        Refresh();
        return ok;
    }

    bool CancelDrag()
    {
        const bool ok = gateway_.CancelDrag();
        Refresh();
        return ok;
    }

    bool Activate(int itemIndex)
    {
        selectedIndex_ = itemIndex;
        const bool ok = gateway_.ActivateItem(itemIndex);
        Refresh();
        return ok;
    }

    bool SelectBagPage(int page)
    {
        const bool ok = gateway_.SelectBagPage(page);
        Refresh();
        return ok;
    }

    bool SelectWeaponSet(int set)
    {
        const bool ok = gateway_.SelectWeaponSet(set);
        Refresh();
        return ok;
    }

    bool SetQuestItemsVisible(bool visible)
    {
        const bool ok = gateway_.SetQuestItemsVisible(visible);
        Refresh();
        return ok;
    }

    bool OpenPersonalShopSales()
    {
        const bool ok = gateway_.OpenPersonalShopSales();
        Refresh();
        return ok;
    }

    bool ToggleLock()
    {
        const bool ok = gateway_.ToggleLock();
        Refresh();
        return ok;
    }

    bool ToggleCostume()
    {
        const bool ok = gateway_.ToggleCostumeMode();
        Refresh();
        return ok;
    }

private:
    IInventoryGateway& gateway_;
    InventorySnapshot snapshot_{};
    int selectedIndex_ = -1;
};
}
