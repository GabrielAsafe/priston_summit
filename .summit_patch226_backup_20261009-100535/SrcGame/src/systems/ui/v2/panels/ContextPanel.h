#pragma once

#include <array>
#include <cstdint>
#include "systems/ui/v2/core/Container.h"
#include "systems/ui/v2/domain/IContextGateway.h"

namespace ui::v2
{
class Button;
class ConfirmationOverlay;
class NumericInputOverlay;

class ContextPanel final : public Container
{
public:
    explicit ContextPanel(IContextGateway& gateway, UIId id = InvalidUIId);

    void Update(float deltaSeconds) override;
    [[nodiscard]] bool IsContextActive() const noexcept { return snapshot_.valid; }
    bool AcceptInventoryItem(int inventoryIndex);
    [[nodiscard]] ContextKind Kind() const noexcept { return snapshot_.kind; }
    [[nodiscard]] bool HasModalInput() const noexcept;
    bool HandleModalKey(std::uint32_t key);

protected:
    UIEventResult OnEvent(const UIEvent& event) override;
    void OnRender(UIRenderContext& context) override;

private:
    struct ItemHit
    {
        int index = -1;
        std::size_t ordinal = 0;
        Rect rect{};
        bool interactive = false;
    };

    void Refresh();
    void LayoutActions();
    void LayoutPaging();
    [[nodiscard]] Rect ItemsAreaLocal() const noexcept;
    [[nodiscard]] int ActionStartY() const noexcept;
    [[nodiscard]] int DetailStartY() const noexcept;
    [[nodiscard]] int StatusStartY() const noexcept;
    [[nodiscard]] bool UsesSpatialGrid() const noexcept;
    [[nodiscard]] Rect SpatialGridRectLocal() const noexcept;
    [[nodiscard]] int ItemCapacity() const noexcept;
    [[nodiscard]] int ItemPageCount() const noexcept;
    [[nodiscard]] std::vector<ItemHit> ItemRects() const;
    [[nodiscard]] const ContextItemSnapshot* ItemAt(Point pointer) const noexcept;
    void LayoutModals();
    bool BeginActivation(int index);
    bool BeginInventoryAcceptance(int inventoryIndex);

    IContextGateway& gateway_;
    ContextSnapshot snapshot_{};
    std::array<Button*, 6> actionButtons_{};
    Button* previousPageButton_ = nullptr;
    Button* nextPageButton_ = nullptr;
    ConfirmationOverlay* confirmation_ = nullptr;
    NumericInputOverlay* numericInput_ = nullptr;
    int itemPage_ = 0;
    int hoveredIndex_ = -1;
    float refreshAccumulator_ = 0.0f;
};
}
