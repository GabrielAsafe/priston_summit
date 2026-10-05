#pragma once

#include "systems/ui/v2/core/Window.h"

namespace ui::v2
{
class Button;

namespace legacy
{
class LegacyNpcDialogWindow final : public Window
{
public:
    explicit LegacyNpcDialogWindow(UIId id = InvalidUIId);

    void Update(float deltaSeconds) override;
    [[nodiscard]] bool HasSupportedDialog() const noexcept { return activeIndex_ >= 0; }
    [[nodiscard]] static bool HasActiveSupportedLegacyDialog();
    [[nodiscard]] static bool HasActiveEventGiftDialog();

protected:
    UIEventResult OnEvent(const UIEvent& event) override;
    void OnRender(UIRenderContext& context) override;

private:
    void Refresh();
    void ActivatePrimary();
    void ActivateGuideHelp();
    void CloseDialog();
    static int FindActiveIndex();
    static bool SupportedKind(int kind);
    static const char* TitleFor(int kind);

    Button* primaryButton_ = nullptr;
    Button* guideButton_ = nullptr;
    Button* closeButton_ = nullptr;
    int activeIndex_ = -1;
    int scrollLine_ = 0;
};
}
}
