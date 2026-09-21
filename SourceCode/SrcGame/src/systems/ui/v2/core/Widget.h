#pragma once

#include "systems/ui/v2/core/Geometry.h"
#include "systems/ui/v2/core/UIEvent.h"
#include "systems/ui/v2/core/UIId.h"

namespace ui::v2
{
class Container;
class UIRenderContext;

class Widget
{
public:
    explicit Widget(UIId id = InvalidUIId);
    virtual ~Widget() = default;

    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;
    Widget(Widget&&) = delete;
    Widget& operator=(Widget&&) = delete;

    virtual void Update(float deltaSeconds);
    virtual void Render(UIRenderContext& context);
    virtual UIEventResult OnEvent(const UIEvent& event);

    void SetBounds(Rect bounds) noexcept { bounds_ = bounds; }
    [[nodiscard]] Rect Bounds() const noexcept { return bounds_; }
    [[nodiscard]] Rect GlobalBounds() const noexcept;

    void SetVisible(bool value) noexcept { visible_ = value; }
    [[nodiscard]] bool IsVisible() const noexcept { return visible_; }

    void SetEnabled(bool value) noexcept { enabled_ = value; }
    [[nodiscard]] bool IsEnabled() const noexcept { return enabled_; }

    void SetHitTestVisible(bool value) noexcept { hitTestVisible_ = value; }
    [[nodiscard]] bool IsHitTestVisible() const noexcept { return hitTestVisible_; }

    void SetFocusable(bool value) noexcept { focusable_ = value; }
    [[nodiscard]] bool IsFocusable() const noexcept { return focusable_; }

    void SetOpacity(float value) noexcept;
    [[nodiscard]] float Opacity() const noexcept { return opacity_; }

    void SetZOrder(int value) noexcept { zOrder_ = value; }
    [[nodiscard]] int ZOrder() const noexcept { return zOrder_; }

    [[nodiscard]] UIId Id() const noexcept { return id_; }
    [[nodiscard]] Widget* Parent() noexcept { return parent_; }
    [[nodiscard]] const Widget* Parent() const noexcept { return parent_; }

    [[nodiscard]] bool HitTest(Point point) const noexcept;

protected:
    virtual void OnRender(UIRenderContext& context);

private:
    friend class Container;
    void SetParent(Widget* parent) noexcept { parent_ = parent; }

    UIId id_ = InvalidUIId;
    Widget* parent_ = nullptr;
    Rect bounds_{};
    bool visible_ = true;
    bool enabled_ = true;
    bool hitTestVisible_ = true;
    bool focusable_ = false;
    float opacity_ = 1.0f;
    int zOrder_ = 0;
};
}
