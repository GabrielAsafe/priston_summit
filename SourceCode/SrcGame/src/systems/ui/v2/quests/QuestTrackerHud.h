#pragma once

#include "systems/ui/v2/core/Window.h"
#include "systems/ui/v2/domain/IQuestGateway.h"

namespace ui::v2
{
class QuestTrackerHud final : public Window
{
public:
    explicit QuestTrackerHud(IQuestGateway& gateway, UIId id = InvalidUIId);

    void Update(float deltaSeconds) override;
    void SetViewport(int width, int height);

protected:
    void OnRender(UIRenderContext& context) override;

private:
    IQuestGateway& gateway_;
    QuestEntrySnapshot tracked_{};
    float refreshAccumulator_ = 0.0f;
};
}
