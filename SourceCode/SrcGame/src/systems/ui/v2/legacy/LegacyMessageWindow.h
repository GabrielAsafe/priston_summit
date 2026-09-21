#pragma once

#include <cstdint>
#include <string>
#include "systems/ui/v2/core/Window.h"

namespace ui::v2
{
class ConfirmationOverlay;

namespace legacy
{
class LegacyMessageWindow final : public Window
{
public:
    explicit LegacyMessageWindow(UIId id = InvalidUIId);

    void Update(float deltaSeconds) override;
    [[nodiscard]] bool HasModalInput() const noexcept;
    bool HandleModalKey(std::uint32_t key);
    [[nodiscard]] bool HasSupportedMessage() const noexcept { return activeKind_ >= 0; }
    [[nodiscard]] static bool HasActiveSupportedLegacyMessage();

protected:
    void OnRender(UIRenderContext& context) override;

private:
    void Refresh();
    void Resolve(bool accepted);
    static int FindActiveKind();
    static bool SupportedKind(int kind);
    static std::string TitleFor(int kind);
    static std::string MessageFor(int kind);

    ConfirmationOverlay* confirmation_ = nullptr;
    int activeKind_ = -1;
};
}
}
