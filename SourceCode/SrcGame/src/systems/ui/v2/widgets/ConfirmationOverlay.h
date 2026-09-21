#pragma once

#include <functional>
#include <string>
#include "systems/ui/v2/core/Container.h"

namespace ui::v2
{
class Button;

class ConfirmationOverlay final : public Container
{
public:
    using Handler = std::function<void()>;

    explicit ConfirmationOverlay(UIId id = InvalidUIId);

    void Show(std::string title, std::string message, Handler onConfirm, Handler onCancel = {});
    void Hide();
    [[nodiscard]] bool IsOpen() const noexcept { return IsVisible(); }
    bool HandleKey(std::uint32_t key);
    void SetDimBackground(bool value) noexcept { dimBackground_ = value; }

    UIEventResult OnEvent(const UIEvent& event) override;

protected:
    void OnRender(UIRenderContext& context) override;

private:
    void Confirm();
    void Cancel();
    void LayoutButtons();

    std::string title_;
    std::string message_;
    Handler onConfirm_;
    Handler onCancel_;
    Button* confirmButton_ = nullptr;
    Button* cancelButton_ = nullptr;
    bool dimBackground_ = true;
};
}
