#pragma once

#include <string_view>
#include "systems/ui/v2/core/Geometry.h"
#include "systems/ui/v2/rendering/FontHandle.h"
#include "systems/ui/v2/rendering/TextureHandle.h"

namespace ui::v2
{
enum class TextAlign
{
    Left,
    Center,
    Right
};

struct TextStyle
{
    Color color{};
    bool bold = false;
    TextAlign align = TextAlign::Left;
};

class IUIRenderer
{
public:
    virtual ~IUIRenderer() = default;

    virtual void Begin() = 0;
    virtual void End() = 0;
    virtual void DrawImage(TextureHandle texture, const Rect& destination, const Rect* source = nullptr) = 0;
    virtual void DrawRect(const Rect& rect, Color color) = 0;
    virtual void DrawTextUtf8(FontHandle font, std::string_view utf8, const Rect& rect, const TextStyle& style) = 0;
    virtual void PushClip(const Rect& rect) = 0;
    virtual void PopClip() = 0;
};

class UIRenderContext
{
public:
    explicit UIRenderContext(IUIRenderer& renderer) : renderer_(renderer) {}

    [[nodiscard]] IUIRenderer& Renderer() noexcept { return renderer_; }

private:
    IUIRenderer& renderer_;
};
}
