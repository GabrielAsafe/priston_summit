#include "systems/ui/v2/legacy/LegacyUIV2Bridge.h"

#include <cstdio>
#include <cstring>
#include <memory>
#include <windows.h>

#include "compat/legacy/sin/sinLinkHeader.h"
#include "core/client/legacy_sin_runtime/sinSubMain.h"
#include "systems/ui/v2/UIV2Config.h"
#include "systems/commerce/coin_shop/CoinShop.h"
#include "systems/commerce/coin_shop/CoinShop_T.h"
#include "systems/commerce/restoration/cRestaureWindow.h"
#include "systems/ui/settings/legacy_window/cSettingsWindow.h"
#include "systems/gameplay/quests/ui/cQuestWindow.h"
#include "systems/ui/v2/core/UIRuntime.h"
#include "systems/ui/v2/core/Window.h"
#include "systems/ui/v2/legacy/LegacyCharacterStatsGateway.h"
#include "systems/ui/v2/legacy/LegacyInventoryGateway.h"
#include "systems/ui/v2/legacy/LegacyCompanionGateway.h"
#include "systems/ui/v2/legacy/LegacyCovenantGateway.h"
#include "systems/ui/v2/legacy/LegacyContextGateway.h"
#include "systems/ui/v2/legacy/LegacyQuestGateway.h"
#include "systems/ui/v2/legacy/LegacySkillGateway.h"
#include "systems/ui/v2/legacy/LegacyMessageWindow.h"
#include "systems/ui/v2/legacy/LegacyNotificationHud.h"
#include "systems/ui/v2/legacy/LegacyNpcDialogWindow.h"
#include "systems/ui/v2/legacy/LegacySkillNpcWindow.h"
#include "systems/ui/v2/hud/PlayerHud.h"
#include "systems/ui/v2/hud/CompanionPartyHud.h"
#include "systems/ui/v2/hud/CovenantEffectHud.h"
#include "systems/ui/v2/quests/QuestJournalWindow.h"
#include "systems/ui/v2/quests/QuestTrackerHud.h"
#include "systems/ui/v2/rendering/Dx9UIRenderer.h"
#include "systems/ui/v2/widgets/Label.h"
#include "systems/ui/v2/widgets/Panel.h"
#include "systems/ui/v2/windows/CharacterWindow.h"
#include "systems/ui/v2/windows/SettingsWindow.h"

extern int SetChatingLine(char* szMessage);

namespace ui::v2::legacy
{
namespace
{
constexpr WindowId BootstrapWindowId = 0x55495632424F4F54ULL; // "UIV2BOOT"
constexpr WindowId CharacterWindowId = 0x5549563243484152ULL; // "UIV2CHAR"
constexpr WindowId PlayerHudWindowId = 0x5549563248554431ULL; // "UIV2HUD1"
constexpr WindowId CompanionPartyHudWindowId = 0x5549563243485544ULL; // "UIV2CHUD"
constexpr WindowId QuestJournalWindowId = 0x5549563251554553ULL; // "UIV2QUES"
constexpr WindowId QuestTrackerWindowId = 0x554956325452414BULL; // "UIV2TRAK"
constexpr WindowId LegacyMessageWindowId = 0x554956324D534733ULL; // "UIV2MSG3"
constexpr WindowId LegacyNotificationWindowId = 0x554956324E4F5446ULL; // "UIV2NOTF"
constexpr WindowId NpcDialogWindowId = 0x554956324E504344ULL; // "UIV2NPCD"
constexpr WindowId SkillNpcWindowId = 0x55495632534B4E50ULL; // "UIV2SKNP"
constexpr WindowId SettingsWindowId = 0x5549563253455454ULL; // "UIV2SETT"
UIRuntime runtime;
DWORD lastTick = 0;
CharacterWindow* characterWindow = nullptr;
PlayerHud* playerHud = nullptr;
CompanionPartyHud* companionPartyHud = nullptr;
QuestJournalWindow* questJournal = nullptr;
QuestTrackerHud* questTracker = nullptr;
LegacyMessageWindow* legacyMessageWindow = nullptr;
LegacyNotificationHud* legacyNotificationHud = nullptr;
LegacyNpcDialogWindow* npcDialogWindow = nullptr;
LegacySkillNpcWindow* skillNpcWindow = nullptr;
SettingsWindow* settingsWindow = nullptr;
LegacyCharacterStatsGateway statsGateway;
LegacySkillGateway skillGateway;
LegacyInventoryGateway inventoryGateway;
LegacyContextGateway contextGateway;
LegacyQuestGateway questGateway;
LegacyCompanionGateway& companionGateway = GetLegacyCompanionGateway();
LegacyCovenantGateway& covenantGateway = GetLegacyCovenantGateway();
bool companionTabRequested = false;
bool covenantTabRequested = false;
CovenantEffectHud covenantEffectHud;

bool RequiresLegacyInventoryUI()
{
    // These flows still read absolute legacy inventory coordinates directly.
    // The NPC blacksmith/shop is deliberately excluded: its repair/sell mode
    // is expressed through CursorClass and the V2 gateway can safely feed the
    // selected logical item into the original repair/sell routines.
    // v1.2 migrates the common contextual commerce/crafting surfaces into
    // the Inventory V2 left panel. Keep fallback only for flows not yet
    // represented by the context gateway.
    // The personal-shop description editor still uses the legacy IME/text
    // widget. Everything else in the personal-shop editor is V2.
    return MyShopExpBox;
}

bool MigratedContextActive()
{
    return cShop.OpenFlag || cCharShop.OpenFlag || cMyShop.OpenFlag || cRestaure.OpenFlag ||
           cTrade.OpenFlag || cWareHouse.OpenFlag || cCraftItem.OpenFlag ||
           cAging.OpenFlag || SmeltingItem.OpenFlag || ManufactureItem.m_OpenFlag ||
           cMixtureReset.OpenFlag || cSkinChange.OpenFlag ||
           cCoinShop.isOpen() || cCoinShop_T.isOpen();
}

bool LegacyTradeContextActiveOrMoving()
{
    return cTrade.OpenFlag || cWareHouse.OpenFlag || cCraftItem.OpenFlag || cAging.OpenFlag ||
           SmeltingItem.OpenFlag || ManufactureItem.m_OpenFlag || cMixtureReset.OpenFlag || cSkinChange.OpenFlag ||
           sinMoveKindInter[SIN_TRADE] > 0 ||
           sinMoveKindInter[SIN_WAREHOUSE] > 0 ||
           sinMoveKindInter[SIN_CRAFTITEM] > 0 ||
           sinMoveKindInter[SIN_AGING] > 0 ||
           sinMoveKindInter[SIN_SMELTING] > 0 ||
           sinMoveKindInter[SIN_MANUFACTURE] > 0 ||
           sinMoveKindInter[SIN_MIXTURE_RESET] > 0 ||
           sinMoveKindInter[SIN_SKIN_CHANGE] > 0;
}

bool AnyInventoryContextActive()
{
    return MigratedContextActive() || LegacyTradeContextActiveOrMoving() || RequiresLegacyInventoryUI();
}

void BuildBootstrapWindow()
{
    auto window = std::make_unique<Window>(1, "UI V2 - Bootstrap");
    window->SetBounds({32, 32, 430, 190});
    window->SetMovable(true);

    auto& body = window->AddChild<Panel>(2, Color{24, 29, 38, 235});
    body.SetBounds({8, 36, 414, 146});
    body.SetHitTestVisible(false);

    auto& title = window->AddChild<Label>(3, "UI V2 runtime ativo");
    title.SetBounds({20, 48, 390, 24});
    title.SetStyle(TextStyle{Color{235, 238, 244, 255}, true, TextAlign::Left});

    auto& line1 = window->AddChild<Label>(4, "Core desacoplado + ownership RAII");
    line1.SetBounds({20, 80, 390, 22});
    line1.SetStyle(TextStyle{Color{190, 201, 218, 255}, false, TextAlign::Left});

    auto& line2 = window->AddChild<Label>(5, "WindowManager, input e renderer DX9");
    line2.SetBounds({20, 106, 390, 22});
    line2.SetStyle(TextStyle{Color{190, 201, 218, 255}, false, TextAlign::Left});

    auto& line3 = window->AddChild<Label>(6, "Character/Commerce UI v1.2 integrada");
    line3.SetBounds({20, 132, 390, 22});
    line3.SetStyle(TextStyle{Color{190, 201, 218, 255}, false, TextAlign::Left});

    runtime.Windows().Register(BootstrapWindowId, std::move(window), UIWindowLayer::Debug);
#if UI_V2_SHOW_BOOTSTRAP_WINDOW
    runtime.Windows().Open(BootstrapWindowId);
#endif
}

void ClearLegacyCharacterFlags()
{
    cCharStatus.OpenFlag = SIN_CLOSE;
    cSkill.OpenFlag = SIN_CLOSE;
    if (!AnyInventoryContextActive())
        cInvenTory.OpenFlag = SIN_CLOSE;
}

void ApplyLegacyTab(CharacterTab tab)
{
    if (tab == CharacterTab::Companions || tab == CharacterTab::Covenants)
    {
        // Companions is a pure V2 surface. Do not alias it to the legacy
        // inventory button: doing so makes SyncCharacterWindow() immediately
        // switch the window back to Inventory on the next frame.
        ClearLegacyCharacterFlags();
        return;
    }

    const int button = tab == CharacterTab::Attributes ? SIN_CHARSTATUS :
                       tab == CharacterTab::Skills ? SIN_SKILL : SIN_INVENTORY;
    cInterFace.CheckAllBox(button);

    cCharStatus.OpenFlag = tab == CharacterTab::Attributes ? SIN_OPEN : SIN_CLOSE;
    cSkill.OpenFlag = tab == CharacterTab::Skills ? SIN_OPEN : SIN_CLOSE;
    cInvenTory.OpenFlag = tab == CharacterTab::Inventory ? SIN_OPEN : SIN_CLOSE;
}

void BuildCharacterWindow()
{
#if UI_V2_CHARACTER_WINDOW
    auto window = std::make_unique<CharacterWindow>(statsGateway, skillGateway, inventoryGateway, contextGateway, companionGateway, covenantGateway, 100);
    characterWindow = window.get();
    characterWindow->SetViewport(WinSizeX, WinSizeY);
    characterWindow->SetOnTabChanged([](CharacterTab tab)
    {
        if (MigratedContextActive() && tab != CharacterTab::Inventory)
        {
            companionTabRequested = false;
            covenantTabRequested = false;
            cInvenTory.OpenFlag = SIN_OPEN;
            if (characterWindow)
                characterWindow->SetActiveTab(CharacterTab::Inventory, false);
            return;
        }

        companionTabRequested = tab == CharacterTab::Companions;
        covenantTabRequested = tab == CharacterTab::Covenants;
        ApplyLegacyTab(tab);
    });
    characterWindow->SetOnClose([]()
    {
        if (characterWindow)
            characterWindow->PrepareClose();
        if (MigratedContextActive())
            contextGateway.CloseContext();
        companionTabRequested = false;
        covenantTabRequested = false;
        ClearLegacyCharacterFlags();
        ResetLegacyCharacterInterfaceMotion();
        runtime.Windows().Close(CharacterWindowId);
    });
    runtime.Windows().Register(CharacterWindowId, std::move(window), UIWindowLayer::NormalWindow);
#endif
}

void ToggleCharacterTab(CharacterTab tab)
{
    if (!characterWindow || RequiresLegacyInventoryUI()) return;
    if (MigratedContextActive() && tab != CharacterTab::Inventory)
        tab = CharacterTab::Inventory;

    companionTabRequested = tab == CharacterTab::Companions;
    covenantTabRequested = tab == CharacterTab::Covenants;
    const bool open = runtime.Windows().IsOpen(CharacterWindowId);
    if (open && characterWindow->ActiveTab() == tab && !MigratedContextActive())
    {
        characterWindow->PrepareClose();
        companionTabRequested = false;
        covenantTabRequested = false;
        ClearLegacyCharacterFlags();
        runtime.Windows().Close(CharacterWindowId);
        return;
    }
    ApplyLegacyTab(tab);
    characterWindow->SetActiveTab(tab, false);
    runtime.Windows().Open(CharacterWindowId);
}

void ToggleQuestJournal()
{
#if UI_V2_QUEST_JOURNAL
    if (!questJournal) return;
    if (runtime.Windows().IsOpen(QuestJournalWindowId))
    {
        questGateway.HideBackend();
        runtime.Windows().Close(QuestJournalWindowId);
    }
    else if (questGateway.EnsureBackendOpen())
        runtime.Windows().Open(QuestJournalWindowId);
#endif
}

void ExecuteHudShortcut(HudShortcutAction action)
{
    switch (action)
    {
        case HudShortcutAction::Attributes: ToggleCharacterTab(CharacterTab::Attributes); break;
        case HudShortcutAction::Skills: ToggleCharacterTab(CharacterTab::Skills); break;
        case HudShortcutAction::Inventory: ToggleCharacterTab(CharacterTab::Inventory); break;
        case HudShortcutAction::Party: ShowParty(); break;
        case HudShortcutAction::Quests: ToggleQuestJournal(); break;
        case HudShortcutAction::System:
            ToggleSettingsWindow();
            break;
        case HudShortcutAction::RunWalk:
            cInterFace.sInterFlags.RunFlag = cInterFace.sInterFlags.RunFlag ? 0 : 1;
            break;
        case HudShortcutAction::Camera:
            ++cInterFace.sInterFlags.CameraAutoFlag;
            if (cInterFace.sInterFlags.CameraAutoFlag > 2) cInterFace.sInterFlags.CameraAutoFlag = 0;
            break;
        case HudShortcutAction::Map:
            cInterFace.sInterFlags.MapOnFlag = cInterFace.sInterFlags.MapOnFlag ? 0 : 1;
            break;
        case HudShortcutAction::Chat:
        {
            if (hFocusWnd == hTextWnd || sinChatEnter)
            {
                hFocusWnd = 0;
                cInterFace.ChatFlag = 0;
                sinChatEnter = 0;
                SetWindowText(hTextWnd, "");
                SetIME_Mode(0);
            }
            else
            {
                char empty[] = "";
                SetChatingLine(empty);
            }
            break;
        }
    }
}

void BuildPlayerHud()
{
#if UI_V2_PLAYER_HUD
    auto hud = std::make_unique<PlayerHud>(statsGateway, skillGateway, inventoryGateway, 140);
    playerHud = hud.get();
    playerHud->SetOnShortcut([](HudShortcutAction action) { ExecuteHudShortcut(action); });
    playerHud->SetViewport(WinSizeX, WinSizeY);
    runtime.Windows().Register(PlayerHudWindowId, std::move(hud), UIWindowLayer::Hud);
    runtime.Windows().Open(PlayerHudWindowId);
#endif
}

void BuildCompanionPartyHud()
{
#if UI_V2_COMPANION_PARTY_HUD
    auto hud = std::make_unique<CompanionPartyHud>(companionGateway, 145);
    companionPartyHud = hud.get();
    companionPartyHud->SetViewport(WinSizeX, WinSizeY);
    runtime.Windows().Register(CompanionPartyHudWindowId, std::move(hud), UIWindowLayer::Hud);
    runtime.Windows().Open(CompanionPartyHudWindowId);
#endif
}

void BuildQuestUI()
{
#if UI_V2_QUEST_JOURNAL
    auto journal = std::make_unique<QuestJournalWindow>(questGateway, 160);
    questJournal = journal.get();
    questJournal->SetViewport(WinSizeX, WinSizeY);
    questJournal->SetOnClose([]()
    {
        questGateway.HideBackend();
        runtime.Windows().Close(QuestJournalWindowId);
    });
    runtime.Windows().Register(QuestJournalWindowId, std::move(journal), UIWindowLayer::NormalWindow);

    auto tracker = std::make_unique<QuestTrackerHud>(questGateway, 161);
    questTracker = tracker.get();
    questTracker->SetViewport(WinSizeX, WinSizeY);
    runtime.Windows().Register(QuestTrackerWindowId, std::move(tracker), UIWindowLayer::Hud);
    runtime.Windows().Open(QuestTrackerWindowId);
#endif
}


void BuildSettingsWindow()
{
    auto window = std::make_unique<SettingsWindow>(210);
    settingsWindow = window.get();
    settingsWindow->SetViewport(WinSizeX, WinSizeY);
    settingsWindow->SetOnClose([]()
    {
        runtime.Windows().Close(SettingsWindowId);
    });
    runtime.Windows().Register(SettingsWindowId, std::move(window), UIWindowLayer::NormalWindow);
}

void BuildLegacyDialogUI()
{
    auto message = std::make_unique<LegacyMessageWindow>(180);
    legacyMessageWindow = message.get();
    legacyMessageWindow->SetBounds({0, 0, WinSizeX, WinSizeY});
    runtime.Windows().Register(LegacyMessageWindowId, std::move(message), UIWindowLayer::Modal);

    auto notifications = std::make_unique<LegacyNotificationHud>(185);
    legacyNotificationHud = notifications.get();
    legacyNotificationHud->SetViewport(WinSizeX, WinSizeY);
    runtime.Windows().Register(LegacyNotificationWindowId, std::move(notifications), UIWindowLayer::Popup);
    runtime.Windows().Open(LegacyNotificationWindowId);

    auto npc = std::make_unique<LegacyNpcDialogWindow>(190);
    npcDialogWindow = npc.get();
    runtime.Windows().Register(NpcDialogWindowId, std::move(npc), UIWindowLayer::Popup);

    auto skillNpc = std::make_unique<LegacySkillNpcWindow>(200);
    skillNpcWindow = skillNpc.get();
    runtime.Windows().Register(SkillNpcWindowId, std::move(skillNpc), UIWindowLayer::Popup);
}

void SyncLegacyDialogUI()
{
    // Legacy status/class flows arm a deferred skill-point reset through
    // sinSkillPontInitButtonFlag and historically wait for an unrelated mouse
    // button release before opening MESSAGE_INIT_SKILL_POINT. Promote that
    // deferred state directly into the V2-owned Message3 path so Event Girl
    // and class-change flows no longer depend on an invisible legacy hitbox.
    if (sinSkillPontInitButtonFlag)
    {
        bool anyMessage3 = false;
        for (int kind = 0; kind < MESSAGE3_MAX; ++kind)
        {
            if (sMessageBox3[kind].Flag)
            {
                anyMessage3 = true;
                break;
            }
        }
        if (!anyMessage3)
        {
            sinSkillPontInitButtonFlag = 0;
            cMessageBox.ShowMessage3(MESSAGE_INIT_SKILL_POINT, sinSkillPointName);
        }
    }

    // Merchant V2 already owns the confirmation and performs the sale through
    // LegacyContextGateway. Any legacy high-rank sell Message3 raised while a
    // V2 shop is active is therefore a duplicate path; cancel it before either
    // legacy rendering or the generic V2 Message3 adapter can show it again.
    if (OwnsLegacyShopUI() && sMessageBox3[MESSAGE_SELL_HIGHRANK_ITEM].Flag)
        cMessageBox.ResolveMessage3ForUIV2(MESSAGE_SELL_HIGHRANK_ITEM, false);

    if (npcDialogWindow)
    {
        npcDialogWindow->Update(0.0f);
        if (npcDialogWindow->HasSupportedDialog())
            runtime.Windows().Open(NpcDialogWindowId);
        else
            runtime.Windows().Close(NpcDialogWindowId);
    }

    if (skillNpcWindow)
    {
        if (SkillNpcFlag)
            runtime.Windows().Open(SkillNpcWindowId);
        else
            runtime.Windows().Close(SkillNpcWindowId);
    }

    if (legacyMessageWindow)
    {
        legacyMessageWindow->Update(0.0f);
        if (legacyMessageWindow->HasSupportedMessage())
            runtime.Windows().Open(LegacyMessageWindowId);
        else
            runtime.Windows().Close(LegacyMessageWindowId);
    }
}

void SyncQuestJournal()
{
#if UI_V2_QUEST_JOURNAL
    if (!questJournal)
        return;

    // External/NPC code can still call cQuestWindow.Open(). Mirror that state
    // into the V2 shell so no legacy quest surface can leak through.
    if (cQuestWindow.OpenFlag && !runtime.Windows().IsOpen(QuestJournalWindowId))
        runtime.Windows().Open(QuestJournalWindowId);
#endif
}

bool InventoryRequestedForV2()
{
    return (cInvenTory.OpenFlag || MigratedContextActive()) && !RequiresLegacyInventoryUI();
}

void SyncCharacterWindow()
{
#if UI_V2_CHARACTER_WINDOW
    if (!characterWindow)
        return;

    // Contextual systems below still depend on original inventory coordinates.
    // The NPC blacksmith is no longer in this set: v1.1 keeps its shop window
    // legacy while the character inventory itself remains V2.
    if (RequiresLegacyInventoryUI())
    {
        if (runtime.Windows().IsOpen(CharacterWindowId))
        {
            characterWindow->PrepareClose();
            runtime.Windows().Close(CharacterWindowId);
            ResetLegacyCharacterInterfaceMotion();
        }
        return;
    }

    const bool wantsAttributes = cCharStatus.OpenFlag != 0;
    const bool wantsSkills = cSkill.OpenFlag != 0;
    const bool wantsInventory = InventoryRequestedForV2();
    if (MigratedContextActive())
    {
        companionTabRequested = false;
        covenantTabRequested = false;
    }
    const bool wantsCompanions = companionTabRequested && !MigratedContextActive();
    const bool wantsCovenants = covenantTabRequested && !MigratedContextActive();
    const bool wantsWindow = wantsAttributes || wantsSkills || wantsInventory || wantsCompanions || wantsCovenants;

    if (!wantsWindow)
    {
        if (runtime.Windows().IsOpen(CharacterWindowId))
        {
            characterWindow->PrepareClose();
            runtime.Windows().Close(CharacterWindowId);
            ResetLegacyCharacterInterfaceMotion();
        }
        return;
    }

    CharacterTab desired = CharacterTab::Attributes;
    // Migrated inventory contexts still take precedence over the pure V2
    // companions surface. Otherwise keep the Companions tab pinned even with
    // all three legacy character flags closed.
    if (wantsInventory)
        desired = CharacterTab::Inventory;
    else if (wantsCovenants)
        desired = CharacterTab::Covenants;
    else if (wantsCompanions)
        desired = CharacterTab::Companions;
    else if (wantsSkills)
        desired = CharacterTab::Skills;

    if (characterWindow->ActiveTab() != desired)
        characterWindow->SetActiveTab(desired, false);
    if (!runtime.Windows().IsOpen(CharacterWindowId))
        runtime.Windows().Open(CharacterWindowId);
#endif
}

Point CurrentUIPointer()
{
    // Winmain snapshots pRealCursorPos immediately before dsMenuCursorPos()
    // remaps pCursorPos for the 800x600 legacy interface. UI V2 renders in the
    // real client/backbuffer coordinate space, therefore this is the canonical
    // pointer for the modern UI. Do not feed pCursorPos into V2 here.
    return {static_cast<int>(pRealCursorPos.x), static_cast<int>(pRealCursorPos.y)};
}

UIEvent PointerEvent(UIEventType type, MouseButton button = MouseButton::None)
{
    UIEvent event;
    event.type = type;
    event.pointer = CurrentUIPointer();
    event.button = button;
    return event;
}

void LogPointerDown(const UIEvent& event)
{
    if (!characterWindow || !runtime.Windows().IsOpen(CharacterWindowId))
        return;

    static int logCount = 0;
    if (logCount >= 64)
        return;
    ++logCount;

    RECT client{};
    GetClientRect(hwnd, &client);
    if (FILE* file = std::fopen("ui_v2_pointer.log", "a"))
    {
        std::fprintf(file,
                     "v2=%d,%d real=%ld,%ld legacy=%ld,%ld client=%ldx%ld WinSize=%dx%d tab=%d\n",
                     event.pointer.x, event.pointer.y, pRealCursorPos.x, pRealCursorPos.y,
                     pCursorPos.x, pCursorPos.y, client.right - client.left, client.bottom - client.top,
                     WinSizeX, WinSizeY, static_cast<int>(characterWindow->ActiveTab()));
        std::fclose(file);
    }
}
}

void Initialize()
{
    if (runtime.IsInitialized())
        return;
    if (!runtime.Initialize(std::make_unique<Dx9UIRenderer>()))
        return;
    runtime.OnResolutionChanged(WinSizeX, WinSizeY);
    BuildBootstrapWindow();
    BuildCharacterWindow();
    BuildPlayerHud();
    BuildCompanionPartyHud();
    BuildQuestUI();
    BuildSettingsWindow();
    BuildLegacyDialogUI();
    // MixGuide/Mixage is a legacy reference-only surface. UI V2 no longer
    // exposes or keeps it open; mixing rules remain in cCraftItem/context.
    cMixGuide::GetInstance()->Close();
    lastTick = GetTickCount();
}

void Shutdown()
{
    if (characterWindow)
        characterWindow->PrepareClose();
    characterWindow = nullptr;
    playerHud = nullptr;
    companionPartyHud = nullptr;
    questJournal = nullptr;
    questTracker = nullptr;
    legacyMessageWindow = nullptr;
    legacyNotificationHud = nullptr;
    npcDialogWindow = nullptr;
    skillNpcWindow = nullptr;
    settingsWindow = nullptr;
    questGateway.ShutdownBackend();
    companionGateway.Reset();
    covenantGateway.Reset();
    companionTabRequested = false;
    covenantTabRequested = false;
    runtime.Shutdown();
    lastTick = 0;
}

void Update()
{
    if (!runtime.IsInitialized())
        return;
    if (runtime.ViewportWidth() != WinSizeX || runtime.ViewportHeight() != WinSizeY)
        OnResolutionChanged(WinSizeX, WinSizeY);

    SyncCharacterWindow();
    SyncQuestJournal();
    SyncLegacyDialogUI();
    if (playerHud)
        playerHud->SetLegacyControlState(cInterFace.sInterFlags.RunFlag != 0,
                                         cInterFace.sInterFlags.CameraAutoFlag,
                                         cInterFace.sInterFlags.MapOnFlag != 0);
    if (cMixGuide::GetInstance()->IsOpen())
        cMixGuide::GetInstance()->Close();

    const DWORD now = GetTickCount();
    covenantGateway.Tick(now);
    const DWORD elapsed = lastTick ? (now - lastTick) : 0;
    lastTick = now;
    const float deltaSeconds = static_cast<float>(elapsed) / 1000.0f;
    runtime.Update(deltaSeconds);
}

void Render()
{
    runtime.Render();
    covenantEffectHud.Render(covenantGateway.Snapshot());
}

bool HandleLegacyMessage(int message)
{
    switch (message)
    {
        case SINLBUTTONDOWN:
        {
            UIEvent event = PointerEvent(UIEventType::PointerDown, MouseButton::Left);
            LogPointerDown(event);
            return runtime.HandleEvent(event);
        }
        case SINLBUTTONUP:
            return runtime.HandleEvent(PointerEvent(UIEventType::PointerUp, MouseButton::Left));
        case SINRBUTTONDOWN:
        {
            UIEvent event = PointerEvent(UIEventType::PointerDown, MouseButton::Right);
            LogPointerDown(event);
            return runtime.HandleEvent(event);
        }
        case SINRBUTTONUP:
            return runtime.HandleEvent(PointerEvent(UIEventType::PointerUp, MouseButton::Right));
        case SINMOUSEMOVE:
            return runtime.HandleEvent(PointerEvent(UIEventType::PointerMove));
        default:
            return false;
    }
}

bool HandleMouseWheel(int delta)
{
    UIEvent event = PointerEvent(UIEventType::Wheel);
    event.wheelDelta = delta;
    return runtime.HandleEvent(event);
}

bool HandleCharacterHotkeys()
{
#if UI_V2_CHARACTER_WINDOW
    if (!runtime.IsInitialized() || !characterWindow || RequiresLegacyInventoryUI())
        return false;

    CharacterTab requestedTab = CharacterTab::Attributes;
    bool pressed = false;

    // Consume the same keys before the old handlers see them. Besides keeping
    // one shell for C/S/V, this prevents the legacy HelpPet calls from drawing
    // *CarStatus / *Skill / *InvenTory text over the modern window.
    if (sinGetKeyClick('C'))
    {
        requestedTab = CharacterTab::Attributes;
        pressed = true;
    }
    else if (sinGetKeyClick('S'))
    {
        requestedTab = CharacterTab::Skills;
        pressed = true;
    }
    else if (sinGetKeyClick('V'))
    {
        requestedTab = CharacterTab::Inventory;
        pressed = true;
    }
    else if (sinGetKeyClick('N'))
    {
        requestedTab = CharacterTab::Covenants;
        pressed = true;
    }

    if (!pressed)
        return false;

    // Legacy C/S/V and the V2 N shortcut explicitly choose one character tab.
    companionTabRequested = requestedTab == CharacterTab::Companions;
    covenantTabRequested = requestedTab == CharacterTab::Covenants;

    // A blacksmith/shop requires the inventory to stay visible. Consume the
    // character hotkeys while that context is active and keep the shell on
    // Inventory rather than briefly closing the shop's working inventory.
    if (MigratedContextActive())
    {
        companionTabRequested = false;
        covenantTabRequested = false;
        cInvenTory.OpenFlag = SIN_OPEN;
        characterWindow->SetActiveTab(CharacterTab::Inventory, false);
        ResetLegacyCharacterInterfaceMotion();
        runtime.Windows().Open(CharacterWindowId);
        return true;
    }

    const bool isOpen = runtime.Windows().IsOpen(CharacterWindowId);
    if (isOpen && characterWindow->ActiveTab() == requestedTab)
    {
        characterWindow->PrepareClose();
        companionTabRequested = false;
        covenantTabRequested = false;
        ClearLegacyCharacterFlags();
        ResetLegacyCharacterInterfaceMotion();
        runtime.Windows().Close(CharacterWindowId);
    }
    else
    {
        ApplyLegacyTab(requestedTab);
        characterWindow->SetActiveTab(requestedTab, false);
        ResetLegacyCharacterInterfaceMotion();
        runtime.Windows().Open(CharacterWindowId);
    }
    return true;
#else
    return false;
#endif
}

bool HandleSkillShortcutHotkeys()
{
#if UI_V2_CHARACTER_WINDOW
    if (!runtime.IsInitialized() || !characterWindow ||
        !runtime.Windows().IsOpen(CharacterWindowId) ||
        !characterWindow->HasArmedSkillShortcutTarget())
        return false;

    static const int keys[8] = {VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8};
    for (int i = 0; i < 8; ++i)
    {
        if (sinGetKeyClick(keys[i]))
            return characterWindow->HandleSkillShortcut(i + 1);
    }
#endif
    return false;
}

bool HandleQuestHotkey()
{
#if UI_V2_QUEST_JOURNAL
    if (!runtime.IsInitialized() || !questJournal)
        return false;
    if (!sinGetKeyClick('U'))
        return false;

    ToggleQuestJournal();
    return true;
#else
    return false;
#endif
}

bool HandleModalHotkeys()
{
    if (!runtime.IsInitialized()) return false;
    const bool messageModal = legacyMessageWindow && runtime.Windows().IsOpen(LegacyMessageWindowId) && legacyMessageWindow->HasModalInput();
    const bool characterModal = characterWindow && runtime.Windows().IsOpen(CharacterWindowId) && characterWindow->HasModalInput();
    const bool questModal = questJournal && runtime.Windows().IsOpen(QuestJournalWindowId) && questJournal->HasModalInput();
    if (!messageModal && !characterModal && !questModal) return false;

    static const int keys[] = {VK_RETURN, VK_ESCAPE, VK_BACK,
        '0','1','2','3','4','5','6','7','8','9',
        VK_NUMPAD0,VK_NUMPAD1,VK_NUMPAD2,VK_NUMPAD3,VK_NUMPAD4,
        VK_NUMPAD5,VK_NUMPAD6,VK_NUMPAD7,VK_NUMPAD8,VK_NUMPAD9};
    for (int key : keys)
    {
        if (!sinGetKeyClick(key)) continue;
        if (messageModal) legacyMessageWindow->HandleModalKey(static_cast<std::uint32_t>(key));
        else if (characterModal) characterWindow->HandleModalKey(static_cast<std::uint32_t>(key));
        else questJournal->HandleModalKey(static_cast<std::uint32_t>(key));
        break;
    }
    return true;
}

bool HandleMainHudHotkeys()
{
#if UI_V2_PLAYER_HUD
    if (!SuppressLegacyMainHud() || HandleModalHotkeys()) return false;
    if (sinGetKeyClick('X')) { ExecuteHudShortcut(HudShortcutAction::System); return true; }
    if (sinGetKeyClick('R')) { ExecuteHudShortcut(HudShortcutAction::RunWalk); return true; }
    if (sinGetKeyClick(VK_TAB)) { ExecuteHudShortcut(HudShortcutAction::Map); return true; }
    if (sinGetKeyClick('Z')) { ExecuteHudShortcut(HudShortcutAction::Camera); return true; }
    if (sinGetKeyClick('D')) { ExecuteHudShortcut(HudShortcutAction::Party); return true; }
    if (sinGetKeyClick('Q')) { ExecuteHudShortcut(HudShortcutAction::Quests); return true; }
#endif
    return false;
}

bool IsSettingsWindowOpen()
{
    return runtime.IsInitialized() && settingsWindow != nullptr && runtime.Windows().IsOpen(SettingsWindowId);
}

void ToggleSettingsWindow()
{
    if (!runtime.IsInitialized() || !settingsWindow)
        return;

    if (runtime.Windows().IsOpen(SettingsWindowId))
    {
        runtime.Windows().Close(SettingsWindowId);
        return;
    }

    // Never stack the old settings surface behind the V2 one. Login/opening can
    // still use cSettings, but gameplay always owns Settings through this window.
    if (cSettings.isOpen())
        cSettings.Close();
    settingsWindow->ReloadFromRuntime();
    runtime.Windows().Open(SettingsWindowId);
}

void OnResolutionChanged(int width, int height)
{
    runtime.OnResolutionChanged(width, height);
    if (characterWindow)
        characterWindow->SetViewport(width, height);
    if (playerHud)
        playerHud->SetViewport(width, height);
    if (companionPartyHud)
        companionPartyHud->SetViewport(width, height);
    if (questJournal)
        questJournal->SetViewport(width, height);
    if (questTracker)
        questTracker->SetViewport(width, height);
    if (legacyMessageWindow)
        legacyMessageWindow->SetBounds({0, 0, width, height});
    if (legacyNotificationHud)
        legacyNotificationHud->SetViewport(width, height);
    if (settingsWindow)
        settingsWindow->SetViewport(width, height);
}

bool OwnsCharacterStatus()
{
#if UI_V2_CHARACTER_WINDOW && UI_V2_SUPPRESS_LEGACY_CHARACTER_UI
    return runtime.IsInitialized() && characterWindow != nullptr &&
           runtime.Windows().IsOpen(CharacterWindowId) &&
           characterWindow->ActiveTab() == CharacterTab::Attributes;
#else
    return false;
#endif
}

bool OwnsSkills()
{
#if UI_V2_CHARACTER_WINDOW && UI_V2_SUPPRESS_LEGACY_CHARACTER_UI
    return runtime.IsInitialized() && characterWindow != nullptr &&
           runtime.Windows().IsOpen(CharacterWindowId) &&
           characterWindow->ActiveTab() == CharacterTab::Skills;
#else
    return false;
#endif
}

bool OwnsInventory()
{
#if UI_V2_CHARACTER_WINDOW && UI_V2_SUPPRESS_LEGACY_CHARACTER_UI
    return runtime.IsInitialized() && characterWindow != nullptr &&
           runtime.Windows().IsOpen(CharacterWindowId) &&
           characterWindow->ActiveTab() == CharacterTab::Inventory &&
           !RequiresLegacyInventoryUI();
#else
    return false;
#endif
}

bool SuppressLegacyCharacterPanels()
{
#if UI_V2_CHARACTER_WINDOW && UI_V2_SUPPRESS_LEGACY_CHARACTER_UI
    if (!runtime.IsInitialized() || characterWindow == nullptr)
        return false;

    // Context flags can be raised by legacy NPC/shop code after the V2 Update()
    // already ran for the current frame. Do not wait one frame for the V2 window
    // to become visible before suppressing the old inventory/status/skill input.
    // Otherwise the hidden legacy inventory can still receive the opening click
    // and route it through old commerce/drop handlers.
    return runtime.Windows().IsOpen(CharacterWindowId) ||
           (MigratedContextActive() && !RequiresLegacyInventoryUI());
#else
    return false;
#endif
}

bool SuppressLegacyMainHud()
{
#if UI_V2_PLAYER_HUD && UI_V2_SUPPRESS_LEGACY_MAIN_HUD
    return runtime.IsInitialized() && playerHud != nullptr &&
           runtime.Windows().IsOpen(PlayerHudWindowId);
#else
    return false;
#endif
}

bool OwnsQuestJournalUI()
{
#if UI_V2_QUEST_JOURNAL && UI_V2_SUPPRESS_LEGACY_QUEST_UI
    return runtime.IsInitialized() && questJournal != nullptr &&
           runtime.Windows().IsOpen(QuestJournalWindowId);
#else
    return false;
#endif
}

bool BlocksWorldInputAt(int x, int y)
{
#if (UI_V2_CHARACTER_WINDOW && UI_V2_SUPPRESS_LEGACY_CHARACTER_UI) || UI_V2_QUEST_JOURNAL
    if (!runtime.IsInitialized())
        return false;
    if (runtime.PointerCapture().Captured() != nullptr)
        return true;
    // The permanent bottom HUD is interactive UI too.  SetMousePlay() asks
    // this function before starting world movement, so include the HUD bounds
    // just like inventory/quest windows instead of letting clicks pass through.
    if (playerHud && runtime.Windows().IsOpen(PlayerHudWindowId) &&
        playerHud->GlobalBounds().Contains(Point{x, y}))
        return true;
    if ((legacyMessageWindow && legacyMessageWindow->HasModalInput()) ||
        (characterWindow && characterWindow->HasModalInput()) ||
        (questJournal && questJournal->HasModalInput()))
        return true;
    if (characterWindow && runtime.Windows().IsOpen(CharacterWindowId))
    {
        // Do not globally block world input just because the V2 inventory owns
        // a MouseItem.  The original game intentionally supports picking an
        // inventory item and clicking outside the inventory to throw/drop it on
        // the field.  Window bounds still block clicks over the V2 UI itself.
        if (characterWindow->GlobalBounds().Contains(Point{x, y}))
            return true;
    }
    if (questJournal && runtime.Windows().IsOpen(QuestJournalWindowId) &&
        questJournal->GlobalBounds().Contains(Point{x, y}))
        return true;
    if (npcDialogWindow && runtime.Windows().IsOpen(NpcDialogWindowId) &&
        npcDialogWindow->GlobalBounds().Contains(Point{x, y}))
        return true;
    if (skillNpcWindow && runtime.Windows().IsOpen(SkillNpcWindowId) &&
        skillNpcWindow->GlobalBounds().Contains(Point{x, y}))
        return true;
    if (settingsWindow && runtime.Windows().IsOpen(SettingsWindowId) &&
        settingsWindow->GlobalBounds().Contains(Point{x, y}))
        return true;
    return false;
#else
    (void)x; (void)y;
    return false;
#endif
}

void ClearLegacyCharacterHelpOverlay()
{
#if UI_V2_CHARACTER_WINDOW && UI_V2_SUPPRESS_LEGACY_CHARACTER_UI
    if ((!SuppressLegacyCharacterPanels() && !SuppressLegacyMainHud()) || !cHelpPet.HelpMsgFlag)
        return;

    if (std::strcmp(cHelpPet.HelpMsgId, "*CarStatus") == 0 ||
        std::strcmp(cHelpPet.HelpMsgId, "*InvenTory") == 0 ||
        std::strcmp(cHelpPet.HelpMsgId, "*Skill") == 0 ||
        std::strcmp(cHelpPet.HelpMsgId, "*Party") == 0 ||
        std::strcmp(cHelpPet.HelpMsgId, "*Quest") == 0 ||
        std::strcmp(cHelpPet.HelpMsgId, "*System") == 0 ||
        std::strcmp(cHelpPet.HelpMsgId, "*RunAndWalk") == 0 ||
        std::strcmp(cHelpPet.HelpMsgId, "*Camera_Hand") == 0 ||
        std::strcmp(cHelpPet.HelpMsgId, "*Camera_Fix") == 0 ||
        std::strcmp(cHelpPet.HelpMsgId, "*Camera_Auto") == 0 ||
        std::strcmp(cHelpPet.HelpMsgId, "*Mini_Map") == 0 ||
        std::strcmp(cHelpPet.HelpMsgId, "*ChatBox") == 0 ||
        std::strcmp(cHelpPet.HelpMsgId, "*Bar_Life") == 0 ||
        std::strcmp(cHelpPet.HelpMsgId, "*Bar_Mana") == 0 ||
        std::strcmp(cHelpPet.HelpMsgId, "*Bar_Stamina") == 0 ||
        std::strcmp(cHelpPet.HelpMsgId, "*Bar_Exp") == 0 ||
        std::strcmp(cHelpPet.HelpMsgId, "*Exp_Num") == 0 ||
        std::strcmp(cHelpPet.HelpMsgId, "*DayandNight") == 0)
    {
        cHelpPet.HelpMsgFlag = 0;
        cHelpPet.HelpMsgTime = 0;
    }
#endif
}

bool IsCharacterWindowOpen()
{
    return runtime.IsInitialized() && runtime.Windows().IsOpen(CharacterWindowId);
}

bool HasMigratedContext()
{
    return runtime.IsInitialized() && MigratedContextActive() && !RequiresLegacyInventoryUI();
}

bool OwnsLegacyShopUI()
{
#if UI_V2_CHARACTER_WINDOW && UI_V2_SUPPRESS_LEGACY_CHARACTER_UI
    if (!runtime.IsInitialized() || RequiresLegacyInventoryUI())
        return false;

    // Own the legacy commerce surface from the instant any shop context becomes
    // active, even if SyncCharacterWindow() has not run yet in this frame. Also
    // keep ownership while a stale legacy slide offset exists so the old shop
    // can never flash during an open/close transition.
    return cShop.OpenFlag || cCharShop.OpenFlag || cMyShop.OpenFlag ||
           sinMoveKindInter[SIN_SHOP] > 0 ||
           sinMoveKindInter[SIN_CHARSHOP] > 0 ||
           sinMoveKindInter[SIN_MYSHOP] > 0;
#else
    return false;
#endif
}

bool OwnsLegacyTradeUI()
{
#if UI_V2_CHARACTER_WINDOW && UI_V2_SUPPRESS_LEGACY_CHARACTER_UI
    return runtime.IsInitialized() && !RequiresLegacyInventoryUI() && LegacyTradeContextActiveOrMoving();
#else
    return false;
#endif
}

bool OwnsLegacyCoinShopUI()
{
#if UI_V2_CHARACTER_WINDOW && UI_V2_SUPPRESS_LEGACY_CHARACTER_UI
    return runtime.IsInitialized() && !RequiresLegacyInventoryUI() &&
           (cCoinShop.isOpen() || cCoinShop_T.isOpen());
#else
    return false;
#endif
}

bool OwnsLegacyRestorationUI()
{
#if UI_V2_CHARACTER_WINDOW && UI_V2_SUPPRESS_LEGACY_CHARACTER_UI
    return runtime.IsInitialized() && !RequiresLegacyInventoryUI() && cRestaure.OpenFlag;
#else
    return false;
#endif
}

bool OwnsLegacyHelpUI()
{
    if (!runtime.IsInitialized() || npcDialogWindow == nullptr) return false;
    // Help can be opened by legacy NPC code after UI V2 Update() has already
    // run for this frame. Own the raw legacy state immediately so cSinHelp can
    // never flash for one frame before the V2 popup is opened next Update().
    return LegacyNpcDialogWindow::HasActiveSupportedLegacyDialog() ||
           (runtime.Windows().IsOpen(NpcDialogWindowId) && npcDialogWindow->HasSupportedDialog());
}

bool OwnsLegacyMessage3UI()
{
    if (!runtime.IsInitialized() || legacyMessageWindow == nullptr) return false;
    // Event Girl uses the legacy INITPOINT help document as its V2 NPC dialog.
    // It still keeps MESSAGE_EVENT_GYUNGFUM active as the authoritative action
    // token, so claim that Message3 here even though LegacyMessageWindow skips
    // it. This prevents the hidden legacy popup from drawing/receiving input.
    if (LegacyNpcDialogWindow::HasActiveEventGiftDialog())
        return true;

    // Same-frame Message3 creation used to leak one frame of the old popup.
    // Raw-state ownership suppresses it immediately; SyncLegacyDialogUI opens
    // the V2 modal on the following Update().
    return LegacyMessageWindow::HasActiveSupportedLegacyMessage() ||
           (runtime.Windows().IsOpen(LegacyMessageWindowId) && legacyMessageWindow->HasSupportedMessage());
}

bool OwnsLegacyNotificationUI()
{
    return runtime.IsInitialized() && legacyNotificationHud != nullptr &&
           runtime.Windows().IsOpen(LegacyNotificationWindowId);
}

bool OwnsLegacySkillNpcUI()
{
    if (!runtime.IsInitialized()) return false;
    // SkillNpcFlag may be raised after Update() by the network/NPC path. Claim
    // it immediately so the old master panel cannot flash for one frame.
    if (SkillNpcFlag) return true;
    if (skillNpcWindow && runtime.Windows().IsOpen(SkillNpcWindowId)) return true;
    return SkillMasterFlag != 0 && OwnsSkills();
}

bool SuppressLegacyMixGuide()
{
    return runtime.IsInitialized();
}

bool SuppressLegacyContextEffects()
{
#if UI_V2_CHARACTER_WINDOW && UI_V2_SUPPRESS_LEGACY_CHARACTER_UI
    return runtime.IsInitialized() && !RequiresLegacyInventoryUI() &&
           (MigratedContextActive() || LegacyTradeContextActiveOrMoving());
#else
    return false;
#endif
}
}
