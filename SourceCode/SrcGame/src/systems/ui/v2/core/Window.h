#pragma once

#include <string>
#include "systems/ui/v2/core/Container.h"

namespace ui::v2
{
class Window : public Container
{
public:
    explicit Window(UIId id = InvalidUIId, std::string title = {});

    void SetTitle(std::string title) { title_ = std::move(title); }
    [[nodiscard]] const std::string& Title() const noexcept { return title_; }

    void SetMovable(bool value) noexcept { movable_ = value; }
    [[nodiscard]] bool IsMovable() const noexcept { return movable_; }

    void SetClosable(bool value) noexcept { closable_ = value; }
    [[nodiscard]] bool IsClosable() const noexcept { return closable_; }

    void SetTitleBarHeight(int value) noexcept { titleBarHeight_ = value; }
    [[nodiscard]] int TitleBarHeight() const noexcept { return titleBarHeight_; }

    UIEventResult OnEvent(const UIEvent& event) override;

protected:
    void OnRender(UIRenderContext& context) override;

private:
    std::string title_;
    bool movable_ = true;
    bool closable_ = true;
    bool dragging_ = false;
    Point dragOffset_{};
    int titleBarHeight_ = 28;
};
}
