#include "systems/ui/v2/input/UIInputRouter.h"
#include "systems/ui/v2/core/Widget.h"
#include "systems/ui/v2/input/UIFocusManager.h"
#include "systems/ui/v2/input/UIPointerCapture.h"
#include "systems/ui/v2/windows/UIWindowManager.h"

namespace ui::v2
{
UIEventResult UIInputRouter::Route(const UIEvent& event)
{
    if (Widget* captured = capture_.Captured())
    {
        const UIEventResult result = captured->OnEvent(event);
        if (event.type == UIEventType::PointerUp)
            capture_.Release();
        if (result != UIEventResult::Ignored)
            return result;
    }

    if (!event.IsPointerEvent())
    {
        if (Widget* focused = focus_.Focused())
        {
            const UIEventResult result = focused->OnEvent(event);
            if (result != UIEventResult::Ignored)
                return result;
        }
    }

    return windows_.Dispatch(event);
}
}
