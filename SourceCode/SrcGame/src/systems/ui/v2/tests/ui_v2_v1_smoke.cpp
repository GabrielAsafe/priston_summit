#include <cassert>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

#include "systems/ui/v2/core/UIRuntime.h"
#include "systems/ui/v2/domain/ICharacterStatsGateway.h"
#include "systems/ui/v2/domain/IContextGateway.h"
#include "systems/ui/v2/domain/IInventoryGateway.h"
#include "systems/ui/v2/domain/ICompanionGateway.h"
#include "systems/ui/v2/domain/ISkillGateway.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/windows/CharacterWindow.h"
#include "systems/ui/v2/hud/PlayerHud.h"
#include "systems/ui/v2/hud/CompanionPartyHud.h"

using namespace ui::v2;

class FakeRenderer final : public IUIRenderer
{
public:
    void Begin() override { ++begins; }
    void End() override { ++ends; }
    void DrawImage(TextureHandle, const Rect&, const Rect*) override { ++images; }
    void DrawRect(const Rect&, Color) override { ++rects; }
    void DrawTextUtf8(FontHandle, std::string_view text, const Rect&, const TextStyle&) override { texts.emplace_back(text); }
    void PushClip(const Rect&) override {}
    void PopClip() override {}
    int begins = 0, ends = 0, images = 0, rects = 0;
    std::vector<std::string> texts;
};

class FakeStats final : public ICharacterStatsGateway
{
public:
    CharacterStatsSnapshot GetSnapshot() const override
    {
        CharacterStatsSnapshot s;
        s.valid = true; s.name = "Tester"; s.jobName = "Knight"; s.level = 80;
        s.strength = 100; s.spirit = 30; s.talent = 70; s.dexterity = 60; s.health = 90;
        s.availablePoints = points; s.lifeCurrent = 500; s.lifeMax = 700; s.manaCurrent = 200; s.manaMax = 300;
        s.staminaCurrent = 350; s.staminaMax = 400; s.damageMin = 100; s.damageMax = 150;
        s.resistances = {10, 11, 12, 13, 14, 15, 16, 17};
        s.attributeImpacts[0].life = 0.8f; s.attributeImpacts[0].stamina = 0.5f;
        s.attributeImpacts[0].maxWeight = 2.0f; s.attributeImpacts[0].affectsDamage = true;
        return s;
    }
    bool AddAttribute(CharacterAttribute, int amount) override { if (points < amount) return false; points -= amount; return true; }
    bool UndoLastAttributePoint() override { ++points; return true; }
    int points = 20;
};

class FakeSkills final : public ISkillGateway
{
public:
    SkillBookSnapshot GetSnapshot() const override
    {
        SkillBookSnapshot b; b.valid = true; b.tier1Points = 3;
        SkillEntrySnapshot s; s.index = 1; s.name = "Sword Mastery"; s.description = "Passive test skill";
        s.rank = 4; s.learned = true; s.unlocked = true; s.visible = true; s.canUpgrade = true;
        b.skills.push_back(s); return b;
    }
    std::vector<DetailLineSnapshot> GetDetails(int) const override
    {
        std::vector<DetailLineSnapshot> lines;
        lines.push_back({"Sword Mastery LV.4", "", DetailLineTone::Header});
        lines.push_back({"A deliberately long description used to verify that the V2 detail panel wraps text instead of clipping it at the right edge.", "", DetailLineTone::Muted});
        for (int i = 0; i < 24; ++i)
            lines.push_back({"Bonus " + std::to_string(i + 1), std::to_string(i * 3) + "%", DetailLineTone::Accent});
        return lines;
    }
    bool RequestUpgrade(int) override { ++upgrades; return true; }
    bool BindLeft(int) override { ++leftBinds; return true; }
    bool BindRight(int) override { ++rightBinds; return true; }
    bool AssignShortcut(int, int shortcut, bool rightMouse) override
    {
        lastShortcut = shortcut; shortcutRight = rightMouse; ++shortcutAssignments; return true;
    }
    int upgrades = 0, leftBinds = 0, rightBinds = 0;
    int shortcutAssignments = 0, lastShortcut = 0;
    bool shortcutRight = false;
};

class FakeInventory final : public IInventoryGateway
{
public:
    InventorySnapshot GetSnapshot() const override
    {
        InventorySnapshot s; s.valid = true; s.gold = 12345; s.weightCurrent = 20; s.weightMax = 100;
        s.activeBagPage = bagPage; s.activeWeaponSet = weaponSet; s.locked = locked;
        InventoryItemSnapshot item; item.index = 2; item.name = "Test Sword"; item.gridX = 0; item.gridY = 0;
        item.widthPixels = 22; item.heightPixels = 44; s.items.push_back(item);
        InventoryItemSnapshot quest; quest.index = 3; quest.name = "Quest Relic"; quest.gridX = 0; quest.gridY = 0; quest.questItem = true;
        s.items.push_back(quest);
        InventoryItemSnapshot potion; potion.index = 4; potion.name = "Health Potion"; potion.equipmentSlot = 11; potion.equipped = true; potion.potionCount = 10;
        s.items.push_back(potion);
        s.drag.active = dragging; return s;
    }
    std::vector<DetailLineSnapshot> GetItemDetails(int itemIndex) const override
    {
        const std::string name = itemIndex == 3 ? "Quest Relic" : (itemIndex == 4 ? "Health Potion" : "Test Sword");
        std::vector<DetailLineSnapshot> lines;
        lines.push_back({name, "", DetailLineTone::Header});
        lines.push_back({"This item has a long legacy-style description that must wrap and remain scrollable instead of disappearing.", "", DetailLineTone::Muted});
        for (int i = 0; i < 20; ++i)
            lines.push_back({"Property " + std::to_string(i + 1), "+" + std::to_string(i + 2), DetailLineTone::Accent});
        return lines;
    }
    bool BeginDrag(int) override { dragging = true; ++beginDrags; return true; }
    bool DropToBag(int, int) override { dragging = false; ++bagDrops; return true; }
    bool DropToQuest(int, int) override { dragging = false; ++questDrops; return true; }
    bool DropToEquipment(int) override { dragging = false; ++equipDrops; return true; }
    bool CancelDrag() override { dragging = false; ++cancels; return true; }
    bool ActivateItem(int) override { ++activations; return true; }
    bool SelectBagPage(int page) override { bagPage = page; ++bagPageChanges; return true; }
    bool SelectWeaponSet(int set) override { weaponSet = set; ++weaponSetChanges; return true; }
    bool SetQuestItemsVisible(bool visible) override { questVisible = visible; ++questVisibilityChanges; return true; }
    bool OpenPersonalShopSales() override { ++personalShopOpens; return true; }
    bool ToggleLock() override { locked = !locked; ++locks; return true; }
    bool ToggleCostumeMode() override { ++costumes; return true; }
    mutable bool dragging = false;
    mutable int bagPage = 1;
    mutable int weaponSet = 1;
    mutable bool questVisible = false;
    mutable bool locked = false;
    int beginDrags = 0, bagDrops = 0, questDrops = 0, equipDrops = 0, cancels = 0, activations = 0;
    int bagPageChanges = 0, weaponSetChanges = 0, questVisibilityChanges = 0, personalShopOpens = 0;
    int locks = 0, costumes = 0;
};

class FakeCompanions final : public ICompanionGateway
{
public:
    FakeCompanions()
    {
        snapshot.valid = true;
        snapshot.mode = CompanionUiMode::Aggressive;
        snapshot.activeCount = 1;
        snapshot.maxActive = 4;
        snapshot.entryCount = 2;
        snapshot.entries[0].name = "Tester";
        snapshot.entries[0].currentCharacter = true;
        snapshot.entries[0].level = 80;
        snapshot.entries[1].name = "MageAlt";
        snapshot.entries[1].selected = true;
        snapshot.entries[1].active = true;
        snapshot.entries[1].level = 72;
        snapshot.entries[1].rightItemName = "Test Staff";
        snapshot.entries[1].rightItemCode = 0x01020304;
        runtime.valid = true;
        runtime.entryCount = 1;
        runtime.entries[0].name = "MageAlt";
        runtime.entries[0].level = 72;
        runtime.entries[0].life = 600;
        runtime.entries[0].maxLife = 800;
        runtime.entries[0].mana = 250;
        runtime.entries[0].maxMana = 400;
        runtime.entries[0].targetName = "Goblin";
        runtime.entries[0].mode = CompanionUiMode::Aggressive;
        runtime.entries[0].tactic = CompanionUiTactic::Balanced;
    }

    const CompanionUiSnapshot& Snapshot() const override { return snapshot; }
    const CompanionRuntimeSnapshot& RuntimeSnapshot() const override { return runtime; }
    void RequestRefresh() override { ++refreshes; }
    void Execute(CompanionUiAction action, const std::string& name = {}, int value = 0) override
    {
        lastAction = action;
        lastName = name;
        lastValue = value;
        ++actions;
    }

    CompanionUiSnapshot snapshot;
    CompanionRuntimeSnapshot runtime;
    int refreshes = 0;
    int actions = 0;
    CompanionUiAction lastAction = CompanionUiAction::Refresh;
    std::string lastName;
    int lastValue = 0;
};

class FakeContext final : public IContextGateway
{
public:
    ContextSnapshot GetSnapshot() const override { return snapshot; }
    bool ActivateItem(int index) override { lastActivated = index; ++activations; return true; }
    bool AcceptInventoryItem(int index) override { lastAcceptedInventory = index; ++acceptedInventory; return true; }
    bool InvokeAction(int actionId) override { lastAction = actionId; ++actions; return true; }
    bool CloseContext() override { snapshot = {}; ++closes; return true; }

    mutable ContextSnapshot snapshot;
    int lastActivated = -1;
    int lastAction = -1;
    int activations = 0;
    int lastAcceptedInventory = -1;
    int acceptedInventory = 0;
    int actions = 0;
    int closes = 0;
};

static void Click(UIRuntime& runtime, int x, int y, MouseButton button = MouseButton::Left)
{
    UIEvent down; down.type = UIEventType::PointerDown; down.button = button; down.pointer = {x, y};
    UIEvent up; up.type = UIEventType::PointerUp; up.button = button; up.pointer = {x, y};
    assert(runtime.HandleEvent(down));
    assert(runtime.HandleEvent(up));
}

int main()
{
    FakeStats stats; FakeSkills skills; FakeInventory inventory; FakeContext context; FakeCompanions companions;
    auto renderer = std::make_unique<FakeRenderer>();
    auto* raw = renderer.get();
    UIRuntime runtime;
    assert(runtime.Initialize(std::move(renderer)));

    auto hud = std::make_unique<PlayerHud>(stats, skills, inventory, 800);
    auto* rawHud = hud.get();
    rawHud->SetViewport(1024, 768);
    constexpr WindowId hudId = 8000;
    assert(runtime.Windows().Register(hudId, std::move(hud), UIWindowLayer::Hud));
    assert(runtime.Windows().Open(hudId));

    auto companionHud = std::make_unique<CompanionPartyHud>(companions, 801);
    auto* rawCompanionHud = companionHud.get();
    rawCompanionHud->SetViewport(1024, 768);
    constexpr WindowId companionHudId = 8001;
    assert(runtime.Windows().Register(companionHudId, std::move(companionHud), UIWindowLayer::Hud));
    assert(runtime.Windows().Open(companionHudId));

    auto character = std::make_unique<CharacterWindow>(stats, skills, inventory, context, companions, 1000);
    auto* rawCharacter = character.get();
    constexpr WindowId id = 9000;
    assert(runtime.Windows().Register(id, std::move(character)));
    assert(runtime.Windows().Open(id));
    runtime.Update(0.2f);
    runtime.Render();
    assert(raw->begins == 1 && raw->ends == 1 && raw->rects > 10 && !raw->texts.empty());
    const auto HasText = [raw](const std::string& expected)
    {
        return std::find(raw->texts.begin(), raw->texts.end(), expected) != raw->texts.end();
    };
    assert(HasText("Personagem"));
    assert(HasText("ATRIBUTOS / POR PONTO"));
    assert(HasText("RESISTENCIAS"));

    // Default tab is Attributes. Exercise a real +1 action through the
    // widget -> view-model -> gateway path before switching tabs.
    assert(rawCharacter->ActiveTab() == CharacterTab::Attributes);
    const int pageX = 24 + 8;
    const int pageY = 24 + 74;
    Click(runtime, pageX + 286 + 20, pageY + 80 + 15);
    assert(stats.points == 19);

    // Skills: select the first card, bind both mouse sides and request an
    // upgrade. The fake gateway lets us assert that no page mutates gameplay
    // state directly.
    Click(runtime, 24 + 136 + 20, 24 + 38 + 15);
    assert(rawCharacter->ActiveTab() == CharacterTab::Skills);
    raw->texts.clear();
    runtime.Render();
    assert(HasText("HABILIDADES"));
    assert(HasText("Sword Mastery"));
    Click(runtime, pageX + 14 + 30, pageY + 70 + 20, MouseButton::Left);
    Click(runtime, pageX + 14 + 30, pageY + 70 + 20, MouseButton::Right);

    // Macro parity: while a skill card is held, F1-F8 is translated into the
    // same legacy shortcut data through the gateway. A plain F-key press is
    // deliberately not handled by the page, leaving execution to cSKILL.
    UIEvent macroDown; macroDown.type = UIEventType::PointerDown; macroDown.button = MouseButton::Left;
    macroDown.pointer = {pageX + 14 + 30, pageY + 70 + 20};
    assert(runtime.HandleEvent(macroDown));
    assert(rawCharacter->HasArmedSkillShortcutTarget());
    assert(rawCharacter->HandleSkillShortcut(3));
    assert(skills.shortcutAssignments == 1 && skills.lastShortcut == 3 && !skills.shortcutRight);
    UIEvent macroUp; macroUp.type = UIEventType::PointerUp; macroUp.button = MouseButton::Left;
    macroUp.pointer = macroDown.pointer;
    assert(runtime.HandleEvent(macroUp));
    assert(!rawCharacter->HasArmedSkillShortcutTarget());

    macroDown.button = MouseButton::Right;
    assert(runtime.HandleEvent(macroDown));
    assert(rawCharacter->HasArmedSkillShortcutTarget());
    assert(rawCharacter->HandleSkillShortcut(4));
    assert(skills.shortcutAssignments == 2 && skills.lastShortcut == 4 && skills.shortcutRight);
    macroUp.button = MouseButton::Right;
    assert(runtime.HandleEvent(macroUp));
    assert(!rawCharacter->HasArmedSkillShortcutTarget());

    // The detail panel is deliberately longer than its viewport. Wheel input
    // must be consumed there and the only explicit action button left is upgrade.
    UIEvent skillWheel; skillWheel.type = UIEventType::Wheel; skillWheel.pointer = {pageX + 620, pageY + 220}; skillWheel.wheelDelta = -120;
    assert(runtime.HandleEvent(skillWheel));
    Click(runtime, pageX + 530, pageY + 440);
    assert(skills.leftBinds == 2);
    assert(skills.rightBinds == 2);
    assert(skills.upgrades == 1);

    // Inventory: pick the first bag item, then place it one cell to the right.
    Click(runtime, 24 + 258 + 20, 24 + 38 + 15);
    assert(rawCharacter->ActiveTab() == CharacterTab::Inventory);
    raw->texts.clear();
    runtime.Render();
    assert(HasText("INVENTARIO"));
    assert(HasText("MOCHILA"));
    assert(HasText("EQUIPAMENTO"));
    assert(HasText("Mochila A"));
    assert(HasText("Mochila B"));
    assert(HasText("Quest Items"));

    // Inventory parity controls: second bag, weapon set, quest items, personal shop and explicit lock.
    Click(runtime, pageX + 444 + 20, pageY + 52 + 15);
    assert(inventory.bagPage == 2 && inventory.bagPageChanges == 1);
    Click(runtime, pageX + 122 + 20, pageY + 52 + 15);
    assert(inventory.weaponSet == 2 && inventory.weaponSetChanges == 1);
    Click(runtime, pageX + 24 + 20, pageY + 52 + 15);
    assert(inventory.weaponSet == 1 && inventory.weaponSetChanges == 2);
    Click(runtime, pageX + 24 + 30, pageY + 418 + 14);
    assert(inventory.personalShopOpens == 1);
    Click(runtime, pageX + 142 + 30, pageY + 418 + 14);
    assert(inventory.locked && inventory.locks == 1);

    Click(runtime, pageX + 346 + 20, pageY + 52 + 15);
    assert(inventory.bagPage == 1);

    // Pointer hit-testing is performed in V2 coordinates. Moving over the
    // rendered item must expose its details before any legacy-coordinate
    // adapter is involved.
    UIEvent move;
    move.type = UIEventType::PointerMove;
    move.pointer = {pageX + 346 + 15, pageY + 92 + 15};
    assert(runtime.HandleEvent(move));
    raw->texts.clear();
    runtime.Render();
    assert(HasText("Test Sword"));

    Click(runtime, pageX + 346 + 15, pageY + 92 + 15);
    assert(inventory.beginDrags == 1 && inventory.dragging);

    // A/B parity: while carrying an item the opposite bag tab remains usable.
    // The legacy implementation supported this and it is how an item is moved
    // from one bag to the other without inventing a second transfer protocol.
    Click(runtime, pageX + 444 + 20, pageY + 52 + 15);
    assert(inventory.bagPage == 2 && inventory.dragging);
    Click(runtime, pageX + 346 + 30 + 15, pageY + 92 + 15);
    assert(inventory.bagDrops == 1 && !inventory.dragging);

    // Recipe contexts expose the empty stone sockets, not only occupied items.
    ContextSnapshot aging;
    aging.valid = true;
    aging.kind = ContextKind::Aging;
    aging.title = "Aging";
    aging.subtitle = "Recipe slots";
    aging.gridColumns = 8;
    aging.gridRows = 4;
    ContextSlotSnapshot primary; primary.gridX = 0; primary.gridY = 0; primary.gridWidth = 3; primary.gridHeight = 4; primary.label = "Item";
    ContextSlotSnapshot stone; stone.gridX = 5; stone.gridY = 0; stone.required = true; stone.label = "Pedra";
    ContextSlotSnapshot special; special.gridX = 3; special.gridY = 3; special.special = true; special.label = "Especial";
    aging.slots = {primary, stone, special};
    context.snapshot = aging;
    runtime.Update(0.2f);
    raw->texts.clear();
    const int rectsBeforeRecipe = raw->rects;
    runtime.Render();
    assert(raw->rects > rectsBeforeRecipe);
    assert(HasText("Aging"));
    assert(HasText("Item"));
    assert(HasText("E"));
    context.snapshot = {};
    runtime.Update(0.2f);

    Click(runtime, pageX + 542 + 20, pageY + 52 + 15);
    assert(inventory.questVisible);
    UIEvent questMove; questMove.type = UIEventType::PointerMove; questMove.pointer = {pageX + 346 + 20, pageY + 92 + 20};
    assert(runtime.HandleEvent(questMove));
    raw->texts.clear(); runtime.Render();
    assert(HasText("Quest Relic"));
    Click(runtime, pageX + 346 + 20, pageY + 92 + 20);
    assert(inventory.dragging);
    Click(runtime, pageX + 346 + 52 + 20, pageY + 92 + 20);
    assert(inventory.questDrops == 1 && !inventory.dragging);
    Click(runtime, pageX + 346 + 20, pageY + 52 + 15);
    assert(!inventory.questVisible);

    // Context UI: an active Aging/merchant-style panel replaces only the
    // left equipment column while keeping the bag usable on the right.
    context.snapshot.valid = true;
    context.snapshot.kind = ContextKind::Aging;
    context.snapshot.title = "Aging";
    context.snapshot.subtitle = "Place an item and materials here.";
    ContextItemSnapshot contextItem;
    contextItem.index = 7; contextItem.name = "Aging Stone";
    context.snapshot.items.push_back(contextItem);
    context.snapshot.actions.push_back(ContextActionSnapshot{40, "Aplicar Aging", true, false});
    runtime.Update(0.2f);
    raw->texts.clear();
    runtime.Render();
    assert(HasText("Aging"));
    assert(HasText("Aplicar Aging"));
    // Context panel starts at page-local (14,48), item area local (10,80).
    Click(runtime, pageX + 14 + 10 + 15, pageY + 48 + 80 + 15);
    assert(context.activations == 1 && context.lastActivated == 7);
    // First action is laid out near the bottom of the context panel.
    Click(runtime, pageX + 14 + 20, pageY + 48 + 314 + 14);
    assert(context.actions == 1 && context.lastAction == 40);
    Click(runtime, pageX + 346 + 15, pageY + 92 + 15, MouseButton::Right);
    assert(context.acceptedInventory == 1 && context.lastAcceptedInventory == 2);
    context.snapshot = {};
    runtime.Update(0.2f);

    // Large contexts (warehouse has 100 legacy slots) must page instead of
    // silently truncating everything after the first visual grid.
    context.snapshot.valid = true;
    context.snapshot.kind = ContextKind::Warehouse;
    context.snapshot.title = "Armazem";
    for (int i = 0; i < 60; ++i)
    {
        ContextItemSnapshot entry;
        entry.index = i;
        entry.name = "Warehouse " + std::to_string(i);
        context.snapshot.items.push_back(entry);
    }
    runtime.Update(0.2f);
    raw->texts.clear();
    runtime.Render();
    assert(HasText("1/2"));
    // Next-page button: panel local x = width-44, y=7.
    Click(runtime, pageX + 14 + 314 - 44 + 16, pageY + 48 + 7 + 12);
    UIEvent warehouseMove;
    warehouseMove.type = UIEventType::PointerMove;
    warehouseMove.pointer = {pageX + 14 + 10 + 14, pageY + 48 + 80 + 14};
    assert(runtime.HandleEvent(warehouseMove));
    raw->texts.clear();
    runtime.Render();
    assert(HasText("2/2"));
    assert(HasText("Warehouse 48"));

    // Coin shops use the same panel but their own server-backed page/section
    // state and currency. Six actions fit without reverting to the legacy UI.
    context.snapshot = {};
    context.snapshot.valid = true;
    context.snapshot.kind = ContextKind::CoinShop;
    context.snapshot.title = "Coin Shop";
    context.snapshot.subtitle = "Armas - pagina 1/1";
    context.snapshot.money = 250;
    context.snapshot.currency = "SCoin";
    ContextItemSnapshot coinItem;
    coinItem.index = 0; coinItem.name = "Premium Sword"; coinItem.price = 100; coinItem.selected = true;
    context.snapshot.items.push_back(coinItem);
    for (int i = 70; i <= 75; ++i)
        context.snapshot.actions.push_back(ContextActionSnapshot{i, i == 74 ? "Comprar" : "Acao", true, false});
    runtime.Update(0.2f);
    raw->texts.clear();
    runtime.Render();
    assert(HasText("Coin Shop"));
    assert(HasText("SCoin: 250"));
    assert(HasText("Comprar"));

    context.snapshot = {};
    runtime.Update(0.2f);

    // Move the complete shell and prove item hitboxes follow the rendered
    // window instead of remaining attached to the original/legacy position.
    UIEvent titleDown; titleDown.type = UIEventType::PointerDown; titleDown.button = MouseButton::Left; titleDown.pointer = {50, 40};
    UIEvent titleMove; titleMove.type = UIEventType::PointerMove; titleMove.pointer = {150, 100};
    UIEvent titleUp; titleUp.type = UIEventType::PointerUp; titleUp.button = MouseButton::Left; titleUp.pointer = {150, 100};
    assert(runtime.HandleEvent(titleDown));
    assert(runtime.HandleEvent(titleMove));
    assert(runtime.HandleEvent(titleUp));
    assert(rawCharacter->Bounds().x != 24 || rawCharacter->Bounds().y != 24);

    const int movedPageX = rawCharacter->Bounds().x + 8;
    const int movedPageY = rawCharacter->Bounds().y + 74;
    const int dragsBeforeOldPositionClick = inventory.beginDrags;
    Click(runtime, pageX + 346 + 15, pageY + 92 + 15);
    assert(inventory.beginDrags == dragsBeforeOldPositionClick && !inventory.dragging);

    Click(runtime, movedPageX + 346 + 15, movedPageY + 88 + 15);
    assert(inventory.beginDrags == dragsBeforeOldPositionClick + 1 && inventory.dragging);

    // Closing the shell while an item is in flight must never leave the
    // legacy MouseItem equivalent stuck. The shell owns that safety action.
    Click(runtime, rawCharacter->Bounds().x + 790 - 42 + 15, rawCharacter->Bounds().y + 5 + 12);
    assert(inventory.cancels == 1 && !inventory.dragging);

    // The permanent V2 HUD replaces the old bottom chrome. It exposes vitals,
    // current skill slots and P1/P2/P3; only consumables are interactive.
    runtime.Windows().Close(id);
    raw->texts.clear();
    runtime.Render();
    assert(HasText("HP  500/700"));
    assert(HasText("MP  200/300"));
    assert(HasText("RES  350/400"));
    const Rect hudBounds = rawHud->GlobalBounds();
    Click(runtime, hudBounds.x + 382 + 20, hudBounds.y + 58 + 8 + 20);
    assert(inventory.activations >= 1);

    // Companions: the fourth tab is a pure V2 surface backed by the structured
    // gateway. Rendering it must not depend on legacy chat commands.
    runtime.Windows().Open(id);
    rawCharacter->SetActiveTab(CharacterTab::Companions, false);
    assert(rawCharacter->ActiveTab() == CharacterTab::Companions);
    assert(companions.refreshes > 0);
    raw->texts.clear();
    runtime.Render();
    assert(HasText("COMPANIONS DA CONTA"));
    assert(HasText("MageAlt - Lv 72"));
    assert(HasText("MageAlt  Lv 72  [AGR/BAL]"));

    // Combat HUD quick controls: left cycles the selected companion mode;
    // middle requests owner-focus tactics; right dismisses.
    const Rect companionHudBounds = rawCompanionHud->GlobalBounds();
    Click(runtime, companionHudBounds.x + 20, companionHudBounds.y + 20, MouseButton::Left);
    assert(companions.lastAction == CompanionUiAction::SetMode);
    assert(companions.lastName == "MageAlt" && companions.lastValue == 1);
    Click(runtime, companionHudBounds.x + 20, companionHudBounds.y + 20, MouseButton::Middle);
    assert(companions.lastAction == CompanionUiAction::SetTactic);
    assert(companions.lastValue == static_cast<int>(CompanionUiTactic::OwnerFocus));
    Click(runtime, companionHudBounds.x + 20, companionHudBounds.y + 20, MouseButton::Right);
    assert(companions.lastAction == CompanionUiAction::Dismiss);

    runtime.Shutdown();
    return 0;
}
