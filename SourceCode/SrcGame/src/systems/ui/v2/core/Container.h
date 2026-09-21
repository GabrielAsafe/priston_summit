#pragma once

#include <algorithm>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "systems/ui/v2/core/Widget.h"

namespace ui::v2
{
class Container : public Widget
{
public:
    using Widget::Widget;
    ~Container() override = default;

    template<class T, class... Args>
    T& AddChild(Args&&... args)
    {
        static_assert(std::is_base_of<Widget, T>::value, "T must derive from ui::v2::Widget");
        auto child = std::make_unique<T>(std::forward<Args>(args)...);
        T& reference = *child;
        AddChild(std::move(child));
        return reference;
    }

    Widget& AddChild(std::unique_ptr<Widget> child);
    bool RemoveChild(UIId id);
    void ClearChildren();

    [[nodiscard]] Widget* FindChild(UIId id) noexcept;
    [[nodiscard]] const Widget* FindChild(UIId id) const noexcept;
    [[nodiscard]] std::size_t ChildCount() const noexcept { return children_.size(); }

    void Update(float deltaSeconds) override;
    void Render(UIRenderContext& context) override;
    UIEventResult OnEvent(const UIEvent& event) override;

protected:
    [[nodiscard]] const std::vector<std::unique_ptr<Widget>>& Children() const noexcept { return children_; }

private:
    std::vector<std::unique_ptr<Widget>> children_;
};
}
