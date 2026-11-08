#include "systems/ui/v2/legacy/LegacyInventoryGateway.h"
#include "systems/ui/v2/legacy/CanonicalInventoryName428.h"
#include "gameplay/crafting/v2/CiStackPresentation442.h" // SUMMIT442A
#include "systems/ui/v2/legacy/SummitEquipmentTypeOverridesGenerated.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/inventory/legacy/sinItem.h"
#include "gameplay/classes/v2/services/SummitCostumeGenderPolicy.h"
#include "gameplay/companions/v2/CompanionTradeClientFreeze.h"

extern char szInfoBuff[5000];
extern char szInfoBuff2[5000];
extern int RedLine[10];
extern int RequireLine[10];

namespace ui::v2::legacy
{
namespace
{
constexpr int LegacyBagX = 23;
constexpr int LegacyBagY = 436;
constexpr int LegacyCell = 22;
constexpr int LegacyColumns = INVENTORY_BOX_ROW; // 12
constexpr int LegacyRows = INVENTORY_BOX_COL;    // 6
constexpr int LegacyQuestCell = 44;
constexpr int LegacyQuestColumns = 6;
constexpr int LegacyQuestRows = 3;
constexpr int HiddenInventoryOffset = 10000;

bool ValidInventoryIndex(int index)
{
    return index >= 0 && index < INVENTORY_MAXITEM;
}

bool ValidEquipmentSlot(int slot)
{
    return slot >= 1 && slot <= INVENTORY_MAX_POS;
}

bool IsQuestItem(const sITEM& item)
{
    return item.sItemInfo.SpecialItemFlag[1] == ITEM_KIND_SPECIAL;
}

// Display only. No HMAC key exists on the client. The server must verify any
// authenticated quantity before authorising an operation involving a CI stack.
summit_ci442::Reading CiReading442(const sITEM& item) {
    const auto hint=summit_ci442::Inspect(item.sItemInfo);
    if((hint.IsCi() || summit_ci442::IsNativeCi(static_cast<std::uint32_t>(item.CODE))) &&
       static_cast<std::uint32_t>(item.CODE)!=static_cast<std::uint32_t>(item.sItemInfo.CODE))
        return {summit_ci442::State::Invalid,0};
    return hint;
}
void PresentCi442(const sITEM& item, InventoryItemSnapshot& entry) {
    const auto hint=CiReading442(item);
    if(!hint.IsCi())return;
    entry.nativeCiQuantity=true;
    entry.ciQuantityInvalid=hint.IsSuspicious();
    entry.ciMutationBlocked=hint.PreventLegacyMutation();
    entry.potionCount=static_cast<int>(hint.quantityHint);
    if(entry.ciMutationBlocked)entry.usable=false;
}
bool UnsafeCiMutation442(const sITEM& item) {
    return CiReading442(item).PreventLegacyMutation();
}

POINT ItemCenter(const sITEM& item)
{
    return {item.x + std::max(1, item.w / 2), item.y + std::max(1, item.h / 2)};
}

std::vector<std::string> SplitLegacyLines(const char* text)
{
    std::vector<std::string> lines;
    if (!text)
        return lines;
    const std::string value(text);
    std::size_t start = 0;
    while (start <= value.size())
    {
        const std::size_t end = value.find("/r", start);
        if (end == std::string::npos)
        {
            if (start < value.size())
                lines.emplace_back(value.substr(start));
            break;
        }
        lines.emplace_back(value.substr(start, end - start));
        start = end + 2;
        if (start == value.size())
            break;
    }
    return lines;
}

bool IsLineIn(const int* values, int count, int oneBasedLine)
{
    for (int i = 0; i < count; ++i)
        if (values[i] > 0 && values[i] == oneBasedLine)
            return true;
    return false;
}

bool CostumeUseAllowed(sITEM& item)
{
    if (!sinChar || item.Class != ITEM_CLASS_COSTUME || item.NotUseFlag ||
        item.sItemInfo.NotUseFlag)
        return false;
    // Recalcular requisitos em copia: nao alterar item persistido nem cursor.
    sITEM candidate = item;
    cInvenTory.CheckRequireItemToSet(&candidate);
    return candidate.sItemInfo.NotUseFlag == 0;
}

class CursorGuard
{
public:
    CursorGuard() : old_(pCursorPos) {}
    ~CursorGuard() { pCursorPos = old_; }

    void Set(int x, int y)
    {
        pCursorPos.x = x;
        pCursorPos.y = y;
    }

private:
    POINT old_{};
};

// The V2 inventory has separate Armor (3) and Costume (14) widgets, but the
// original client maps both to the same screen rectangle. CostumeFlag is a
// legacy hit-testing discriminator. Scope it to this synchronous operation;
// never change the user's selected visual mode or its cooldown/timer.
class ScopedLegacyEquipmentHitTest
{
public:
    explicit ScopedLegacyEquipmentHitTest(int equipmentSlot)
        : saved_(cInvenTory.CostumeFlag)
    {
        if (equipmentSlot == 3)
            cInvenTory.CostumeFlag = 0;
        else if (equipmentSlot == 14)
            cInvenTory.CostumeFlag = 1;
    }

    ~ScopedLegacyEquipmentHitTest()
    {
        cInvenTory.CostumeFlag = saved_;
    }

    ScopedLegacyEquipmentHitTest(const ScopedLegacyEquipmentHitTest&) = delete;
    ScopedLegacyEquipmentHitTest& operator=(const ScopedLegacyEquipmentHitTest&) = delete;

private:
    int saved_;
};

// Corrupted inventory records may already have an item in the wrong slot.
// Pick the legacy view by the item kind so it can still be removed in V2,
// without using this helper to *place* any item in the wrong slot.
int LegacySelectionSlot(const sITEM& item)
{
    if (item.ItemPosition != 3 && item.ItemPosition != 14)
        return 0;
    return item.ItemPosition; // Remocao pelo slot fisico, mesmo se tipo antigo estiver incorreto.
}

// If two records of the same type occupy the overlapping legacy rectangle,
// coordinate-based pickup cannot select one by array index safely.
bool AmbiguousLegacyEquippedSelection(int itemIndex)
{
    const sITEM& chosen = cInvenTory.InvenItem[itemIndex];
    if (chosen.ItemPosition != 3 && chosen.ItemPosition != 14)
        return false;
    for (int i = 0; i < INVENTORY_MAXITEM; ++i)
    {
        if (i == itemIndex)
            continue;
        const sITEM& other = cInvenTory.InvenItem[i];
        if (other.Flag && (other.ItemPosition == 3 || other.ItemPosition == 14) &&
            (other.Class == ITEM_CLASS_COSTUME) == (chosen.Class == ITEM_CLASS_COSTUME))
            return true;
    }
    return false;
}
}

InventorySnapshot LegacyInventoryGateway::GetSnapshot() const
{
    InventorySnapshot out;
    if (!sinChar)
        return out;

    out.valid = true;
    out.columns = LegacyColumns;
    out.rows = LegacyRows;
    out.questColumns = LegacyQuestColumns;
    out.questRows = LegacyQuestRows;
    out.activeBagPage = sinChar->ArrowPosi[0] == 2 ? 2 : 1;
    out.activeWeaponSet = sinChar->ArrowPosi[1] == 4 ? 2 : 1;
    out.gold = sinChar->Money;
    out.weightCurrent = sinChar->Weight[0];
    out.weightMax = sinChar->Weight[1];
    out.locked = cInvenTory.LockFlag != 0;
    out.costumeMode = cInvenTory.CostumeFlag != 0;
    out.personalShopOpen = cMyShop.OpenFlag != 0 || MyShopSendButton != 0;
    out.blacksmithOpen = cShop.OpenFlag != 0;
    out.items.reserve(INVENTORY_MAXITEM);

    for (int i = 0; i < INVENTORY_MAXITEM; ++i)
    {
        const sITEM& item = cInvenTory.InvenItem[i];
        if (!item.Flag)
            continue;

        InventoryItemSnapshot entry;
        entry.index = i;
        entry.code = item.CODE;
        entry.name = summit_inventory_name_428::Display(item.CODE, item.sItemInfo.ItemName); // SUMMIT428A
        entry.widthPixels = std::max(1, item.w);
        entry.heightPixels = std::max(1, item.h);
        entry.equipmentSlot = item.ItemPosition;
        entry.equipped = item.ItemPosition > 0;
        entry.questItem = IsQuestItem(item);
        entry.listedForPersonalShop = i < 100 && MyShopItemIndex[i] != 0;
        entry.potionCount = item.sItemInfo.PotionCount > 0 ? item.sItemInfo.PotionCount : item.PotionCount;
        entry.levelRequirement = item.sItemInfo.Level;
        entry.weight = item.sItemInfo.Weight;
        entry.durabilityCurrent = item.sItemInfo.Durability[0];
        entry.durabilityMax = item.sItemInfo.Durability[1];
        entry.usable = item.sItemInfo.NotUseFlag == 0 && item.NotUseFlag == 0;
        PresentCi442(item,entry); // SUMMIT442A: CI quantity hint or invalid
        if (item.lpItem)
            entry.texture = TextureHandle(reinterpret_cast<std::uintptr_t>(item.lpItem));

        if (!entry.equipped)
        {
            if (entry.questItem)
            {
                // Quest items live in the same legacy array but are hidden by
                // adding 10000 to X while the quest panel is closed. Normalize
                // that storage detail into stable logical 6x3 coordinates.
                int normalizedX = item.x;
                while (normalizedX >= HiddenInventoryOffset)
                    normalizedX -= HiddenInventoryOffset;
                const int relativeX = normalizedX - LegacyBagX;
                const int relativeY = item.y - LegacyBagY;
                if (relativeX < 0 || relativeY < 0 ||
                    relativeX >= LegacyQuestColumns * LegacyQuestCell ||
                    relativeY >= LegacyQuestRows * LegacyQuestCell)
                    continue;
                entry.gridX = relativeX / LegacyQuestCell;
                entry.gridY = relativeY / LegacyQuestCell;
            }
            else
            {
                // Normal bag items are only interactive when their active A/B
                // page is resident in InvenItem and visible in the 12x6 grid.
                const int relativeX = item.x - LegacyBagX;
                const int relativeY = item.y - LegacyBagY;
                if (relativeX < 0 || relativeY < 0 ||
                    relativeX >= LegacyColumns * LegacyCell ||
                    relativeY >= LegacyRows * LegacyCell)
                    continue;
                entry.gridX = relativeX / LegacyCell;
                entry.gridY = relativeY / LegacyCell;
            }
        }

        out.items.push_back(std::move(entry));
    }

    if (MouseItem.Flag)
    {
        out.drag.active = true;
        out.drag.name = summit_inventory_name_428::Display(MouseItem.CODE, MouseItem.sItemInfo.ItemName); // SUMMIT428A
        out.drag.widthPixels = std::max(1, MouseItem.w);
        out.drag.heightPixels = std::max(1, MouseItem.h);
        if (MouseItem.lpItem)
            out.drag.texture = TextureHandle(reinterpret_cast<std::uintptr_t>(MouseItem.lpItem));
    }

    return out;
}

// SUMMIT427A: InvenItemTemp[0..79] holds the inactive legacy bag (A/B).
// Do not call ChangeABItem, toggle ArrowPosi or modify character state just to read.
std::vector<InventoryItemSnapshot> LegacyInventoryGateway::GetInactiveBagForCrafting427() const
{
    std::vector<InventoryItemSnapshot> result;
    if (!sinChar) return result;
    result.reserve(80);
    for(int index=0;index<80;++index) {
        const sITEM& item=cInvenTory.InvenItemTemp[index];
        if(!item.Flag || !item.CODE || item.ItemPosition!=0 || IsQuestItem(item))continue;
        const int relativeX=item.x-LegacyBagX, relativeY=item.y-LegacyBagY;
        if(relativeX<0 || relativeY<0 || relativeX>=LegacyColumns*LegacyCell || relativeY>=LegacyRows*LegacyCell)continue;
        InventoryItemSnapshot entry;
        entry.index=-1; // NEVER expose inactive indices as actionable selection
        entry.code=item.CODE;
        entry.name=item.sItemInfo.ItemName[0] ? item.sItemInfo.ItemName : item.ItemNameIndex;
        entry.potionCount=item.sItemInfo.PotionCount>0?item.sItemInfo.PotionCount:item.PotionCount;
        PresentCi442(item,entry); // SUMMIT442A: read-only B/A quantity
        if(item.lpItem)entry.texture=TextureHandle(reinterpret_cast<std::uintptr_t>(item.lpItem));
        result.push_back(std::move(entry));
    }
    return result;
}

std::vector<DetailLineSnapshot> LegacyInventoryGateway::GetItemDetails(int itemIndex) const
{
    std::vector<DetailLineSnapshot> out;
    if (!ValidInventoryIndex(itemIndex))
        return out;

    sITEM& item = cInvenTory.InvenItem[itemIndex];
    if (!item.Flag)
        return out;

    cItem.ShowItemInfo(&item, 0, itemIndex);
    const auto left = SplitLegacyLines(szInfoBuff);
    const auto right = SplitLegacyLines(szInfoBuff2);
    const std::size_t count = std::max(left.size(), right.size());
    out.reserve(count);

    for (std::size_t i = 0; i < count; ++i)
    {
        DetailLineSnapshot line;
        if (i < left.size()) line.left = left[i];
        if (i == 0 && !line.left.empty()) line.left = summit_inventory_name_428::Display(item.CODE, item.sItemInfo.ItemName); // SUMMIT428A
        if (i < right.size()) line.right = right[i];
        const int oneBased = static_cast<int>(i) + 1;
        if (i == 0)
            line.tone = DetailLineTone::Header;
        else if (IsLineIn(RedLine, 10, oneBased))
            line.tone = DetailLineTone::Danger;
        else if (IsLineIn(RequireLine, 10, oneBased))
            line.tone = DetailLineTone::Muted;
        else if (!line.right.empty())
            line.tone = DetailLineTone::Accent;
        else
            line.tone = DetailLineTone::Normal;

        if (!line.left.empty() || !line.right.empty())
            out.push_back(std::move(line));
    }
    const auto ci442=CiReading442(item);
    if(ci442.IsCi()) {
        DetailLineSnapshot amount;
        amount.left="Quantidade CI";
        amount.right=ci442.IsSuspicious()?"METADADOS INVALIDOS":
            ("x"+std::to_string(ci442.quantityHint));
        amount.tone=ci442.IsSuspicious()?DetailLineTone::Danger:DetailLineTone::Accent;
        out.push_back(std::move(amount));
        if(ci442.PreventLegacyMutation()) {
            DetailLineSnapshot warning;
            warning.left="Mover/consumir pilha: requer servidor";
            warning.tone=DetailLineTone::Muted;
            out.push_back(std::move(warning));
        }
    }
    return out;
}

bool LegacyInventoryGateway::BeginDrag(int itemIndex)
{
    if(gameplay::companions::v2::TradeClientFrozen()) return false;
    if (!ValidInventoryIndex(itemIndex) || MouseItem.Flag)
        return false;

    sITEM& item = cInvenTory.InvenItem[itemIndex];
    if (!item.Flag || UnsafeCiMutation442(item))
        return false; // SUMMIT442A: legacy drag cannot re-sign CI stack

    // A personal-shop listing is intentionally immutable until removed from
    // the sale list. PickUpInvenItem enforces this too; keep the adapter honest.
    if (itemIndex < 100 && MyShopItemIndex[itemIndex])
        return false;
    if (AmbiguousLegacyEquippedSelection(itemIndex))
        return false; // fail closed instead of accidentally removing another item

    // Selection on the V2 slot must not depend on the visual Costume button.
    const ScopedLegacyEquipmentHitTest hitTest(LegacySelectionSlot(item));
    const POINT center = ItemCenter(item);
    CursorGuard cursor;
    cursor.Set(center.x, center.y);
    const int previousOpenFlag = cInvenTory.OpenFlag;
    cInvenTory.OpenFlag = SIN_OPEN;
    cInvenTory.CheckMousePotionNumForm();
    const bool picked = cInvenTory.PickUpInvenItem(center.x, center.y, 1) != 0 && MouseItem.Flag;
    cInvenTory.ReFormMousePotionNum();
    cInvenTory.OpenFlag = previousOpenFlag;
    if (picked)
        cInvenTory.ReFormInvenItem();
    return picked;
}

bool LegacyInventoryGateway::DropAtLegacyPoint(int x, int y)
{
    if(gameplay::companions::v2::TradeClientFrozen()) return false;
    if (!MouseItem.Flag)
        return false;

    CursorGuard cursor;
    cursor.Set(x, y);
    const int previousOpenFlag = cInvenTory.OpenFlag;
    cInvenTory.OpenFlag = SIN_OPEN;
    InvenArrowPosi = 0;

    MouseItem.x = x - std::max(1, MouseItem.w / 2);
    MouseItem.y = y - std::max(1, MouseItem.h / 2);
    cInvenTory.SetInvenItemAreaCheck(&MouseItem);
    cInvenTory.LButtonDown(x, y);
    cInvenTory.LButtonUp(x, y);
    cInvenTory.ReFormMousePotionNum();
    const bool placed = MouseItem.Flag == 0;
    cInvenTory.OpenFlag = previousOpenFlag;
    return placed;
}

bool LegacyInventoryGateway::DropToBag(int column, int row)
{
    if (!MouseItem.Flag || column < 0 || row < 0 || column >= LegacyColumns || row >= LegacyRows)
        return false;

    const int widthCells = std::max(1, (MouseItem.w + LegacyCell - 1) / LegacyCell);
    const int heightCells = std::max(1, (MouseItem.h + LegacyCell - 1) / LegacyCell);
    if (column + widthCells > LegacyColumns || row + heightCells > LegacyRows)
        return false;

    // V2 grid coordinates describe the desired top-left cell. DropAtLegacyPoint()
    // receives a cursor position and recentres MouseItem around that cursor before
    // the legacy 22px snap. Using the centre of a single cell shifted multi-cell
    // items left/up; offset by half of the actual item instead.
    const int targetX = LegacyBagX + column * LegacyCell;
    const int targetY = LegacyBagY + row * LegacyCell;
    return DropAtLegacyPoint(targetX + std::max(1, MouseItem.w / 2),
                             targetY + std::max(1, MouseItem.h / 2));
}

bool LegacyInventoryGateway::DropToQuest(int column, int row)
{
    if (!MouseItem.Flag || column < 0 || row < 0 || column >= LegacyQuestColumns || row >= LegacyQuestRows)
        return false;
    if (!SetQuestItemsVisible(true))
        return false;

    const int widthCells = std::max(1, (MouseItem.w + LegacyQuestCell - 1) / LegacyQuestCell);
    const int heightCells = std::max(1, (MouseItem.h + LegacyQuestCell - 1) / LegacyQuestCell);
    if (column + widthCells > LegacyQuestColumns || row + heightCells > LegacyQuestRows)
        return false;

    const int targetX = LegacyBagX + column * LegacyQuestCell;
    const int targetY = LegacyBagY + row * LegacyQuestCell;
    return DropAtLegacyPoint(targetX + std::max(1, MouseItem.w / 2),
                             targetY + std::max(1, MouseItem.h / 2));
}

bool LegacyInventoryGateway::DropToEquipment(int equipmentSlot)
{
    if (!ValidEquipmentSlot(equipmentSlot) || !MouseItem.Flag)
        return false;

    // The user chose a distinct V2 slot. Validate the destination by item
    // class BEFORE translating to the overlapping legacy screen coordinate.
    if ((equipmentSlot == 3 && MouseItem.Class != ITEM_CLASS_ARMOR) ||
        (equipmentSlot == 14 && MouseItem.Class != ITEM_CLASS_COSTUME))
        return false;
    if ((sInven[equipmentSlot - 1].Position & MouseItem.Class) == 0)
        return false;

    const int declared = SummitEquipmentSlotByCode(MouseItem.CODE);
    if (declared && (declared != equipmentSlot ||
        MouseItem.Class != (declared == 14 ? ITEM_CLASS_COSTUME : ITEM_CLASS_ARMOR)))
        return false;
    if (equipmentSlot == 14 && !CostumeUseAllowed(MouseItem)) return false;
    const ScopedLegacyEquipmentHitTest hitTest(equipmentSlot);
    const RECT& rect = sInven[equipmentSlot - 1].Rect;
    return DropAtLegacyPoint((rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2);
}

bool LegacyInventoryGateway::CancelDrag()
{
    if (!MouseItem.Flag)
        return true;

    const int previousOpenFlag = cInvenTory.OpenFlag;
    cInvenTory.OpenFlag = SIN_OPEN;

    MouseItem.SetX = MouseItem.OldX;
    MouseItem.SetY = MouseItem.OldY;
    MouseItem.x = MouseItem.OldX;
    MouseItem.y = MouseItem.OldY;
    bool restored = false;
    // Cancelar nao pode reintroduzir no slot 14 um traje agora invalido.
    if (MouseItem.ItemPosition == 14 && !CostumeUseAllowed(MouseItem)) {
        MouseItem.ItemPosition = 0;
        MouseItem.SetModelPosi = 0;
        restored = cInvenTory.InvenEmptyAearCheck(&MouseItem) != 0 && !MouseItem.Flag;
    } else {
        const ScopedLegacyEquipmentHitTest hitTest(MouseItem.ItemPosition);
        restored = cInvenTory.LastSetInvenItem(&MouseItem) != 0;
        if (!restored && MouseItem.Flag) {
            MouseItem.ItemPosition = 0;
            restored = cInvenTory.AutoSetInvenItem(&MouseItem) != 0;
        }
    }

    cInvenTory.OpenFlag = previousOpenFlag;
    return restored || !MouseItem.Flag;
}

bool LegacyInventoryGateway::ActivateItem(int itemIndex)
{
    if (!ValidInventoryIndex(itemIndex) || MouseItem.Flag)
        return false;

    sITEM& item = cInvenTory.InvenItem[itemIndex];
    if (!item.Flag || AmbiguousLegacyEquippedSelection(itemIndex) || UnsafeCiMutation442(item))
        return false; // SUMMIT442A: no legacy activation on sealed/invalid CI stack

    // SUMMIT_V2_EXPLICIT_BODY_SLOT: clique direito e drag usam A MESMA operacao.
    // Apenas os itens de corpo no saco seguem esta via. Equipados e consumiveis
    // continuam no percurso original, com selecao legacy protegida.
    if (item.ItemPosition == 0 &&
        (item.Class == ITEM_CLASS_ARMOR || item.Class == ITEM_CLASS_COSTUME))
    {
        if (itemIndex < 100 && MyShopItemIndex[itemIndex]) return false;
        const int configured = SummitEquipmentSlotByCode(item.CODE);
        const int destination = configured ? configured :
            (item.Class == ITEM_CLASS_COSTUME ? 14 : 3);
        // Os registros legados com tipo antigo precisam de migracao controlada;
        // jamais equipar Uniforme como armadura apenas para o fazer funcionar.
        if (item.Class != (destination == 14 ? ITEM_CLASS_COSTUME : ITEM_CLASS_ARMOR))
            return false;
        if (!BeginDrag(itemIndex)) return false;
        if (DropToEquipment(destination)) return true;
        CancelDrag(); // Recoloca no saco em caso de falha.
        return false;
    }

    // Right-click selection of an equipped item must target its V2 widget,
    // regardless of the currently displayed outfit.
    const ScopedLegacyEquipmentHitTest hitTest(LegacySelectionSlot(item));
    CursorGuard cursor;
    const POINT center = ItemCenter(item);
    cursor.Set(center.x, center.y);
    const int previousOpenFlag = cInvenTory.OpenFlag;
    cInvenTory.OpenFlag = SIN_OPEN;

    cInvenTory.PickUpInvenItem(center.x, center.y, 0);
    const bool selected = SelectInvenItemIndex == itemIndex + 1;
    if (selected)
        cInvenTory.RButtonDown(center.x, center.y);

    cInvenTory.OpenFlag = previousOpenFlag;
    return selected;
}

bool LegacyInventoryGateway::SelectBagPage(int page)
{
    if (!sinChar || (page != 1 && page != 2))
        return false;
    const int current = sinChar->ArrowPosi[0] == 2 ? 2 : 1;
    if (current == page)
        return true;

    cInvenTory.ChangeSpecialItem(2);
    cInvenTory.ChangeABItem(page);
    return (sinChar->ArrowPosi[0] == page);
}

bool LegacyInventoryGateway::SelectWeaponSet(int set)
{
    if (!sinChar || MouseItem.Flag || (set != 1 && set != 2) || sinChangeSetFlag)
        return false;
    const int current = sinChar->ArrowPosi[1] == 4 ? 2 : 1;
    if (current == set)
        return true;

    // The original UI defers weapon-set swapping to the character update so
    // animation/motion restrictions remain identical to the legacy behavior.
    sinChangeSetFlag = set == 1 ? 3 : 4;
    return true;
}

bool LegacyInventoryGateway::SetQuestItemsVisible(bool visible)
{
    const bool current = SpecialItemShowFlag != 0;
    if (current == visible)
        return true;
    if (MouseItem.Flag)
        return false;
    if (visible)
        cInvenTory.ChangeSpecialItem();
    else
        cInvenTory.ChangeSpecialItem(2);
    return (SpecialItemShowFlag != 0) == visible;
}

bool LegacyInventoryGateway::OpenPersonalShopSales()
{
    if (MouseItem.Flag)
        return false;
    cInterFace.CheckAllBox(SIN_MYSHOP);
    return cMyShop.OpenFlag != 0 || MyShopSendButton != 0;
}

bool LegacyInventoryGateway::ToggleLock()
{
    if (MouseItem.Flag)
        return false;
    cInvenTory.setLock();
    return true;
}

bool LegacyInventoryGateway::ToggleCostumeMode()
{
    if (MouseItem.Flag)
        return false;
    return cInvenTory.setCostumer();
}
}
