#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "systems/ui/v2/domain/ICovenantGateway.h"
#include "systems/ui/v2/pages/CovenantsPage.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"

using namespace ui::v2;

class FakeRenderer final : public IUIRenderer
{
public:
    void Begin() override {}
    void End() override {}
    void DrawImage(TextureHandle, const Rect&, const Rect*) override {}
    void DrawRect(const Rect&, Color) override { ++rects; }
    void DrawTextUtf8(FontHandle, std::string_view text, const Rect&, const TextStyle&) override { texts.emplace_back(text); }
    void PushClip(const Rect&) override {}
    void PopClip() override {}
    int rects = 0;
    std::vector<std::string> texts;
};

class FakeGateway final : public ICovenantGateway
{
public:
    FakeGateway()
    {
        state.valid = true;
        state.selectedCovenantId = 1;
        state.serverNow = 1000;
        state.entryCount = 5;
        const char* keys[5] = {"hunters","gatherers","artisans","fortune","ascendants"};
        for (int i = 0; i < 5; ++i)
        {
            state.entries[static_cast<std::size_t>(i)].covenantId = i + 1;
            state.entries[static_cast<std::size_t>(i)].key = keys[i];
            state.entries[static_cast<std::size_t>(i)].canJoin = true;
        }
        state.rankCount = 3;
        state.ranks[0] = {1,0,"tracker"};
        state.ranks[1] = {2,1000,"trophy_refining"};
        state.ranks[2] = {3,3000,"hunters_mark"};
        state.modifierCount = 2;
        state.modifiers[0] = {1,1000,"hunt.drop_chance","hunt_target"};
        state.modifiers[1] = {1,800,"combat.damage_taken","hunt_target"};
    }
    const CovenantUiSnapshot& Snapshot() const override { return state; }
    void SelectCovenant(int covenantId) override { state.selectedCovenantId = covenantId; ++selects; }
    void RequestRefresh() override { ++refreshes; }
    void Execute(CovenantUiAction action, int value = 0, const std::string& key = {}) override
    { lastAction = action; lastValue = value; lastKey = key; ++actions; }

    mutable CovenantUiSnapshot state;
    int selects = 0;
    int refreshes = 0;
    int actions = 0;
    int lastValue = 0;
    std::string lastKey;
    CovenantUiAction lastAction = CovenantUiAction::Refresh;
};

static void Click(CovenantsPage& page, int x, int y)
{
    UIEvent down; down.type = UIEventType::PointerDown; down.button = MouseButton::Left; down.pointer = {x,y};
    UIEvent up; up.type = UIEventType::PointerUp; up.button = MouseButton::Left; up.pointer = {x,y};
    assert(page.OnEvent(down) != UIEventResult::Ignored);
    assert(page.OnEvent(up) != UIEventResult::Ignored);
}

int main()
{
    FakeGateway gateway;
    CovenantsPage page(gateway, 1);
    page.SetBounds({0,0,774,478});
    page.Update(0.1f);

    FakeRenderer renderer;
    UIRenderContext context(renderer);
    page.Render(context);
    assert(renderer.rects > 5);
    assert(std::find(renderer.texts.begin(), renderer.texts.end(), "COVENANTS") != renderer.texts.end());
    assert(std::find(renderer.texts.begin(), renderer.texts.end(), "Hunters") != renderer.texts.end());

    // Select Gatherers from the left rail.
    Click(page, 40, 96);
    assert(gateway.selects == 1);
    assert(gateway.state.selectedCovenantId == 2);

    // Select Hunters again and use the primary action; unjoined => Join.
    Click(page, 40, 54);
    page.Update(0.01f);
    Click(page, 230, 442);
    assert(gateway.actions == 1);
    assert(gateway.lastAction == CovenantUiAction::Join);

    return 0;
}
