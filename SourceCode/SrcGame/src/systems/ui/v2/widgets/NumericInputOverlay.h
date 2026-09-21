#pragma once

#include <functional>
#include <string>
#include "systems/ui/v2/core/Container.h"

namespace ui::v2
{
class Button;

class NumericInputOverlay final : public Container
{
public:
    using ConfirmHandler = std::function<void(int)>;
    using OptionConfirmHandler = std::function<void(int, bool)>;
    using CancelHandler = std::function<void()>;

    explicit NumericInputOverlay(UIId id = InvalidUIId);

    void Show(std::string title, std::string message, int minimum, int maximum, int initial,
              ConfirmHandler onConfirm, CancelHandler onCancel = {});
    void ShowWithOption(std::string title, std::string message, int minimum, int maximum, int initial,
                        std::string primaryOption, std::string alternateOption,
                        OptionConfirmHandler onConfirm, CancelHandler onCancel = {});
    void Hide();
    [[nodiscard]] bool IsOpen() const noexcept { return IsVisible(); }
    [[nodiscard]] int Value() const noexcept;
    [[nodiscard]] bool AlternateSelected() const noexcept { return alternateSelected_; }
    bool HandleKey(std::uint32_t key);

    UIEventResult OnEvent(const UIEvent& event) override;

protected:
    void OnRender(UIRenderContext& context) override;

private:
    void Confirm();
    void Cancel();
    void LayoutButtons();
    void Normalize();
    void RefreshOptionButtons();

    std::string title_;
    std::string message_;
    std::string valueText_;
    std::string primaryOption_;
    std::string alternateOption_;
    int minimum_ = 0;
    int maximum_ = 0;
    bool alternateSelected_ = false;
    OptionConfirmHandler onConfirm_;
    CancelHandler onCancel_;
    Button* primaryOptionButton_ = nullptr;
    Button* alternateOptionButton_ = nullptr;
    Button* confirmButton_ = nullptr;
    Button* cancelButton_ = nullptr;
};
}
