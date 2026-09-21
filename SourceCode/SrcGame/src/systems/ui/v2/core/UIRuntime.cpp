#include "systems/ui/v2/core/UIRuntime.h"
#include "systems/ui/v2/input/UIInputRouter.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"

namespace ui::v2
{
UIRuntime::UIRuntime() = default;
UIRuntime::~UIRuntime() { Shutdown(); }

bool UIRuntime::Initialize(std::unique_ptr<IUIRenderer> renderer)
{
    if (initialized_ || !renderer)
        return false;
    renderer_ = std::move(renderer);
    inputRouter_ = std::make_unique<UIInputRouter>(windows_, focus_, pointerCapture_);
    initialized_ = true;
    return true;
}

void UIRuntime::Shutdown()
{
    if (!initialized_)
        return;
    pointerCapture_.Release();
    focus_.ClearFocus();
    windows_.CloseAll();
    inputRouter_.reset();
    renderer_.reset();
    initialized_ = false;
}

void UIRuntime::Update(float deltaSeconds)
{
    if (!initialized_ || !enabled_)
        return;
    windows_.Update(deltaSeconds);
}

void UIRuntime::Render()
{
    if (!initialized_ || !enabled_ || !renderer_ || !windows_.HasOpenWindows())
        return;
    renderer_->Begin();
    UIRenderContext context(*renderer_);
    windows_.Render(context);
    renderer_->End();
}

bool UIRuntime::HandleEvent(const UIEvent& event)
{
    if (!initialized_ || !enabled_ || !inputRouter_)
        return false;
    return inputRouter_->Route(event) != UIEventResult::Ignored;
}

void UIRuntime::OnResolutionChanged(int width, int height) noexcept
{
    viewportWidth_ = width;
    viewportHeight_ = height;
}

void UIRuntime::OnDeviceLost() {}
void UIRuntime::OnDeviceReset() {}
}
