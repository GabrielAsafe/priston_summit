#pragma once

#include "systems/ui/v2/core/UIEvent.h"

namespace ui::v2
{
class UIFocusManager;
class UIPointerCapture;
class UIWindowManager;

class UIInputRouter
{
public:
    UIInputRouter(UIWindowManager& windows, UIFocusManager& focus, UIPointerCapture& capture)
        : windows_(windows), focus_(focus), capture_(capture) {}

    UIEventResult Route(const UIEvent& event);

private:
    UIWindowManager& windows_;
    UIFocusManager& focus_;
    UIPointerCapture& capture_;
};
}
