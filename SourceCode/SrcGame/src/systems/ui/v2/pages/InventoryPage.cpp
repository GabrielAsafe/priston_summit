#include "systems/ui/v2/pages/InventoryPage.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <string>
#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/domain/IContextGateway.h"
#include "systems/ui/v2/panels/ContextPanel.h"
#include "systems/ui/v2/widgets/TextLayout.h"
#include "systems/ui/v2/widgets/DetailTextLayout.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/Button.h"
#include "systems/commerce/coin_shop/CoinShop.h"
#include "systems/commerce/coin_shop/CoinShop_T.h"

namespace ui::v2
{
namespace
{
constexpr int Cell = 30;
constexpr int QuestCell = 52;
constexpr int BagX = 346;
constexpr int BagY = 92;
constexpr int EquipmentX = 24;
constexpr int EquipmentY = 78;
constexpr int DetailLineHeight = 18;

struct EquipmentSlotLayout
{
    int slot;
    int x;
    int y;
    int w;
    int h;
    const char* label;
};

constexpr std::array<EquipmentSlotLayout, 16> EquipmentSlots = {{
    {1, 16, 44, 58, 98, "Mao D"},
    {2, 170, 44, 58, 98, "Mao E"},
    {3, 93, 44, 66, 98, "Armadura"},
    {4, 16, 166, 42, 42, "Amuleto"},
    {5, 64, 166, 42, 42, "Anel D"},
    {6, 112, 166, 42, 42, "Anel E"},
    {7, 160, 166, 42, 42, "Sheltom"},
    {8, 16, 218, 54, 54, "Bracelete"},
    {9, 78, 218, 54, 54, "Luvas"},
    {10, 140, 218, 54, 54, "Botas"},
    {11, 16, 292, 42, 42, "P1"},
    {12, 64, 292, 42, 42, "P2"},
    {13, 112, 292, 42, 42, "P3"},
    {14, 202, 218, 66, 98, "Traje"},
    {15, 202, 166, 42, 42, "Brinco E"},
    {16, 250, 166, 42, 42, "Brinco D"}
}};

Color ToneColor(DetailLineTone tone)
{
    switch (tone)
    {
        case DetailLineTone::Muted: return UITheme::TextMuted;
        case DetailLineTone::Accent: return UITheme::Accent;
        case DetailLineTone::Positive: return UITheme::Positive;
        case DetailLineTone::Danger: return UITheme::Danger;
        case DetailLineTone::Header: return UITheme::Text;
        case DetailLineTone::Normal:
        default: return UITheme::Text;
    }
}

void DrawBorder(IUIRenderer& r, const Rect& rect, Color color)
{
    r.DrawRect({rect.x, rect.y, rect.width, 1}, color);
    r.DrawRect({rect.x, rect.y + rect.height - 1, rect.width, 1}, color);
    r.DrawRect({rect.x, rect.y, 1, rect.height}, color);
    r.DrawRect({rect.x + rect.width - 1, rect.y, 1, rect.height}, color);
}

void LogInventoryPointer(const char* action, Point pointer, const Rect& root, int itemIndex, int bagColumn, int bagRow, int equipmentSlot, bool result)
{
#ifdef _WIN32
    static int logCount = 0;
    if (logCount >= 64)
        return;
    ++logCount;
    if (FILE* file = std::fopen("ui_v2_inventory_input.log", "a"))
    {
        std::fprintf(file, "%s pointer=%d,%d root=%d,%d,%d,%d item=%d bag=%d,%d equip=%d result=%d\n",
                     action ? action : "event", pointer.x, pointer.y, root.x, root.y, root.width, root.height,
                     itemIndex, bagColumn, bagRow, equipmentSlot, result ? 1 : 0);
        std::fclose(file);
    }
#else
    (void)action; (void)pointer; (void)root; (void)itemIndex; (void)bagColumn;
    (void)bagRow; (void)equipmentSlot; (void)result;
#endif
}
}

InventoryPage::InventoryPage(IInventoryGateway& gateway, IContextGateway& contextGateway, UIId id)
    : Container(id), viewModel_(gateway)
{
    auto& bagA = AddChild<Button>(310, "Mochila A");
    bagA.SetBounds({346, 52, 92, 30});
    bagA.SetZOrder(2);
    bagA.SetOnClick([this](const UIEvent&) { SelectBagPage(1); });
    bagAButton_ = &bagA;

    auto& bagB = AddChild<Button>(311, "Mochila B");
    bagB.SetBounds({444, 52, 92, 30});
    bagB.SetZOrder(2);
    bagB.SetOnClick([this](const UIEvent&) { SelectBagPage(2); });
    bagBButton_ = &bagB;

    auto& quest = AddChild<Button>(312, "Itens de Missão");
    quest.SetBounds({542, 52, 140, 30});
    quest.SetZOrder(2);
    quest.SetOnClick([this](const UIEvent&) { SelectQuestItems(); });
    questButton_ = &quest;

    auto& set1 = AddChild<Button>(313, "Armas 1");
    set1.SetBounds({24, 52, 92, 30});
    set1.SetZOrder(2);
    set1.SetOnClick([this](const UIEvent&) { viewModel_.SelectWeaponSet(1); });
    weaponSet1Button_ = &set1;

    auto& set2 = AddChild<Button>(314, "Armas 2");
    set2.SetBounds({122, 52, 92, 30});
    set2.SetZOrder(2);
    set2.SetOnClick([this](const UIEvent&) { viewModel_.SelectWeaponSet(2); });
    weaponSet2Button_ = &set2;

    auto& personal = AddChild<Button>(315, "Venda pessoal");
    personal.SetBounds({24, 418, 112, 28});
    personal.SetZOrder(2);
    personal.SetOnClick([this](const UIEvent&) { viewModel_.OpenPersonalShopSales(); });
    personalShopButton_ = &personal;

    auto& lock = AddChild<Button>(320, "Travar itens");
    lock.SetBounds({142, 418, 112, 28});
    lock.SetZOrder(2);
    lock.SetOnClick([this](const UIEvent&) { viewModel_.ToggleLock(); });
    lockButton_ = &lock;

    auto& costume = AddChild<Button>(321, "Traje");
    costume.SetBounds({260, 418, 72, 28});
    costume.SetZOrder(2);
    costume.SetOnClick([this](const UIEvent&) { viewModel_.ToggleCostume(); });
    costumeButton_ = &costume;

    auto& cancel = AddChild<Button>(322, "Cancelar movimento");
    cancel.SetBounds({24, 418, 308, 28});
    cancel.SetZOrder(2);
    cancel.SetOnClick([this](const UIEvent&) { viewModel_.CancelDrag(); });
    cancelDragButton_ = &cancel;

    auto& context = AddChild<ContextPanel>(contextGateway, 330);
    context.SetBounds({14, 48, 314, 414});
    context.SetZOrder(1);
    context.SetVisible(true);
    contextPanel_ = &context;

    viewModel_.Refresh();
}

bool InventoryPage::HasModalInput() const noexcept
{
    return contextPanel_ && contextPanel_->HasModalInput();
}

bool InventoryPage::HandleModalKey(std::uint32_t key)
{
    return contextPanel_ && contextPanel_->HasModalInput() ? contextPanel_->HandleModalKey(key) : false;
}

void InventoryPage::SelectBagPage(int page)
{
    // The original inventory allows switching A/B while an item is on the
    // cursor. Preserve that behavior so the item can be moved between bags.
    if (questMode_)
        viewModel_.SetQuestItemsVisible(false);
    questMode_ = false;
    hoveredIndex_ = -1;
    viewModel_.SelectBagPage(page);
}

void InventoryPage::SelectQuestItems()
{
    if (viewModel_.Snapshot().drag.active)
        return;
    if (!questMode_ && viewModel_.SetQuestItemsVisible(true))
    {
        questMode_ = true;
        hoveredIndex_ = -1;
    }
}

void InventoryPage::Update(float deltaSeconds)
{
    // PATCH 227: only request missing wallet balances, not the item catalogs.
    // Recheck at most once per 10s if a response has not arrived.
    currencyFetchAccumulator_ += deltaSeconds;
    if (currencyFetchAccumulator_ >= 10.0f)
    {
        currencyFetchAccumulator_ = 0.0f;
        if (!cCoinShop.BalanceReceived) (void)cCoinShop.RequestBalanceForUI();
        if (!cCoinShop_T.BalanceReceived) (void)cCoinShop_T.RequestBalanceForUI();
    }
    refreshAccumulator_ += deltaSeconds;
    if (refreshAccumulator_ >= 0.10f)
    {
        refreshAccumulator_ = 0.0f;
        viewModel_.Refresh();
    }

    const auto& snapshot = viewModel_.Snapshot();
    bagAButton_->SetSelected(!questMode_ && snapshot.activeBagPage == 1);
    bagBButton_->SetSelected(!questMode_ && snapshot.activeBagPage == 2);
    questButton_->SetSelected(questMode_);
    weaponSet1Button_->SetSelected(snapshot.activeWeaponSet == 1);
    weaponSet2Button_->SetSelected(snapshot.activeWeaponSet == 2);
    lockButton_->SetText(snapshot.locked ? "Destravar itens" : "Travar itens");
    lockButton_->SetSelected(snapshot.locked);
    costumeButton_->SetSelected(snapshot.costumeMode);
    personalShopButton_->SetSelected(snapshot.personalShopOpen);
    cancelDragButton_->SetEnabled(snapshot.drag.active);

    // Update ContextPanel first so ownership/visibility reacts in the same frame
    // that a merchant/crafting context opens. The previous order left the
    // inventory footer buttons visible for one frame over the V2 action row.
    Container::Update(deltaSeconds);

    const bool contextActive = contextPanel_ && contextPanel_->IsContextActive();
    weaponSet1Button_->SetVisible(!contextActive);
    weaponSet2Button_->SetVisible(!contextActive);
    const bool showNormalActions = !contextActive && !snapshot.drag.active;
    personalShopButton_->SetVisible(showNormalActions);
    lockButton_->SetVisible(showNormalActions);
    costumeButton_->SetVisible(showNormalActions);
    cancelDragButton_->SetVisible(!contextActive && snapshot.drag.active);
}

Rect InventoryPage::BagRectLocal() const noexcept
{
    const auto& s = viewModel_.Snapshot();
    if (questMode_)
        return {BagX, BagY, s.questColumns * QuestCell, s.questRows * QuestCell};
    return {BagX, BagY, s.columns * Cell, s.rows * Cell};
}

Rect InventoryPage::EquipmentSlotRectLocal(int slot) const noexcept
{
    for (const auto& entry : EquipmentSlots)
        if (entry.slot == slot)
            return {EquipmentX + entry.x, EquipmentY + entry.y, entry.w, entry.h};
    return {};
}

Rect InventoryPage::DetailsRectLocal() const noexcept
{
    const int pageHeight = std::max(1, Bounds().height);
    const int width = viewModel_.Snapshot().columns * Cell;
    const int top = 292;
    return {BagX, top, width, std::max(120, pageHeight - top - 12)};
}

void InventoryPage::RefreshDetails(int itemIndex)
{
    if (itemIndex < 0)
    {
        detailItemIndex_ = -1;
        detailLines_.clear();
        detailScroll_ = 0;
        return;
    }

    if (detailItemIndex_ != itemIndex)
        detailScroll_ = 0;
    detailItemIndex_ = itemIndex;
    detailLines_ = viewModel_.DetailsFor(itemIndex);
    detailScroll_ = std::clamp(detailScroll_, 0, MaxDetailScroll());
}

int InventoryPage::MaxDetailScroll() const noexcept
{
    const Rect panel = DetailsRectLocal();
    const Rect content{panel.x + 10, panel.y + 30, std::max(24, panel.width - 20), std::max(0, panel.height - 40)};
    const auto rows = LayoutDetailRows(detailLines_, content.width);
    const int totalHeight = static_cast<int>(rows.size()) * DetailLineHeight;
    return std::max(0, totalHeight - content.height);
}

bool InventoryPage::ItemVisibleInCurrentView(const InventoryItemSnapshot& item) const noexcept
{
    if (item.equipped)
        return true;
    return questMode_ ? item.questItem : !item.questItem;
}

Rect InventoryPage::ItemRectLocal(const InventoryItemSnapshot& item) const noexcept
{
    if (item.equipmentSlot > 0)
        return EquipmentSlotRectLocal(item.equipmentSlot);
    if (!ItemVisibleInCurrentView(item) || item.gridX < 0 || item.gridY < 0)
        return {};

    if (questMode_)
        return {BagX + item.gridX * QuestCell, BagY + item.gridY * QuestCell, QuestCell, QuestCell};

    const int cellsW = std::max(1, (item.widthPixels + 21) / 22);
    const int cellsH = std::max(1, (item.heightPixels + 21) / 22);
    return {BagX + item.gridX * Cell, BagY + item.gridY * Cell, cellsW * Cell, cellsH * Cell};
}

const InventoryItemSnapshot* InventoryPage::ItemAt(Point pointer) const noexcept
{
    const Rect global = GlobalBounds();
    const Point local{pointer.x - global.x, pointer.y - global.y};
    for (const auto& item : viewModel_.Snapshot().items)
    {
        if (contextPanel_ && contextPanel_->IsContextActive() && item.equipped)
            continue;
        if (!ItemVisibleInCurrentView(item))
            continue;
        const Rect rect = ItemRectLocal(item);
        if (!rect.IsEmpty() && rect.Contains(local))
            return &item;
    }
    return nullptr;
}

InventoryPage::HitResult InventoryPage::Hit(Point pointer) const noexcept
{
    HitResult hit;
    const Rect global = GlobalBounds();
    const Point local{pointer.x - global.x, pointer.y - global.y};

    if (const auto* item = ItemAt(pointer))
        hit.itemIndex = item->index;

    const Rect bag = BagRectLocal();
    if (bag.Contains(local))
    {
        const int cell = questMode_ ? QuestCell : Cell;
        const int columns = questMode_ ? viewModel_.Snapshot().questColumns : viewModel_.Snapshot().columns;
        const int rows = questMode_ ? viewModel_.Snapshot().questRows : viewModel_.Snapshot().rows;
        hit.bagColumn = std::clamp((local.x - bag.x) / cell, 0, columns - 1);
        hit.bagRow = std::clamp((local.y - bag.y) / cell, 0, rows - 1);
    }

    if (!(contextPanel_ && contextPanel_->IsContextActive()))
    for (const auto& slot : EquipmentSlots)
    {
        if (EquipmentSlotRectLocal(slot.slot).Contains(local))
        {
            hit.equipmentSlot = slot.slot;
            break;
        }
    }
    return hit;
}

UIEventResult InventoryPage::OnEvent(const UIEvent& event)
{
    lastPointer_ = event.pointer;
    const UIEventResult childResult = Container::OnEvent(event);
    if (childResult != UIEventResult::Ignored)
        return childResult;

    const Rect root = GlobalBounds();
    const Rect detailsLocal = DetailsRectLocal();
    const Rect detailsGlobal{root.x + detailsLocal.x, root.y + detailsLocal.y,
                             detailsLocal.width, detailsLocal.height};

    if (event.type == UIEventType::Wheel && detailsGlobal.Contains(event.pointer))
    {
        const int direction = event.wheelDelta > 0 ? -1 : (event.wheelDelta < 0 ? 1 : 0);
        detailScroll_ = std::clamp(detailScroll_ + direction * DetailLineHeight * 2, 0, MaxDetailScroll());
        return UIEventResult::Handled;
    }

    if (event.type == UIEventType::PointerDown && event.button == MouseButton::Left)
    {
        const int maxScroll = MaxDetailScroll();
        const Rect scrollTrack{detailsGlobal.x + detailsGlobal.width - 10,
                               detailsGlobal.y + 30, 8, std::max(1, detailsGlobal.height - 40)};
        if (maxScroll > 0 && scrollTrack.Contains(event.pointer))
        {
            const int relative = std::clamp(event.pointer.y - scrollTrack.y, 0, scrollTrack.height);
            detailScroll_ = relative * maxScroll / std::max(1, scrollTrack.height);
            return UIEventResult::Handled;
        }
    }

    if (event.type == UIEventType::PointerMove)
    {
        const int previous = hoveredIndex_;
        if (const auto* item = ItemAt(event.pointer))
            hoveredIndex_ = item->index;
        else
            hoveredIndex_ = -1;
        if (hoveredIndex_ != previous)
        {
            if (hoveredIndex_ >= 0)
                RefreshDetails(hoveredIndex_);
            else if (viewModel_.SelectedIndex() >= 0)
                RefreshDetails(viewModel_.SelectedIndex());
            else
                RefreshDetails(-1);
        }
        return HitTest(event.pointer) ? UIEventResult::Handled : UIEventResult::Ignored;
    }

    if (event.type == UIEventType::PointerDown && HitTest(event.pointer))
    {
        const HitResult hit = Hit(event.pointer);
        const bool dragging = viewModel_.Snapshot().drag.active;
        LogInventoryPointer("pointer-down", event.pointer, GlobalBounds(), hit.itemIndex,
                            hit.bagColumn, hit.bagRow, hit.equipmentSlot, false);

        if (event.button == MouseButton::Right)
        {
            if (!dragging && hit.itemIndex >= 0)
            {
                viewModel_.Select(hit.itemIndex);
                RefreshDetails(hit.itemIndex);
                // In an NPC merchant, either mouse button only selects the
                // inventory item; the V2 Vender button performs the transaction.
                // Crafting/warehouse/trade continue using their explicit
                // context acceptance paths, with original validations intact.
                const bool merchantSelection = contextPanel_ && contextPanel_->IsContextActive() &&
                                               contextPanel_->Kind() == ContextKind::Merchant;
                if (merchantSelection)
                    contextPanel_->SelectInventoryForMerchant(hit.itemIndex);
                const bool activated = merchantSelection ? true :
                    ((contextPanel_ && contextPanel_->IsContextActive())
                        ? contextPanel_->AcceptInventoryItem(hit.itemIndex)
                        : viewModel_.Activate(hit.itemIndex));
                LogInventoryPointer(contextPanel_ && contextPanel_->IsContextActive() ? "context-accept" : "activate",
                                    event.pointer, GlobalBounds(), hit.itemIndex,
                                    hit.bagColumn, hit.bagRow, hit.equipmentSlot, activated);
            }
            else if (dragging)
            {
                const bool cancelled = viewModel_.CancelDrag();
                LogInventoryPointer("cancel-drag", event.pointer, GlobalBounds(), hit.itemIndex,
                                    hit.bagColumn, hit.bagRow, hit.equipmentSlot, cancelled);
            }
            return UIEventResult::Handled;
        }

        if (event.button == MouseButton::Left)
        {
            if (dragging)
            {
                if (hit.equipmentSlot > 0)
                    viewModel_.DropToEquipment(hit.equipmentSlot);
                else if (hit.bagColumn >= 0)
                {
                    // The drag preview is centred on the pointer. Convert that
                    // centre position back to the item's top-left grid cell so
                    // multi-cell items do not treat the hovered cell as their
                    // top-left corner. This also lets large items reach the
                    // right/bottom edges without requiring the cursor to sit on
                    // their upper-left cell.
                    const auto& drag = viewModel_.Snapshot().drag;
                    const Rect global = GlobalBounds();
                    const Point local{event.pointer.x - global.x, event.pointer.y - global.y};
                    const Rect bag = BagRectLocal();
                    const int cell = questMode_ ? QuestCell : Cell;
                    const int columns = questMode_ ? viewModel_.Snapshot().questColumns : viewModel_.Snapshot().columns;
                    const int rows = questMode_ ? viewModel_.Snapshot().questRows : viewModel_.Snapshot().rows;
                    const int cellsW = questMode_ ? 1 : std::max(1, (drag.widthPixels + 21) / 22);
                    const int cellsH = questMode_ ? 1 : std::max(1, (drag.heightPixels + 21) / 22);

                    const auto roundCell = [cell](int value)
                    {
                        return value >= 0
                            ? (value + cell / 2) / cell
                            : -((-value + cell / 2) / cell);
                    };

                    const int centredLeft = (local.x - bag.x) - (cellsW * cell) / 2;
                    const int centredTop = (local.y - bag.y) - (cellsH * cell) / 2;
                    const int dropColumn = std::clamp(roundCell(centredLeft), 0, std::max(0, columns - cellsW));
                    const int dropRow = std::clamp(roundCell(centredTop), 0, std::max(0, rows - cellsH));

                    if (questMode_)
                        viewModel_.DropToQuest(dropColumn, dropRow);
                    else
                        viewModel_.DropToBag(dropColumn, dropRow);
                }
                return UIEventResult::Handled;
            }

            if (hit.itemIndex >= 0)
            {
                viewModel_.Select(hit.itemIndex);
                RefreshDetails(hit.itemIndex);
                // Contextual commerce/crafting uses right-click to feed the
                // original validated operation. Keep left-click as selection
                // so an item cannot get stuck on the cursor behind the panel.
                if (contextPanel_ && contextPanel_->IsContextActive())
                {
                    if (contextPanel_->Kind() == ContextKind::Merchant)
                        contextPanel_->SelectInventoryForMerchant(hit.itemIndex);
                    return UIEventResult::Handled;
                }
                const bool began = viewModel_.BeginDrag(hit.itemIndex);
                LogInventoryPointer("begin-drag", event.pointer, GlobalBounds(), hit.itemIndex,
                                    hit.bagColumn, hit.bagRow, hit.equipmentSlot, began);
                return UIEventResult::Handled;
            }
        }
    }

    return HitTest(event.pointer) && event.IsPointerEvent() ? UIEventResult::Handled : UIEventResult::Ignored;
}

void InventoryPage::CancelOutstandingDrag()
{
    if (viewModel_.Snapshot().drag.active)
        viewModel_.CancelDrag();
    if (questMode_)
    {
        viewModel_.SetQuestItemsVisible(false);
        questMode_ = false;
    }
}

void InventoryPage::DrawBag(UIRenderContext& context)
{
    const Rect root = GlobalBounds();
    const Rect bagLocal = BagRectLocal();
    const Rect bag{root.x + bagLocal.x, root.y + bagLocal.y, bagLocal.width, bagLocal.height};
    auto& r = context.Renderer();
    const int cell = questMode_ ? QuestCell : Cell;
    const int columns = questMode_ ? viewModel_.Snapshot().questColumns : viewModel_.Snapshot().columns;
    const int rows = questMode_ ? viewModel_.Snapshot().questRows : viewModel_.Snapshot().rows;

    r.DrawTextUtf8(FontHandle{}, questMode_ ? "ITENS DE QUEST" : "MOCHILA",
                   {bag.x, bag.y - 30, bag.width, 22},
                   TextStyle{UITheme::TextMuted, true, TextAlign::Left});
    r.DrawRect(bag, UITheme::Slot);
    for (int x = 0; x <= columns; ++x)
        r.DrawRect({bag.x + x * cell, bag.y, 1, bag.height}, UITheme::BorderSoft);
    for (int y = 0; y <= rows; ++y)
        r.DrawRect({bag.x, bag.y + y * cell, bag.width, 1}, UITheme::BorderSoft);

    for (const auto& item : viewModel_.Snapshot().items)
    {
        if (item.equipmentSlot != 0 || !ItemVisibleInCurrentView(item) || item.gridX < 0 || item.gridY < 0)
            continue;
        Rect rect = ItemRectLocal(item);
        rect.x += root.x;
        rect.y += root.y;
        const bool hovered = item.index == hoveredIndex_;
        const bool selected = item.index == viewModel_.SelectedIndex();
        if (hovered || selected)
            r.DrawRect(rect, hovered ? UITheme::SlotHover : UITheme::SurfacePressed);
        if (item.texture.IsValid())
        {
            const int inset = questMode_ ? 6 : 2;
            r.DrawImage(item.texture, {rect.x + inset, rect.y + inset, rect.width - inset * 2, rect.height - inset * 2}, nullptr);
        }
        else
            r.DrawTextUtf8(FontHandle{}, "?", rect, TextStyle{UITheme::TextMuted, true, TextAlign::Center});
        if (hovered || selected)
            DrawBorder(r, rect, hovered ? UITheme::Accent : UITheme::Border);
        if (item.potionCount > 1)
            r.DrawTextUtf8(FontHandle{}, std::to_string(item.potionCount), {rect.x + 2, rect.y + rect.height - 18, rect.width - 4, 16},
                           TextStyle{UITheme::Text, true, TextAlign::Right});
        if (item.listedForPersonalShop)
        {
            DrawBorder(r, rect, UITheme::Accent);
            r.DrawTextUtf8(FontHandle{}, "VENDA", {rect.x + 2, rect.y + 2, rect.width - 4, 14},
                           TextStyle{UITheme::Accent, true, TextAlign::Center});
        }
        if (!item.usable)
            DrawBorder(r, rect, UITheme::Danger);
    }
}

void InventoryPage::DrawEquipment(UIRenderContext& context)
{
    const Rect root = GlobalBounds();
    auto& r = context.Renderer();
    r.DrawTextUtf8(FontHandle{}, "EQUIPAMENTO", {root.x + EquipmentX, root.y + EquipmentY + 10, 290, 22},
                   TextStyle{UITheme::TextMuted, true, TextAlign::Left});

    for (const auto& slot : EquipmentSlots)
    {
        Rect rect = EquipmentSlotRectLocal(slot.slot);
        rect.x += root.x;
        rect.y += root.y;
        r.DrawRect(rect, UITheme::Slot);
        DrawBorder(r, rect, UITheme::BorderSoft);
        r.DrawTextUtf8(FontHandle{}, slot.label, {rect.x + 2, rect.y + rect.height - 18, rect.width - 4, 16},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Center});
    }

    for (const auto& item : viewModel_.Snapshot().items)
    {
        if (item.equipmentSlot <= 0)
            continue;
        Rect rect = ItemRectLocal(item);
        rect.x += root.x;
        rect.y += root.y;
        const bool hovered = item.index == hoveredIndex_;
        if (hovered)
            r.DrawRect(rect, UITheme::SlotHover);
        if (item.texture.IsValid())
            r.DrawImage(item.texture, {rect.x + 4, rect.y + 4, rect.width - 8, rect.height - 8}, nullptr);
        if (hovered)
            DrawBorder(r, rect, UITheme::Accent);
        if (!item.usable)
            DrawBorder(r, rect, UITheme::Danger);
    }
}

void InventoryPage::DrawDetails(UIRenderContext& context)
{
    const Rect root = GlobalBounds();
    const Rect local = DetailsRectLocal();
    const Rect panel{root.x + local.x, root.y + local.y, local.width, local.height};
    auto& r = context.Renderer();

    r.DrawRect(panel, UITheme::Surface);
    DrawBorder(r, panel, UITheme::BorderSoft);

    const InventoryItemSnapshot* item = nullptr;
    const int effectiveIndex = hoveredIndex_ >= 0 ? hoveredIndex_ : viewModel_.SelectedIndex();
    if (effectiveIndex >= 0)
    {
        for (const auto& candidate : viewModel_.Snapshot().items)
            if (candidate.index == effectiveIndex) { item = &candidate; break; }
    }

    if (!item)
    {
        r.DrawTextUtf8(FontHandle{}, "DETALHES DO ITEM",
                       {panel.x + 10, panel.y + 6, panel.width - 20, 20},
                       TextStyle{UITheme::TextMuted, true, TextAlign::Left});
        const std::string line1 = questMode_ ? "Itens especiais de quest do personagem."
                                             : "Passe o mouse ou selecione um item para ver todos os atributos.";
        const auto help = WrapTextApprox(line1, panel.width - 24, 4);
        int y = panel.y + 36;
        for (const auto& line : help)
        {
            r.DrawTextUtf8(FontHandle{}, line, {panel.x + 12, y, panel.width - 24, DetailLineHeight},
                           TextStyle{UITheme::TextMuted, false, TextAlign::Left});
            y += DetailLineHeight;
        }
        return;
    }

    if (detailItemIndex_ != item->index)
        RefreshDetails(item->index);

    r.DrawTextUtf8(FontHandle{}, "DETALHES",
                   {panel.x + 10, panel.y + 6, panel.width - 20, 20},
                   TextStyle{UITheme::TextMuted, true, TextAlign::Left});

    const Rect content{panel.x + 10, panel.y + 30, std::max(24, panel.width - 20), std::max(0, panel.height - 40)};
    const auto rows = LayoutDetailRows(detailLines_, content.width);
    r.PushClip(content);
    int y = content.y - detailScroll_;
    for (const auto& line : rows)
    {
        if (y + DetailLineHeight >= content.y && y <= content.y + content.height)
        {
            const Color color = ToneColor(line.tone);
            const bool bold = line.tone == DetailLineTone::Header || line.tone == DetailLineTone::Positive;
            const int rightWidth = line.right.empty() ? 0 : std::min(96, content.width / 3);
            const int leftWidth = content.width - rightWidth - (rightWidth ? 6 : 0);
            r.DrawTextUtf8(FontHandle{}, line.left, {content.x, y, leftWidth, DetailLineHeight},
                           TextStyle{color, bold, TextAlign::Left});
            if (!line.right.empty())
                r.DrawTextUtf8(FontHandle{}, line.right,
                               {content.x + leftWidth + 6, y, rightWidth, DetailLineHeight},
                               TextStyle{UITheme::Accent, bold, TextAlign::Right});
        }
        y += DetailLineHeight;
    }
    r.PopClip();

    const int maxScroll = MaxDetailScroll();
    if (maxScroll > 0)
    {
        const Rect track{panel.x + panel.width - 6, content.y, 3, content.height};
        r.DrawRect(track, UITheme::BorderSoft);
        const int totalHeight = static_cast<int>(rows.size()) * DetailLineHeight;
        const int thumbHeight = std::max(20, content.height * content.height / std::max(content.height, totalHeight));
        const int thumbTravel = std::max(0, content.height - thumbHeight);
        const int thumbY = content.y + (maxScroll ? detailScroll_ * thumbTravel / maxScroll : 0);
        r.DrawRect({track.x, thumbY, track.width, thumbHeight}, UITheme::AccentMuted);
    }
}

void InventoryPage::DrawDrag(UIRenderContext& context)
{
    const auto& drag = viewModel_.Snapshot().drag;
    if (!drag.active)
        return;

    auto& r = context.Renderer();
    const int cell = questMode_ ? QuestCell : Cell;
    const int cellsW = questMode_ ? 1 : std::max(1, (drag.widthPixels + 21) / 22);
    const int cellsH = questMode_ ? 1 : std::max(1, (drag.heightPixels + 21) / 22);
    const int w = cellsW * cell;
    const int h = cellsH * cell;
    const Rect rect{lastPointer_.x - w / 2, lastPointer_.y - h / 2, w, h};
    r.DrawRect(rect, Color{30, 35, 44, 210});
    if (drag.texture.IsValid())
        r.DrawImage(drag.texture, rect, nullptr);
    DrawBorder(r, rect, UITheme::Accent);
}

void InventoryPage::OnRender(UIRenderContext& context)
{
    const Rect root = GlobalBounds();
    auto& r = context.Renderer();
    r.DrawRect(root, UITheme::WindowBg);

    const auto& snapshot = viewModel_.Snapshot();
    if (!snapshot.valid)
    {
        r.DrawTextUtf8(FontHandle{}, "Inventario indisponivel.", {root.x + 20, root.y + 20, root.width - 40, 24},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        return;
    }

    r.DrawTextUtf8(FontHandle{}, "INVENTARIO", {root.x + 24, root.y + 18, 180, 24},
                   TextStyle{UITheme::Text, true, TextAlign::Left});
    r.DrawTextUtf8(FontHandle{}, "Ouro: " + std::to_string(snapshot.gold), {root.x + 346, root.y + 18, 180, 24},
                   TextStyle{UITheme::Accent, true, TextAlign::Left});
    r.DrawTextUtf8(FontHandle{}, "Peso: " + std::to_string(snapshot.weightCurrent) + "/" + std::to_string(snapshot.weightMax),
                   {root.x + 536, root.y + 18, 190, 24}, TextStyle{UITheme::TextMuted, false, TextAlign::Left});
    // PATCH 227: balances last reported by their respective server wallets.
    r.DrawTextUtf8(FontHandle{}, "SCoin: " + (cCoinShop.BalanceReceived ? std::to_string(cCoinShop.Coin) : std::string("--")),
                   {root.x + 346, root.y + 36, 160, 15}, TextStyle{UITheme::TextMuted, false, TextAlign::Left});
    r.DrawTextUtf8(FontHandle{}, "TCoin: " + (cCoinShop_T.BalanceReceived ? std::to_string(cCoinShop_T.Coin) : std::string("--")),
                   {root.x + 526, root.y + 36, 160, 15}, TextStyle{UITheme::TextMuted, false, TextAlign::Left});

    if (!(contextPanel_ && contextPanel_->IsContextActive()))
        DrawEquipment(context);
    DrawBag(context);
    DrawDetails(context);

    if (!(contextPanel_ && contextPanel_->IsContextActive()))
        r.DrawTextUtf8(FontHandle{}, snapshot.locked ? "Itens travados: movimento bloqueado." : "Travar itens impede mover itens por engano.",
                       {root.x + 24, root.y + 452, 318, 16}, TextStyle{UITheme::TextMuted, false, TextAlign::Left});
    if (contextPanel_ && contextPanel_->IsContextActive())
        r.DrawTextUtf8(FontHandle{}, "Contexto ativo: botão direito na mochila envia itens para a operação.",
                       {root.x + 346, root.y + 430, 360, 20}, TextStyle{UITheme::Accent, false, TextAlign::Left});
    else if (snapshot.blacksmithOpen)
        r.DrawTextUtf8(FontHandle{}, "Ferreiro ativo: reparar/vender usa as regras originais.",
                       {root.x + 346, root.y + 430, 360, 20}, TextStyle{UITheme::Accent, false, TextAlign::Left});

    DrawDrag(context);
}
}
