#include "systems/ui/v2/windows/UIWindowManager.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"

#include <algorithm>

namespace ui::v2
{
bool UIWindowManager::Register(WindowId id, std::unique_ptr<Window> window, UIWindowLayer layer)
{
    if (!window || id == InvalidWindowId || Find(id))
        return false;
    window->SetVisible(false);
    entries_.push_back({id, std::move(window), layer, nextOrder_++, false});
    return true;
}

bool UIWindowManager::Unregister(WindowId id)
{
    const auto it = std::find_if(entries_.begin(), entries_.end(), [id](const Entry& entry) { return entry.id == id; });
    if (it == entries_.end())
        return false;
    if (capturedWindow_ == it->window.get())
        capturedWindow_ = nullptr;
    entries_.erase(it);
    return true;
}

bool UIWindowManager::Open(WindowId id)
{
    Entry* entry = Find(id);
    if (!entry)
        return false;
    entry->open = true;
    entry->window->SetVisible(true);
    BringToFront(id);
    return true;
}

bool UIWindowManager::Close(WindowId id)
{
    Entry* entry = Find(id);
    if (!entry)
        return false;
    if (capturedWindow_ == entry->window.get())
        capturedWindow_ = nullptr;
    entry->open = false;
    entry->window->SetVisible(false);
    return true;
}

bool UIWindowManager::Toggle(WindowId id)
{
    return IsOpen(id) ? Close(id) : Open(id);
}

bool UIWindowManager::IsOpen(WindowId id) const
{
    const Entry* entry = Find(id);
    return entry && entry->open;
}

bool UIWindowManager::HasOpenWindows() const noexcept
{
    for (const auto& entry : entries_)
        if (entry.open)
            return true;
    return false;
}

void UIWindowManager::BringToFront(WindowId id)
{
    Entry* entry = Find(id);
    if (entry)
        entry->order = nextOrder_++;
}

Window* UIWindowManager::Get(WindowId id) noexcept
{
    Entry* entry = Find(id);
    return entry ? entry->window.get() : nullptr;
}

const Window* UIWindowManager::Get(WindowId id) const noexcept
{
    const Entry* entry = Find(id);
    return entry ? entry->window.get() : nullptr;
}

void UIWindowManager::Update(float deltaSeconds)
{
    for (Entry* entry : OrderedEntries(false))
        if (entry->open)
            entry->window->Update(deltaSeconds);
}

void UIWindowManager::Render(UIRenderContext& context)
{
    for (Entry* entry : OrderedEntries(false))
        if (entry->open)
            entry->window->Render(context);
}

UIEventResult UIWindowManager::Dispatch(const UIEvent& event)
{
    if (capturedWindow_)
    {
        const UIEventResult result = capturedWindow_->OnEvent(event);
        if (event.type == UIEventType::PointerUp)
            capturedWindow_ = nullptr;
        return result == UIEventResult::Ignored ? UIEventResult::Handled : result;
    }

    for (Entry* entry : OrderedEntries(true))
    {
        if (!entry->open)
            continue;
        Window& window = *entry->window;
        if (event.IsPointerEvent() && !window.HitTest(event.pointer))
            continue;

        const UIEventResult result = window.OnEvent(event);
        if (result == UIEventResult::Ignored)
            continue;
        BringToFront(entry->id);
        // Capture the owning window for the duration of every pointer press.
        // This guarantees a PointerUp reaches controls even when the cursor
        // leaves the button/window before release, preventing stuck pressed
        // states. Explicit Captured is still used by movable windows.
        if (result == UIEventResult::Captured || event.type == UIEventType::PointerDown)
            capturedWindow_ = &window;
        return result;
    }
    return UIEventResult::Ignored;
}

void UIWindowManager::CloseAll()
{
    capturedWindow_ = nullptr;
    for (auto& entry : entries_)
    {
        entry.open = false;
        entry.window->SetVisible(false);
    }
}

UIWindowManager::Entry* UIWindowManager::Find(WindowId id) noexcept
{
    for (auto& entry : entries_)
        if (entry.id == id)
            return &entry;
    return nullptr;
}

const UIWindowManager::Entry* UIWindowManager::Find(WindowId id) const noexcept
{
    for (const auto& entry : entries_)
        if (entry.id == id)
            return &entry;
    return nullptr;
}

std::vector<UIWindowManager::Entry*> UIWindowManager::OrderedEntries(bool topFirst)
{
    std::vector<Entry*> ordered;
    ordered.reserve(entries_.size());
    for (auto& entry : entries_)
        ordered.push_back(&entry);

    std::stable_sort(ordered.begin(), ordered.end(), [topFirst](const Entry* a, const Entry* b)
    {
        const auto layerA = static_cast<std::uint8_t>(a->layer);
        const auto layerB = static_cast<std::uint8_t>(b->layer);
        if (layerA != layerB)
            return topFirst ? layerA > layerB : layerA < layerB;
        return topFirst ? a->order > b->order : a->order < b->order;
    });
    return ordered;
}
}
