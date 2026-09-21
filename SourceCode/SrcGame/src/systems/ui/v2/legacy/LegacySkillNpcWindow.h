#pragma once

#include "systems/ui/v2/core/Window.h"

namespace ui::v2
{
class Button;
class Label;

namespace legacy
{
class LegacySkillNpcWindow final : public Window
{
public:
    explicit LegacySkillNpcWindow(UIId id = InvalidUIId);

    void Update(float deltaSeconds) override;

private:
    void ChooseSkills();
    void ChooseChangeJob();
    void ChooseExit();

    Label* description_ = nullptr;
    Button* skillButton_ = nullptr;
    Button* changeJobButton_ = nullptr;
    Button* exitButton_ = nullptr;
};
}
}
