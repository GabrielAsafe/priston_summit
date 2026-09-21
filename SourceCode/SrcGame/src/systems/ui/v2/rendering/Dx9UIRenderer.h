#pragma once

#include <memory>
#include "systems/ui/v2/rendering/IUIRenderer.h"

namespace ui::v2
{
class Dx9UIRenderer final : public IUIRenderer
{
public:
    Dx9UIRenderer();
    ~Dx9UIRenderer() override;

    void Begin() override;
    void End() override;
    void DrawImage(TextureHandle texture, const Rect& destination, const Rect* source) override;
    void DrawRect(const Rect& rect, Color color) override;
    void DrawTextUtf8(FontHandle font, std::string_view utf8, const Rect& rect, const TextStyle& style) override;
    void PushClip(const Rect& rect) override;
    void PopClip() override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
