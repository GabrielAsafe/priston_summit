#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "systems/ui/v2/core/UIEvent.h"
#include "systems/ui/v2/core/UIId.h"
#include "systems/ui/v2/core/Window.h"

namespace ui::v2
{
enum class UIWindowLayer : std::uint8_t
{
    Hud,
    NormalWindow,
    Popup,
    Modal,
    Tooltip,
    Cursor,
    Debug
};

class UIWindowManager
{
public:
    bool Register(WindowId id, std::unique_ptr<Window> window, UIWindowLayer layer = UIWindowLayer::NormalWindow);
    bool Unregister(WindowId id);

    bool Open(WindowId id);
    bool Close(WindowId id);
    bool Toggle(WindowId id);
    [[nodiscard]] bool IsOpen(WindowId id) const;
    [[nodiscard]] bool HasOpenWindows() const noexcept;

    void BringToFront(WindowId id);
    [[nodiscard]] Window* Get(WindowId id) noexcept;
    [[nodiscard]] const Window* Get(WindowId id) const noexcept;

    void Update(float deltaSeconds);
    void Render(UIRenderContext& context);
    UIEventResult Dispatch(const UIEvent& event);

    void CloseAll();

private:
    struct Entry
    {
        WindowId id = InvalidWindowId;
        std::unique_ptr<Window> window;
        UIWindowLayer layer = UIWindowLayer::NormalWindow;
        std::uint64_t order = 0;
        bool open = false;
    };

    Entry* Find(WindowId id) noexcept;
    const Entry* Find(WindowId id) const noexcept;
    std::vector<Entry*> OrderedEntries(bool topFirst);

    std::vector<Entry> entries_;
    Window* capturedWindow_ = nullptr;
    std::uint64_t nextOrder_ = 1;
};
}
