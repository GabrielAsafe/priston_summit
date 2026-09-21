#pragma once

#include <memory>
#include "systems/ui/v2/input/UIFocusManager.h"
#include "systems/ui/v2/input/UIPointerCapture.h"
#include "systems/ui/v2/windows/UIWindowManager.h"

namespace ui::v2
{
class IUIRenderer;
class UIInputRouter;

class UIRuntime final
{
public:
    UIRuntime();
    ~UIRuntime();

    UIRuntime(const UIRuntime&) = delete;
    UIRuntime& operator=(const UIRuntime&) = delete;

    bool Initialize(std::unique_ptr<IUIRenderer> renderer);
    void Shutdown();

    void Update(float deltaSeconds);
    void Render();
    bool HandleEvent(const UIEvent& event);

    void OnResolutionChanged(int width, int height) noexcept;
    void OnDeviceLost();
    void OnDeviceReset();

    void SetEnabled(bool value) noexcept { enabled_ = value; }
    [[nodiscard]] bool IsEnabled() const noexcept { return enabled_; }
    [[nodiscard]] bool IsInitialized() const noexcept { return initialized_; }

    [[nodiscard]] int ViewportWidth() const noexcept { return viewportWidth_; }
    [[nodiscard]] int ViewportHeight() const noexcept { return viewportHeight_; }

    [[nodiscard]] UIWindowManager& Windows() noexcept { return windows_; }
    [[nodiscard]] UIFocusManager& Focus() noexcept { return focus_; }
    [[nodiscard]] UIPointerCapture& PointerCapture() noexcept { return pointerCapture_; }

private:
    std::unique_ptr<IUIRenderer> renderer_;
    std::unique_ptr<UIInputRouter> inputRouter_;
    UIWindowManager windows_;
    UIFocusManager focus_;
    UIPointerCapture pointerCapture_;
    bool initialized_ = false;
    bool enabled_ = true;
    int viewportWidth_ = 0;
    int viewportHeight_ = 0;
};
}
