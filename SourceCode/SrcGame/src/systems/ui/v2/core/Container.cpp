#include "systems/ui/v2/core/Container.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"

namespace ui::v2
{
Widget& Container::AddChild(std::unique_ptr<Widget> child)
{
    child->SetParent(this);
    Widget& reference = *child;
    children_.push_back(std::move(child));
    std::stable_sort(children_.begin(), children_.end(), [](const auto& a, const auto& b)
    {
        return a->ZOrder() < b->ZOrder();
    });
    return reference;
}

bool Container::RemoveChild(UIId id)
{
    const auto it = std::find_if(children_.begin(), children_.end(), [id](const auto& child)
    {
        return child->Id() == id;
    });
    if (it == children_.end())
        return false;
    (*it)->SetParent(nullptr);
    children_.erase(it);
    return true;
}

void Container::ClearChildren()
{
    for (auto& child : children_)
        child->SetParent(nullptr);
    children_.clear();
}

Widget* Container::FindChild(UIId id) noexcept
{
    for (auto& child : children_)
    {
        if (child->Id() == id)
            return child.get();
        if (auto* container = dynamic_cast<Container*>(child.get()))
            if (auto* nested = container->FindChild(id))
                return nested;
    }
    return nullptr;
}

const Widget* Container::FindChild(UIId id) const noexcept
{
    for (const auto& child : children_)
    {
        if (child->Id() == id)
            return child.get();
        if (const auto* container = dynamic_cast<const Container*>(child.get()))
            if (const auto* nested = container->FindChild(id))
                return nested;
    }
    return nullptr;
}

void Container::Update(float deltaSeconds)
{
    Widget::Update(deltaSeconds);
    if (!IsVisible())
        return;
    for (auto& child : children_)
        if (child->IsVisible())
            child->Update(deltaSeconds);
}

void Container::Render(UIRenderContext& context)
{
    if (!IsVisible())
        return;
    Widget::Render(context);
    for (auto& child : children_)
        child->Render(context);
}

UIEventResult Container::OnEvent(const UIEvent& event)
{
    if (!IsVisible() || !IsEnabled())
        return UIEventResult::Ignored;

    for (auto it = children_.rbegin(); it != children_.rend(); ++it)
    {
        Widget& child = **it;
        if (!child.IsVisible() || !child.IsEnabled())
            continue;
        // PointerUp is broadcast down the active branch even when the cursor
        // is outside a child so stateful controls can cancel a press cleanly.
        if (event.IsPointerEvent() && event.type != UIEventType::PointerUp && !child.HitTest(event.pointer))
            continue;
        const UIEventResult result = child.OnEvent(event);
        if (result != UIEventResult::Ignored)
            return result;
    }
    return Widget::OnEvent(event);
}
}
