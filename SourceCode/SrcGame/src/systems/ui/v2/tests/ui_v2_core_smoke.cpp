#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "systems/ui/v2/core/UIRuntime.h"
#include "systems/ui/v2/core/Window.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/widgets/Label.h"

using namespace ui::v2;

class FakeRenderer final : public IUIRenderer
{
public:
    void Begin() override { ++beginCount; }
    void End() override { ++endCount; }
    void DrawImage(TextureHandle, const Rect&, const Rect*) override {}
    void DrawRect(const Rect&, Color) override { ++rectCount; }
    void DrawTextUtf8(FontHandle, std::string_view text, const Rect&, const TextStyle&) override
    {
        texts.emplace_back(text);
    }
    void PushClip(const Rect&) override {}
    void PopClip() override {}

    int beginCount = 0;
    int endCount = 0;
    int rectCount = 0;
    std::vector<std::string> texts;
};

int main()
{
    auto renderer = std::make_unique<FakeRenderer>();
    FakeRenderer* rawRenderer = renderer.get();

    UIRuntime runtime;
    assert(runtime.Initialize(std::move(renderer)));
    runtime.OnResolutionChanged(1280, 720);

    auto window = std::make_unique<Window>(100, "Character");
    window->SetBounds({100, 100, 300, 200});
    auto& label = window->AddChild<Label>(101, "Attributes");
    label.SetBounds({20, 50, 120, 20});

    constexpr WindowId characterWindow = 200;
    assert(runtime.Windows().Register(characterWindow, std::move(window)));
    assert(runtime.Windows().Open(characterWindow));

    runtime.Update(1.0f / 60.0f);
    runtime.Render();
    assert(rawRenderer->beginCount == 1);
    assert(rawRenderer->endCount == 1);
    assert(rawRenderer->rectCount >= 2);
    assert(rawRenderer->texts.size() >= 2);

    UIEvent down;
    down.type = UIEventType::PointerDown;
    down.button = MouseButton::Left;
    down.pointer = {120, 110};
    assert(runtime.HandleEvent(down));

    UIEvent move;
    move.type = UIEventType::PointerMove;
    move.pointer = {220, 210};
    assert(runtime.HandleEvent(move));

    UIEvent up;
    up.type = UIEventType::PointerUp;
    up.button = MouseButton::Left;
    up.pointer = {220, 210};
    assert(runtime.HandleEvent(up));

    const Window* moved = runtime.Windows().Get(characterWindow);
    assert(moved);
    assert(moved->Bounds().x == 200);
    assert(moved->Bounds().y == 200);

    runtime.Windows().Close(characterWindow);
    UIEvent outside;
    outside.type = UIEventType::PointerDown;
    outside.button = MouseButton::Left;
    outside.pointer = {210, 210};
    assert(!runtime.HandleEvent(outside));

    runtime.Shutdown();
    return 0;
}
