#pragma once

#include "systems/ui/v2/core/Window.h"

namespace ui::v2::legacy
{
class LegacyNotificationHud final : public Window
{
public:
    explicit LegacyNotificationHud(UIId id = InvalidUIId);

    void SetViewport(int width, int height);
    [[nodiscard]] static bool HasActiveLegacyNotification();

protected:
    void OnRender(UIRenderContext& context) override;

private:
    static int FindActiveMessage();
    static int FindActiveMessage1();
};
}
