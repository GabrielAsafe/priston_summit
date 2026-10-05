#include "systems/ui/v2/panels/ContextPanel.h"

#include <algorithm>
#include <cmath>
#include <string>
#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/Button.h"
#include "systems/ui/v2/widgets/ConfirmationOverlay.h"
#include "systems/ui/v2/widgets/NumericInputOverlay.h"
#include "systems/ui/v2/widgets/TextLayout.h"

namespace ui::v2
{
namespace
{
void Border(IUIRenderer& r, const Rect& rect, Color color)
{
    r.DrawRect({rect.x, rect.y, rect.width, 1}, color);
    r.DrawRect({rect.x, rect.y + rect.height - 1, rect.width, 1}, color);
    r.DrawRect({rect.x, rect.y, 1, rect.height}, color);
    r.DrawRect({rect.x + rect.width - 1, rect.y, 1, rect.height}, color);
}
}

ContextPanel::ContextPanel(IContextGateway& gateway, UIId id) : Container(id), gateway_(gateway)
{
    for (std::size_t i = 0; i < actionButtons_.size(); ++i)
    {
        auto& button = AddChild<Button>(700 + static_cast<UIId>(i), "");
        button.SetVisible(false);
        button.SetOnClick([this, i](const UIEvent&)
        {
            if (i < snapshot_.actions.size())
                gateway_.InvokeAction(snapshot_.actions[i].id);
            Refresh();
        });
        actionButtons_[i] = &button;
    }

    auto& previous = AddChild<Button>(790, "<");
    previous.SetVisible(false);
    previous.SetOnClick([this](const UIEvent&)
    {
        if (itemPage_ > 0) --itemPage_;
        hoveredIndex_ = -1;
        LayoutPaging();
    });
    previousPageButton_ = &previous;

    auto& next = AddChild<Button>(791, ">");
    next.SetVisible(false);
    next.SetOnClick([this](const UIEvent&)
    {
        const int pages = ItemPageCount();
        if (itemPage_ + 1 < pages) ++itemPage_;
        hoveredIndex_ = -1;
        LayoutPaging();
    });
    nextPageButton_ = &next;

    auto& confirmation = AddChild<ConfirmationOverlay>(792);
    confirmation.SetZOrder(100);
    confirmation_ = &confirmation;

    auto& numeric = AddChild<NumericInputOverlay>(796);
    numeric.SetZOrder(101);
    numericInput_ = &numeric;

    Refresh();
    LayoutModals();
}

void ContextPanel::Refresh()
{
    const ContextKind oldKind = snapshot_.kind;
    snapshot_ = gateway_.GetSnapshot();
    if (snapshot_.kind != oldKind)
        itemPage_ = 0;
    itemPage_ = std::clamp(itemPage_, 0, std::max(0, ItemPageCount() - 1));
    LayoutActions();
    LayoutPaging();
}

void ContextPanel::LayoutActions()
{
    const int width = Bounds().width;
    const int count = std::min<int>(static_cast<int>(snapshot_.actions.size()), static_cast<int>(actionButtons_.size()));
    const int columns = count > 3 ? 2 : 1;
    const int gap = 6;
    const int buttonWidth = columns == 2 ? std::max(70, (width - 20 - gap) / 2) : std::max(80, width - 20);
    const int startY = std::max(300, Bounds().height - 104);

    for (std::size_t i = 0; i < actionButtons_.size(); ++i)
    {
        Button* button = actionButtons_[i];
        if (static_cast<int>(i) >= count)
        {
            button->SetVisible(false);
            continue;
        }
        const auto& action = snapshot_.actions[i];
        const int col = static_cast<int>(i) % columns;
        const int row = static_cast<int>(i) / columns;
        button->SetVisible(true);
        button->SetEnabled(action.enabled);
        button->SetSelected(action.selected);
        button->SetText(action.label);
        button->SetBounds({10 + col * (buttonWidth + gap), startY + row * 34, buttonWidth, 28});
    }
}

void ContextPanel::LayoutPaging()
{
    if (!previousPageButton_ || !nextPageButton_)
        return;
    const int pages = ItemPageCount();
    const bool visible = pages > 1;
    const int width = std::max(1, Bounds().width);
    previousPageButton_->SetVisible(visible);
    nextPageButton_->SetVisible(visible);
    previousPageButton_->SetEnabled(itemPage_ > 0);
    nextPageButton_->SetEnabled(itemPage_ + 1 < pages);
    previousPageButton_->SetBounds({width - 82, 7, 32, 24});
    nextPageButton_->SetBounds({width - 44, 7, 32, 24});
}

int ContextPanel::ItemCapacity() const noexcept
{
    if (UsesSpatialGrid())
        return std::max(1, snapshot_.gridColumns * snapshot_.gridRows);
    const Rect area = ItemsAreaLocal();
    int columns = 5;
    if (snapshot_.kind == ContextKind::Warehouse)
        columns = 8;
    else if (snapshot_.kind == ContextKind::Trade)
        columns = 5;
    else if (snapshot_.kind == ContextKind::Merchant || snapshot_.kind == ContextKind::PlayerMerchant ||
             snapshot_.kind == ContextKind::PersonalShop)
        columns = 6;
    else if (snapshot_.kind == ContextKind::CoinShop || snapshot_.kind == ContextKind::TimeCoinShop)
        columns = 4;
    else if (!snapshot_.items.empty() && snapshot_.items.size() <= 5)
        columns = static_cast<int>(snapshot_.items.size());
    columns = std::max(1, columns);
    const int cell = std::max(28, std::min(46, area.width / columns));
    const int rows = std::max(1, area.height / cell);
    return std::max(1, columns * rows);
}

int ContextPanel::ItemPageCount() const noexcept
{
    if (UsesSpatialGrid())
        return 1;
    if (snapshot_.items.empty())
        return 1;
    const int capacity = ItemCapacity();
    return (static_cast<int>(snapshot_.items.size()) + capacity - 1) / capacity;
}

void ContextPanel::Update(float deltaSeconds)
{
    refreshAccumulator_ += deltaSeconds;
    if (refreshAccumulator_ >= 0.10f)
    {
        refreshAccumulator_ = 0.0f;
        Refresh();
    }
    Container::Update(deltaSeconds);
}

Rect ContextPanel::ItemsAreaLocal() const noexcept
{
    const int bottomReserve = snapshot_.statusText.empty() ? 198 : 244;
    return {10, 80, std::max(1, Bounds().width - 20), std::max(1, Bounds().height - bottomReserve)};
}

bool ContextPanel::UsesSpatialGrid() const noexcept
{
    return snapshot_.gridColumns > 0 && snapshot_.gridRows > 0;
}

Rect ContextPanel::SpatialGridRectLocal() const noexcept
{
    const Rect area = ItemsAreaLocal();
    if (!UsesSpatialGrid())
        return area;
    const int cell = std::max(12, std::min(area.width / snapshot_.gridColumns, area.height / snapshot_.gridRows));
    const int width = cell * snapshot_.gridColumns;
    const int height = cell * snapshot_.gridRows;
    return {area.x + std::max(0, (area.width - width) / 2),
            area.y + std::max(0, (area.height - height) / 2), width, height};
}

std::vector<ContextPanel::ItemHit> ContextPanel::ItemRects() const
{
    std::vector<ItemHit> result;
    if (!snapshot_.valid || snapshot_.items.empty())
        return result;

    if (UsesSpatialGrid())
    {
        const Rect grid = SpatialGridRectLocal();
        const int cell = std::max(1, grid.width / snapshot_.gridColumns);
        result.reserve(snapshot_.items.size());
        for (std::size_t ordinal = 0; ordinal < snapshot_.items.size(); ++ordinal)
        {
            const auto& item = snapshot_.items[ordinal];
            if (item.gridX < 0 || item.gridY < 0)
                continue;
            const int gx = std::clamp(item.gridX, 0, snapshot_.gridColumns - 1);
            const int gy = std::clamp(item.gridY, 0, snapshot_.gridRows - 1);
            const int gw = std::clamp(item.gridWidth, 1, snapshot_.gridColumns - gx);
            const int gh = std::clamp(item.gridHeight, 1, snapshot_.gridRows - gy);
            result.push_back(ItemHit{item.index, ordinal,
                                     {grid.x + gx * cell, grid.y + gy * cell, gw * cell, gh * cell},
                                     item.interactive});
        }
        return result;
    }

    const Rect area = ItemsAreaLocal();
    int columns = 5;
    if (snapshot_.kind == ContextKind::Warehouse)
        columns = 8;
    else if (snapshot_.kind == ContextKind::Trade)
        columns = 5;
    else if (snapshot_.kind == ContextKind::Merchant || snapshot_.kind == ContextKind::PlayerMerchant ||
             snapshot_.kind == ContextKind::PersonalShop)
        columns = 6;
    else if (snapshot_.kind == ContextKind::Restoration)
        columns = 5;
    else if (snapshot_.kind == ContextKind::CoinShop || snapshot_.kind == ContextKind::TimeCoinShop)
        columns = 4;
    else if (snapshot_.items.size() <= 5)
        columns = static_cast<int>(snapshot_.items.size());

    columns = std::max(1, columns);
    const int cell = std::max(28, std::min(46, area.width / columns));
    const int maxRows = std::max(1, area.height / cell);
    const int capacity = std::max(1, columns * maxRows);
    const int begin = std::min(static_cast<int>(snapshot_.items.size()), itemPage_ * capacity);
    const int endOrdinal = std::min(static_cast<int>(snapshot_.items.size()), begin + capacity);

    result.reserve(std::max(0, endOrdinal - begin));
    for (int ordinal = begin; ordinal < endOrdinal; ++ordinal)
    {
        const int localIndex = ordinal - begin;
        const int col = localIndex % columns;
        const int row = localIndex / columns;
        result.push_back(ItemHit{snapshot_.items[ordinal].index, static_cast<std::size_t>(ordinal),
                                 {area.x + col * cell, area.y + row * cell, cell - 3, cell - 3},
                                 snapshot_.items[ordinal].interactive});
    }
    return result;
}

const ContextItemSnapshot* ContextPanel::ItemAt(Point pointer) const noexcept
{
    const Rect root = GlobalBounds();
    const Point local{pointer.x - root.x, pointer.y - root.y};
    const auto rects = ItemRects();
    for (const auto& hit : rects)
        if (hit.rect.Contains(local) && hit.ordinal < snapshot_.items.size())
            return &snapshot_.items[hit.ordinal];
    return nullptr;
}


void ContextPanel::LayoutModals()
{
    const Rect bounds = Bounds();
    const Rect full{0, 0, std::max(1, bounds.width), std::max(1, bounds.height)};
    if (confirmation_) confirmation_->SetBounds(full);
    if (numericInput_) numericInput_->SetBounds(full);
}

bool ContextPanel::HasModalInput() const noexcept
{
    return (confirmation_ && confirmation_->IsOpen()) || (numericInput_ && numericInput_->IsOpen());
}

bool ContextPanel::HandleModalKey(std::uint32_t key)
{
    if (numericInput_ && numericInput_->IsOpen()) return numericInput_->HandleKey(key);
    if (confirmation_ && confirmation_->IsOpen()) return confirmation_->HandleKey(key);
    return false;
}

bool ContextPanel::BeginActivation(int index)
{
    const ContextInputRequest request = gateway_.ItemActivationRequest(index);
    if (request.kind == ContextInputKind::Confirmation && confirmation_)
    {
        confirmation_->Show(request.title, request.message, [this, index]()
        {
            gateway_.ActivateItemResolved(index, 1);
            Refresh();
        });
        return true;
    }
    if (request.kind == ContextInputKind::Number && numericInput_)
    {
        numericInput_->ShowWithOption(request.title, request.message, request.minimum, request.maximum, request.initial,
                                      request.primaryOption, request.alternateOption,
                                      [this, index](int value, bool alternate)
        {
            gateway_.ActivateItemResolvedOption(index, value, alternate);
            Refresh();
        });
        return true;
    }
    const bool result = gateway_.ActivateItem(index);
    Refresh();
    return result;
}

bool ContextPanel::BeginInventoryAcceptance(int inventoryIndex)
{
    const ContextInputRequest request = gateway_.InventoryAcceptanceRequest(inventoryIndex);
    if (request.kind == ContextInputKind::Confirmation && confirmation_)
    {
        confirmation_->Show(request.title, request.message, [this, inventoryIndex]()
        {
            gateway_.AcceptInventoryItemResolved(inventoryIndex, 1);
            Refresh();
        });
        return true;
    }
    if (request.kind == ContextInputKind::Number && numericInput_)
    {
        numericInput_->ShowWithOption(request.title, request.message, request.minimum, request.maximum, request.initial,
                                      request.primaryOption, request.alternateOption,
                                      [this, inventoryIndex](int value, bool alternate)
        {
            gateway_.AcceptInventoryItemResolvedOption(inventoryIndex, value, alternate);
            Refresh();
        });
        return true;
    }
    const bool result = gateway_.AcceptInventoryItem(inventoryIndex);
    Refresh();
    return result;
}

bool ContextPanel::AcceptInventoryItem(int inventoryIndex)
{
    if (!snapshot_.valid || inventoryIndex < 0 || HasModalInput())
        return false;
    return BeginInventoryAcceptance(inventoryIndex);
}

UIEventResult ContextPanel::OnEvent(const UIEvent& event)
{
    LayoutModals();
    const UIEventResult child = Container::OnEvent(event);
    if (child != UIEventResult::Ignored)
        return child;
    if (!snapshot_.valid)
        return UIEventResult::Ignored;

    if (event.type == UIEventType::PointerMove)
    {
        const auto* item = ItemAt(event.pointer);
        hoveredIndex_ = item ? item->index : -1;
        return HitTest(event.pointer) ? UIEventResult::Handled : UIEventResult::Ignored;
    }

    if (event.type == UIEventType::PointerDown && HitTest(event.pointer))
    {
        if (const auto* item = ItemAt(event.pointer))
        {
            if (item->interactive)
                BeginActivation(item->index);
        }
        return UIEventResult::Handled;
    }

    return HitTest(event.pointer) && event.IsPointerEvent() ? UIEventResult::Handled : UIEventResult::Ignored;
}

void ContextPanel::OnRender(UIRenderContext& context)
{
    LayoutModals();
    if (!snapshot_.valid)
        return;

    const Rect root = GlobalBounds();
    auto& r = context.Renderer();
    r.DrawRect(root, UITheme::Surface);
    Border(r, root, UITheme::Border);

    const int pages = ItemPageCount();
    const int titleReserve = pages > 1 ? 94 : 24;
    r.DrawTextUtf8(FontHandle{}, EllipsizeText(snapshot_.title, root.width - titleReserve, 8),
                   {root.x + 12, root.y + 10, root.width - titleReserve, 24},
                   TextStyle{UITheme::Text, true, TextAlign::Left});

    const auto subtitle = WrapTextApprox(snapshot_.subtitle, root.width - 24, 2, 7);
    for (std::size_t i = 0; i < subtitle.size(); ++i)
        r.DrawTextUtf8(FontHandle{}, subtitle[i], {root.x + 12, root.y + 36 + static_cast<int>(i) * 18, root.width - 24, 18},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Left});

    if (pages > 1)
        r.DrawTextUtf8(FontHandle{}, std::to_string(itemPage_ + 1) + "/" + std::to_string(pages),
                       {root.x + root.width - 120, root.y + 10, 32, 24},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Right});

    if (UsesSpatialGrid() && snapshot_.showEmptyGrid)
    {
        const Rect localGrid = SpatialGridRectLocal();
        const Rect grid{root.x + localGrid.x, root.y + localGrid.y, localGrid.width, localGrid.height};
        const int cell = std::max(1, grid.width / snapshot_.gridColumns);
        r.DrawRect(grid, UITheme::Slot);
        for (int x = 0; x <= snapshot_.gridColumns; ++x)
            r.DrawRect({grid.x + x * cell, grid.y, 1, grid.height}, UITheme::BorderSoft);
        for (int y = 0; y <= snapshot_.gridRows; ++y)
            r.DrawRect({grid.x, grid.y + y * cell, grid.width, 1}, UITheme::BorderSoft);
    }

    // Craft/Aging use explicit logical slots instead of the generic full-grid
    // background.  This restores the visual outline from the old interfaces:
    // one main-item area, twelve individual stone sockets and the optional
    // special-stone socket. Aging may also provide the original gray sheltom
    // hints for the required combination.
    if (UsesSpatialGrid() && !snapshot_.slots.empty())
    {
        const Rect localGrid = SpatialGridRectLocal();
        const Rect grid{root.x + localGrid.x, root.y + localGrid.y, localGrid.width, localGrid.height};
        const int cell = std::max(1, grid.width / snapshot_.gridColumns);
        for (const auto& slot : snapshot_.slots)
        {
            const int gx = std::clamp(slot.gridX, 0, snapshot_.gridColumns - 1);
            const int gy = std::clamp(slot.gridY, 0, snapshot_.gridRows - 1);
            const int gw = std::clamp(slot.gridWidth, 1, snapshot_.gridColumns - gx);
            const int gh = std::clamp(slot.gridHeight, 1, snapshot_.gridRows - gy);
            const Rect rect{grid.x + gx * cell, grid.y + gy * cell, gw * cell, gh * cell};
            r.DrawRect(rect, UITheme::Slot);
            const Color border = slot.special ? UITheme::Warning : (slot.required ? UITheme::Accent : UITheme::Border);
            Border(r, rect, border);
            if (slot.hintTexture.IsValid())
            {
                const int inset = std::max(2, cell / 8);
                r.DrawImage(slot.hintTexture,
                            {rect.x + inset, rect.y + inset, std::max(1, rect.width - inset * 2), std::max(1, rect.height - inset * 2)},
                            nullptr);
            }
            if (slot.gridWidth > 1 && !slot.label.empty())
                r.DrawTextUtf8(FontHandle{}, slot.label, {rect.x + 4, rect.y + 4, rect.width - 8, 18},
                               TextStyle{UITheme::TextMuted, false, TextAlign::Left});
            else if (slot.special && !slot.label.empty())
                r.DrawTextUtf8(FontHandle{}, "E", {rect.x, rect.y + std::max(0, rect.height - 17), rect.width, 15},
                               TextStyle{UITheme::Warning, true, TextAlign::Center});
        }
    }

    const auto rects = ItemRects();
    for (const auto& hit : rects)
    {
        if (hit.ordinal >= snapshot_.items.size())
            continue;
        const auto& item = snapshot_.items[hit.ordinal];
        Rect rect = hit.rect;
        rect.x += root.x;
        rect.y += root.y;
        const bool hovered = item.index == hoveredIndex_;
        r.DrawRect(rect, hovered ? UITheme::SurfaceHover : (item.selected ? UITheme::SurfacePressed : UITheme::Slot));
        Color border = item.remote ? UITheme::AccentMuted : ((hovered || item.selected) ? UITheme::Accent : UITheme::BorderSoft);
        if (!item.classCompatible)
            border = UITheme::Danger;
        else if (!item.requirementsMet)
            border = UITheme::Warning;
        Border(r, rect, border);
        if (item.texture.IsValid())
            r.DrawImage(item.texture, {rect.x + 3, rect.y + 3, std::max(1, rect.width - 6), std::max(1, rect.height - 6)}, nullptr);
        if (!item.classCompatible)
            r.DrawRect({rect.x, rect.y + rect.height - 4, rect.width, 4}, UITheme::Danger);
        else if (!item.requirementsMet)
            r.DrawRect({rect.x, rect.y + rect.height - 4, rect.width, 4}, UITheme::Warning);
        else if (!item.interactive)
            r.DrawRect({rect.x, rect.y + rect.height - 3, rect.width, 3}, UITheme::Disabled);
    }

    if (!snapshot_.statusText.empty())
        r.DrawTextUtf8(FontHandle{}, EllipsizeText(snapshot_.statusText, root.width - 24),
                       {root.x + 12, root.y + root.height - 202, root.width - 24, 18},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Left});

    const ContextItemSnapshot* hovered = nullptr;
    for (const auto& item : snapshot_.items)
        if (item.index == hoveredIndex_) { hovered = &item; break; }
    if (hovered)
    {
        std::string info = hovered->name;
        if (hovered->price > 0)
            info += "  |  " + std::to_string(hovered->price) + " " + snapshot_.currency;
        if (!hovered->caption.empty())
            info += "  |  " + hovered->caption;
        if (!hovered->restriction.empty())
            info += "  |  " + hovered->restriction;
        // PATCH 188: merchant details are semantic content, not a one-line label.
        // Keep price/caption/requirements readable instead of truncating them with
        // ellipsis. Reserve three wrapped lines above the currency/actions footer.
        const auto detailLines = WrapTextApprox(info, root.width - 24, 3, 7);
        for (std::size_t line = 0; line < detailLines.size(); ++line)
            r.DrawTextUtf8(FontHandle{}, detailLines[line],
                           {root.x + 12, root.y + root.height - 178 + static_cast<int>(line) * 18,
                            root.width - 24, 18},
                           TextStyle{UITheme::Text, false, TextAlign::Left});
    }

    std::string money = snapshot_.currency + ": " + std::to_string(snapshot_.money);
    if (snapshot_.fee > 0)
        money += "   Custo: " + std::to_string(snapshot_.fee) + " " + snapshot_.feeCurrency;
    r.DrawTextUtf8(FontHandle{}, EllipsizeText(money, root.width - 24),
                   {root.x + 12, root.y + root.height - 122, root.width - 24, 20},
                   TextStyle{UITheme::Accent, true, TextAlign::Left});
}
}
