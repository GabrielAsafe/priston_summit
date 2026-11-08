#include "systems/ui/v2/legacy/LegacyUIV2Bridge.h"

#include <algorithm>
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
#include "systems/ui/v2/legacy/LegacyMixGuideGateway.h"
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
#include "systems/ui/v2/crafting/MixGuideWindow.h"
#include "systems/ui/v2/crafting/ProfessionTestPanel387.h" // SUMMIT_387_HEADER
#include "systems/ui/v2/crafting/ProfessionCraftWindow378.h"
#include "systems/ui/v2/help/GuideWindow.h"
#include "systems/ui/v2/social/SocialWindow.h"
#include "systems/ui/v2/social/PartyStatusHud.h"
#include "systems/ui/v2/rendering/Dx9UIRenderer.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "content/v1/localization/content/LocalizationRuntime.h"
#include "systems/ui/v2/widgets/Label.h"
#include "systems/ui/v2/widgets/Button.h"
#include "systems/world/field/field.h"
#include "core/gameplay/runtime/playmain.h"
#include "systems/rendering/legacy_3d/smType.h"
#include "core/data/legacy_loader/fileread.h"
#include <array>
#include <algorithm>
#include <vector>
#include <functional>
#include <deque>
#include <mutex>
#include "systems/ui/v2/crafting/CraftNetwork407.h"
#ifdef GAME_GM
extern int CustomField;
#endif
#include "systems/ui/v2/widgets/Panel.h"
#include "systems/ui/v2/windows/CharacterWindow.h"
#include "systems/ui/v2/windows/SettingsWindow.h"
#include "systems/network/client/netplay.h"

extern int SetChatingLine(char* szMessage);
extern bool SummitApplyGMLevel(int desiredLevel);
// SUMMIT_390A_RATE_LIMIT_GLOBALS: defined once in netplay.cpp.
extern DWORD dwLastChatTime;
extern DWORD dwContiueChatCount;
// PATCH 220: restart state flags are defined in sinInterFace.cpp.
extern int CheckExpFlag;
extern int CheckExpFlag2;

namespace ui::v2::legacy
{
namespace
{
std::mutex craftMutex407;
std::deque<std::string> craftMessages407;
}
// This is called only from the already decoded server network packet route.
// Network thread never modifies UI widgets directly.
bool ReceiveServerCraft407(const char* message) {
    if(!message || std::strncmp(message,"@S407|",6)!=0)return false;
    const char* end=static_cast<const char*>(std::memchr(message,0,201));
    if(!end)return true;
    const std::size_t length=static_cast<std::size_t>(end-message);
    if(length<10 || length>200)return true;
    std::lock_guard<std::mutex> lock(craftMutex407);
    if(craftMessages407.size()>=128)craftMessages407.pop_front();
    craftMessages407.emplace_back(message,length);
    return true; // internal protocol must not pollute normal chat
}
namespace
{
#ifdef GAME_GM
constexpr WindowId GMWarpListWindowId = 0x5549563257415250ULL; // "UIV2WARP"
constexpr WindowId GMSpawnEditorWindowId = 0x5549563253504E33ULL; // "UIV2SPN3"
#endif
constexpr WindowId BootstrapWindowId = 0x55495632424F4F54ULL; // "UIV2BOOT"
constexpr WindowId CharacterWindowId = 0x5549563243484152ULL; // "UIV2CHAR"
constexpr WindowId PlayerHudWindowId = 0x5549563248554431ULL; // "UIV2HUD1"
constexpr WindowId CompanionPartyHudWindowId = 0x5549563243485544ULL; // "UIV2CHUD"
constexpr WindowId QuestJournalWindowId = 0x5549563251554553ULL; // "UIV2QUES"
constexpr WindowId QuestTrackerWindowId = 0x554956325452414BULL; // "UIV2TRAK"
constexpr WindowId ProfessionCraft378Id = 0x5549563250524F38ULL;
constexpr WindowId ProfessionTest387Id = 0x5549563250545337ULL; // SUMMIT_387_ID
constexpr WindowId MixGuideWindowId = 0x554956324D495847ULL; // "UIV2MIXG"
constexpr WindowId LegacyMessageWindowId = 0x554956324D534733ULL; // "UIV2MSG3"
constexpr WindowId LegacyNotificationWindowId = 0x554956324E4F5446ULL;
constexpr WindowId DeathWindowId = 0x5549563244454154ULL; // UIV2DEAT // "UIV2NOTF"
constexpr WindowId NpcDialogWindowId = 0x554956324E504344ULL; // "UIV2NPCD"
constexpr WindowId SkillNpcWindowId = 0x55495632534B4E50ULL; // "UIV2SKNP"
constexpr WindowId SettingsWindowId = 0x5549563253455454ULL; // "UIV2SETT"
constexpr WindowId GuideWindowId = 0x5549563247554944ULL; // "UIV2GUID"
constexpr WindowId SocialWindowId = 0x55495632534F434CULL; // "UIV2SOCL"
constexpr WindowId PartyStatusHudId = 0x5549563250485544ULL; // "UIV2PHUD"
#ifdef GAME_GM
class GMWarpListWindow;
#endif
// PATCH 219: V2 is the only death-screen presentation and input owner.
// The original restart state/timeout/network lifecycle remain authoritative.
class DeathWindow final : public Window
{
public:
    explicit DeathWindow(UIId id) : Window(id, {})
    {
        SetMovable(false);
        SetClosable(false);
        SetHitTestVisible(true);
        SetViewport(WinSizeX, WinSizeY);
    }
    void SetViewport(int width, int height) { SetBounds({0, 0, width, height}); pressed_ = -1; }
    UIEventResult OnEvent(const UIEvent& event) override
    {
        if (!ReStartFlag) return UIEventResult::Ignored;
        if (event.type == UIEventType::PointerDown && event.button == MouseButton::Left)
        {
            pressed_ = HitOption(event.pointer);
            return UIEventResult::Handled;
        }
        if (event.type == UIEventType::PointerUp && event.button == MouseButton::Left)
        {
            const int selected = HitOption(event.pointer);
            if (selected >= 0 && selected == pressed_ && (selected != 0 || FieldAllowed()))
                Choose(selected);
            pressed_ = -1;
            return UIEventResult::Handled;
        }
        // Modal: no background hotkeys or interactions while dead.
        return UIEventResult::Handled;
    }
protected:
    void OnRender(UIRenderContext& context) override
    {
        if (!ReStartFlag) return;
        const Rect full = GlobalBounds();
        auto& r = context.Renderer();
        r.DrawRect(full, Color{5, 9, 17, 185});
        const Rect panel = PanelRect();
        r.DrawRect(panel, UITheme::WindowBg);
        r.DrawRect({panel.x, panel.y, panel.width, 3}, UITheme::Accent);
        r.DrawRect({panel.x, panel.y + panel.height - 1, panel.width, 1}, UITheme::Border);
        r.DrawTextUtf8(FontHandle{}, Text("ui.death.title"),
                       {panel.x + 22, panel.y + 19, panel.width - 44, 29},
                       TextStyle{UITheme::Text, true, TextAlign::Center});
        r.DrawTextUtf8(FontHandle{}, Text("ui.death.description"),
                       {panel.x + 22, panel.y + 56, panel.width - 44, 25},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Center});
        for (int i = 0; i < 3; ++i)
        {
            const Rect button = OptionRect(i);
            const bool allowed = i != 0 || FieldAllowed();
            r.DrawRect(button, allowed ? UITheme::SurfaceRaised : UITheme::Surface);
            r.DrawRect({button.x, button.y, button.width, 2}, allowed ? UITheme::AccentMuted : UITheme::BorderSoft);
            const char* key = i == 0 ? "ui.death.field" : (i == 1 ? "ui.death.town" : "ui.death.exit");
            r.DrawTextUtf8(FontHandle{}, Text(key),
                           {button.x + 6, button.y + 11, button.width - 12, 23},
                           TextStyle{allowed ? UITheme::Text : UITheme::Disabled, false, TextAlign::Center});
        }
        const char* hint = FieldAllowed() ? "ui.death.timeout" : "ui.death.field_blocked";
        r.DrawTextUtf8(FontHandle{}, Text(hint),
                       {panel.x + 20, panel.y + 193, panel.width - 40, 24},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Center});
    }
private:
    int pressed_ = -1;
    static std::string Text(const char* key)
    {
        std::string value;
        if (summit::content::localization::GetLocalizationValue(key, value) && !value.empty())
            return value;
        return key; // Missing translation is visible, never silently hardcoded.
    }
    static bool FieldAllowed()
    {
        if (RestartCheckExp) return false;
        // Keep the original castle restriction; do not bypass gameplay rules.
        if (lpCurPlayer && lpCurPlayer->OnStageField >= 0 &&
            StageField[lpCurPlayer->OnStageField] &&
            StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_CASTLE)
            return false;
        return true;
    }
    Rect PanelRect() const
    {
        const Rect full = GlobalBounds();
        const int width = std::min(520, std::max(320, full.width - 28));
        const int height = 242;
        return {full.x + (full.width - width) / 2, full.y + (full.height - height) / 2, width, height};
    }
    Rect OptionRect(int i) const
    {
        const Rect p = PanelRect();
        const int gap = 10;
        const int margin = 19;
        const int width = (p.width - margin * 2 - gap * 2) / 3;
        return {p.x + margin + (width + gap) * i, p.y + 120, width, 52};
    }
    int HitOption(Point point) const
    {
        for (int i = 0; i < 3; ++i)
            if (OptionRect(i).Contains(point)) return i;
        return -1;
    }
    static void Choose(int i)
    {
        if (i == 0 && !FieldAllowed()) return;
        ReStartOptionIndex = i == 0 ? RESTART_FEILD : (i == 1 ? RESTART_TOWN : RESTART_EXIT);
        ReStartFlag = 0;
        RestartCheckExp = 0;
        ReStartIndex = 0;
        CheckExpFlag2 = 1;
        CheckExpFlag = 0;
    }
};

UIRuntime runtime;
DWORD lastTick = 0;
CharacterWindow* characterWindow = nullptr;
PlayerHud* playerHud = nullptr;
CompanionPartyHud* companionPartyHud = nullptr;
QuestJournalWindow* questJournal = nullptr;
QuestTrackerHud* questTracker = nullptr;
MixGuideWindow* mixGuideWindow = nullptr;
ProfessionTestPanel387* professionTestPanel387 = nullptr; // SUMMIT_387_PTR
ProfessionCraftWindow378* professionCraftWindow378=nullptr;
LegacyMessageWindow* legacyMessageWindow = nullptr;
LegacyNotificationHud* legacyNotificationHud = nullptr;
DeathWindow* deathWindow = nullptr;
LegacyNpcDialogWindow* npcDialogWindow = nullptr;
LegacySkillNpcWindow* skillNpcWindow = nullptr;
SettingsWindow* settingsWindow = nullptr;
#ifdef GAME_GM
GMWarpListWindow* gmWarpListWindow = nullptr;
class GMSpawnEditorWindow;
GMSpawnEditorWindow* gmSpawnEditorWindow = nullptr;
#endif
GuideWindow* guideWindow = nullptr;
SocialWindow* socialWindow = nullptr;
PartyStatusHud* partyStatusHud = nullptr;
LegacyCharacterStatsGateway statsGateway;
LegacySkillGateway skillGateway;
LegacyInventoryGateway inventoryGateway;
LegacyContextGateway contextGateway;
LegacyQuestGateway questGateway;
LegacyMixGuideGateway mixGuideGateway;
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
    if (tab == CharacterTab::Companions || tab == CharacterTab::Covenants || tab == CharacterTab::Crafting) // SUMMIT392_B_LEGACY
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
    // SUMMIT392_B_CONTEXT: server-authorized text commands; never fabricate success.
    characterWindow->SetCraftingContext(
        []() -> std::string { return GameMode == 2 && lpCurPlayer ? std::string(lpCurPlayer->smCharInfo.szName) : std::string(); },
        [](const std::string& command) -> bool {
            if (GameMode != 2 || !lpCurPlayer || command.size() < 10 || command.size() > 200 ||
                (command.compare(0, 12, "/craft make ") != 0 &&
                 command.compare(0, 12, "/craft sync ") != 0 &&
                 command.compare(0, 12, "/craft plan ") != 0 && // SUMMIT426A_CRAFT_BRIDGE
                 command.compare(0, 13, "/craft check ") != 0)) return false;
            char text[208] = {};
            std::memcpy(text, command.data(), command.size());
            return SendChatMessageToServer(text)!=0; // transport result, not recipe ACK
        });
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
        covenantTabRequested = tab == CharacterTab::Covenants || tab == CharacterTab::Crafting; // SUMMIT392_B_TAB_REQUEST
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
    covenantTabRequested = tab == CharacterTab::Covenants || tab == CharacterTab::Crafting; // SUMMIT392_B_TOGGLE
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
        case HudShortcutAction::Party: OpenSocialWindow(); break;
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


void BuildProfessionCraft378UI()
{
    auto window=std::make_unique<ProfessionCraftWindow378>(3780);
    professionCraftWindow378=window.get();
    professionCraftWindow378->SetViewport(WinSizeX,WinSizeY);
    professionCraftWindow378->SetCommand([](const std::string& command)->bool {
        if(GameMode!=2 || !lpCurPlayer || command.empty() || command.size()>100)return false;
        char text[128]={0};std::memcpy(text,command.c_str(),command.size());
        return SendChatMessageToServer(text)!=0;
    });
    professionCraftWindow378->SetOnClose([](){runtime.Windows().Close(ProfessionCraft378Id);});
    runtime.Windows().Register(ProfessionCraft378Id,std::move(window),UIWindowLayer::NormalWindow);
}

// SUMMIT_387_BUILDER: all commands use the already-authenticated legacy transport.
void BuildProfessionTest387UI()
{
    auto window = std::make_unique<ProfessionTestPanel387>(3870);
    professionTestPanel387 = window.get();
    professionTestPanel387->SetViewport(WinSizeX, WinSizeY);
    // SUMMIT_390A_SEND_DIAGNOSTIC: preserve legacy rate limits and server authority.
    // A successful handoff is NOT a successful craft or an authenticated server ACK.
    professionTestPanel387->SetCommand([](const std::string& command) -> std::string {
        if (GameMode != 2 || !lpCurPlayer)
            return "NAO ENVIADO: personagem fora do mundo. Entra no jogo.";
        if (command.empty() || command.size() > 78)
            return "NAO ENVIADO: comando vazio ou excede o limite legado de 78 bytes.";
        if (!smWsockServer)
            return "NAO ENVIADO: socket principal indisponivel. Verifica a ligacao.";
        if (lpCurPlayer->OnStageField < 0 || lpCurPlayer->OnStageField >= 2 ||
            !StageField[lpCurPlayer->OnStageField])
            return "NAO ENVIADO: mapa ainda nao carregado. Tenta depois de entrar.";
        if (dwLastChatTime && dwLastChatTime > dwPlayTime)
            return "NAO ENVIADO: chat temporariamente bloqueado. Aguarda e repete.";
        if (dwLastChatTime && dwPlayTime >= dwLastChatTime &&
            (dwPlayTime - dwLastChatTime) < 2000 && dwContiueChatCount >= 10)
            return "NAO ENVIADO: limite de chat atingido. Espera 2 segundos.";
        char text[128] = {};
        std::memcpy(text, command.data(), command.size());
        if (!SendChatMessageToServer(text))
            return "NAO ENVIADO: transporte legado recusou (limite de chat). Tenta depois.";
        return "PEDIDO ENCAMINHADO: " + command + " | confirma no servidor/recibo SQLite.";
    });
    professionTestPanel387->SetOnClose([]() { runtime.Windows().Close(ProfessionTest387Id); });
    runtime.Windows().Register(ProfessionTest387Id, std::move(window), UIWindowLayer::NormalWindow);
}

void BuildMixGuideUI()
{
    auto window = std::make_unique<MixGuideWindow>(mixGuideGateway, 170);
    mixGuideWindow = window.get();
    mixGuideWindow->SetViewport(WinSizeX, WinSizeY);
    mixGuideWindow->SetOnClose([]()
    {
        runtime.Windows().Close(MixGuideWindowId);
    });
    runtime.Windows().Register(MixGuideWindowId, std::move(window), UIWindowLayer::NormalWindow);
}

// SUMMIT_PATCH_214_GM_WARP_LIST_V2_BEGIN
#ifdef GAME_GM
// The developer warp panel deliberately uses the same permission boundary and
// queued WarpCustomField path as the pre-existing GM Shift+F6 shortcut.
class GMWarpListWindow final : public Window
{
public:
    using SelectHandler = std::function<void(int)>;
    static constexpr int Rows = 11;

    explicit GMWarpListWindow(UIId id) : Window(id, "GM - Warp Gates")
    {
        SetBounds({50, 30, 750, 525});
        SetTitleBarHeight(34);
        SetMovable(true);
        SetClosable(true);

        auto& close = AddChild<Button>(3101, "X");
        close.SetOnClick([this](const UIEvent&) { if (onClose_) onClose_(); });
        close_ = &close;

        auto& back = AddChild<Button>(3102, "< Anterior");
        back.SetOnClick([this](const UIEvent&) {
            if (page_ > 0) { --page_; Refresh(); }
        });
        back_ = &back;

        auto& next = AddChild<Button>(3103, "Proxima >");
        next.SetOnClick([this](const UIEvent&) {
            if ((page_ + 1) * Rows < static_cast<int>(fields_.size())) { ++page_; Refresh(); }
        });
        next_ = &next;

        auto& indicator = AddChild<Label>(3104, "");
        indicator_ = &indicator;

        for (int row = 0; row < Rows; ++row)
        {
            auto& button = AddChild<Button>(3110 + row, "");
            button.SetOnClick([this, row](const UIEvent&) {
                const int index = page_ * Rows + row;
                if (index < 0 || index >= static_cast<int>(fields_.size()) || !onSelect_)
                    return;
                onSelect_(fields_[static_cast<std::size_t>(index)]);
            });
            entries_[static_cast<std::size_t>(row)] = &button;
        }
        Layout();
    }

    void SetOnClose(std::function<void()> handler) { onClose_ = std::move(handler); }
    void SetOnSelect(SelectHandler handler) { onSelect_ = std::move(handler); }

    void SetViewport(int width, int height)
    {
        Rect b = Bounds();
        b.width = std::min(750, std::max(460, width - 24));
        b.height = std::min(525, std::max(380, height - 24));
        b.x = std::max(0, (width - b.width) / 2);
        b.y = std::max(0, (height - b.height) / 2);
        SetBounds(b);
        Layout();
    }

    void Reload()
    {
        fields_.clear();
        const int count = std::min(FIELD_MAX, std::max(0, FieldCount));
        for (int i = 0; i < count; ++i)
        {
            if (sField[i].szName[0])
                fields_.push_back(i);
        }
        page_ = 0;
        Refresh();
    }

    UIEventResult OnEvent(const UIEvent& event) override
    {
        if (event.type == UIEventType::Wheel && HitTest(event.pointer))
        {
            const int nextPage = page_ + (event.wheelDelta < 0 ? 1 : -1);
            const int maxPage = fields_.empty() ? 0 : static_cast<int>((fields_.size() - 1) / Rows);
            page_ = std::max(0, std::min(maxPage, nextPage));
            Refresh();
            return UIEventResult::Handled;
        }
        return Window::OnEvent(event);
    }

private:
    void Layout()
    {
        const Rect b = Bounds();
        if (close_) close_->SetBounds({b.width - 44, 5, 32, 24});
        const int available = b.height - 110;
        const int rowHeight = std::max(23, available / Rows);
        for (int row = 0; row < Rows; ++row)
            entries_[static_cast<std::size_t>(row)]->SetBounds(
                {18, 49 + row * rowHeight, b.width - 36, rowHeight - 3});
        const int footer = b.height - 47;
        if (back_) back_->SetBounds({18, footer, 132, 28});
        if (indicator_) indicator_->SetBounds({b.width / 2 - 90, footer + 4, 180, 24});
        if (next_) next_->SetBounds({b.width - 150, footer, 132, 28});
    }

    void Refresh()
    {
        for (int row = 0; row < Rows; ++row)
        {
            Button* button = entries_[static_cast<std::size_t>(row)];
            const int index = page_ * Rows + row;
            if (index >= static_cast<int>(fields_.size()))
            {
                button->SetVisible(false);
                continue;
            }
            const int field = fields_[static_cast<std::size_t>(index)];
            const sFIELD& info = sField[field];
            char label[224];
            const bool current = (StageField[0] == &sField[field] || StageField[1] == &sField[field]);
            std::snprintf(label, sizeof(label), "%s %03d  %-36.36s   Sky %02X/%02X/%02X",
                current ? "*" : " ", field, info.szName,
                info.BackImageCode[0] & 0xFF,
                info.BackImageCode[1] & 0xFF,
                info.BackImageCode[2] & 0xFF);
            button->SetText(label);
            button->SetVisible(true);
        }
        if (back_) back_->SetEnabled(page_ > 0);
        if (next_) next_->SetEnabled((page_ + 1) * Rows < static_cast<int>(fields_.size()));
        if (indicator_)
        {
            char status[64];
            std::snprintf(status, sizeof(status), "%d / %d  (%d)",
                page_ + 1, std::max(1, static_cast<int>((fields_.size() + Rows - 1) / Rows)),
                static_cast<int>(fields_.size()));
            indicator_->SetText(status);
        }
    }

    std::array<Button*, Rows> entries_{};
    Button* close_ = nullptr;
    Button* back_ = nullptr;
    Button* next_ = nullptr;
    Label* indicator_ = nullptr;
    std::vector<int> fields_;
    int page_ = 0;
    SelectHandler onSelect_;
    std::function<void()> onClose_;
};

// SUMMIT363: native V2 authoring surface. It does not render legacy panels.
// The already authenticated /spawnplan server ingress is a temporary transport;
// plans remain disabled until their monster templates are checked/published.
class GMSpawnEditorWindow final : public Window
{
public:
    explicit GMSpawnEditorWindow(UIId id) : Window(id, "GM - Pontos de spawn")
    {
        SetBounds({125, 90, 530, 360});
        SetTitleBarHeight(34);
        SetMovable(true);
        SetClosable(true);
        auto& close=AddChild<Button>(3701,"X");
        close.SetBounds({482,5,30,24});
        close.SetOnClick([this](const UIEvent&) { if(onClose_) onClose_(); });
        auto& model=AddChild<Button>(3702,"Modelo");
        model.SetBounds({20,60,490,32});
        model.SetOnClick([this](const UIEvent&) { model_=(model_+1)%3; Refresh(); });
        modelButton_=&model;
        auto& density=AddChild<Button>(3703,"Densidade");
        density.SetBounds({20,110,490,32});
        density.SetOnClick([this](const UIEvent&) { density_=(density_+1)%3; Refresh(); });
        densityButton_=&density;
        auto& save=AddChild<Button>(3704,"Adicionar ponto nesta posicao");
        save.SetBounds({20,207,490,36});
        save.SetOnClick([this](const UIEvent&) { Submit(); });
        saveButton_=&save;
        auto& info=AddChild<Label>(3705,"");
        info.SetBounds({20,158,485,38});
        info.SetStyle(TextStyle{UITheme::TextMuted,false,TextAlign::Left});
        info_=&info;
        auto& status=AddChild<Label>(3706,"");
        status.SetBounds({20,258,485,23});
        status.SetStyle(TextStyle{UITheme::Text,false,TextAlign::Left});
        status_=&status;
        auto& note=AddChild<Label>(3707,"Os pontos gravados NAO geram monstros ate serem publicados.");
        note.SetBounds({20,294,490,24});
        note.SetStyle(TextStyle{UITheme::TextMuted,false,TextAlign::Left});
        Refresh();
    }
    void SetOnClose(std::function<void()> close) { onClose_=std::move(close); }
    void ResetStatus() { status_->SetText("Seleciona modelo e densidade; o servidor usa a tua posicao atual."); }
private:
    void Refresh()
    {
        static const char* models[]={"undeadminer","monplant","turtlecannon"};
        static const char* densities[]={"low","medium","high"};
        static const char* labels[]={"Baixa (2)","Media (6)","Alta (12)"};
        modelButton_->SetText(std::string("Modelo experimental: ")+models[model_]);
        densityButton_->SetText(std::string("Densidade: ")+labels[density_]);
        info_->SetText("Posicao: capturada do jogador no servidor ao confirmar.");
    }
    void Submit()
    {
        if(GameMode!=2 || !lpCurPlayer || smConfig.getSecurity()<AccountTypes::SEC_MODERATOR)
        { status_->SetText("Sem permissao GM para guardar pontos."); return; }
        static const char* models[]={"undeadminer","monplant","turtlecannon"};
        static const char* densities[]={"low","medium","high"};
        char command[112]={};
        std::snprintf(command,sizeof(command),"/spawnplan add %s %s",models[model_],densities[density_]);
        // The server resolves coordinates from its authenticated session;
        // the client never sends claimed world positions or a player name.
        if(SendChatMessageToServer(command))
            status_->SetText("Pedido enviado. Confirma a gravacao no chat do servidor.");
        else
            status_->SetText("Falha ao enviar pedido. Nenhum ponto foi confirmado.");
    }
    int model_=0, density_=0;
    Button* modelButton_=nullptr;
    Button* densityButton_=nullptr;
    Button* saveButton_=nullptr;
    Label* info_=nullptr;
    Label* status_=nullptr;
    std::function<void()> onClose_;
};

void BuildGMSpawnEditorWindow()
{
    auto window=std::make_unique<GMSpawnEditorWindow>(3700);
    gmSpawnEditorWindow=window.get();
    gmSpawnEditorWindow->SetOnClose([](){runtime.Windows().Close(GMSpawnEditorWindowId);});
    runtime.Windows().Register(GMSpawnEditorWindowId,std::move(window),UIWindowLayer::NormalWindow);
}

void BuildGMWarpListWindow()
{
    auto window = std::make_unique<GMWarpListWindow>(240);
    gmWarpListWindow = window.get();
    gmWarpListWindow->SetViewport(WinSizeX, WinSizeY);
    gmWarpListWindow->SetOnClose([]() { runtime.Windows().Close(GMWarpListWindowId); });
    gmWarpListWindow->SetOnSelect([](int field) {
        if (smConfig.getSecurity() < AccountTypes::SEC_MODERATOR ||
            field < 0 || field >= FIELD_MAX || field >= FieldCount || !sField[field].szName[0])
            return;
        // Do not teleport directly from a UI callback. The gameplay loop will
        // execute its existing deferred GAME_GM / WarpCustomField transition.
        ::CustomField = field;
        SkipNextField = 2;
        runtime.Windows().Close(GMWarpListWindowId);
    });
    runtime.Windows().Register(GMWarpListWindowId, std::move(window), UIWindowLayer::NormalWindow);
}
#endif
// SUMMIT_PATCH_214_GM_WARP_LIST_V2_END

void BuildSettingsWindow()
{
    auto window = std::make_unique<SettingsWindow>(210);
    settingsWindow = window.get();
    settingsWindow->SetViewport(WinSizeX, WinSizeY);
    // SUMMIT 355: reutiliza autorizacao e parser reais; nao ha bypass de security.
    settingsWindow->SetGmBridge(
        []() -> int { return GameMode == 2 ? static_cast<int>(smConfig.getSecurity()) : 0; },
        [](const std::string& command, bool local) -> bool {
            if (GameMode != 2 || !lpCurPlayer || command.empty() || command.size() > 88)
                return false;
            if (local) {
                const size_t space = command.find(' ');
                if(space==std::string::npos) return false;
                const std::string action=command.substr(0,space);
                if(action!="/level" && action!="/Levelup") return false;
                const int level=std::atoi(command.c_str()+space+1);
                return SummitApplyGMLevel(level);
            }
            char message[256] = {};
            std::memcpy(message, command.c_str(), command.size());
            return SendChatMessageToServer(message) != 0;
        },
        []() -> std::vector<std::string> {
            std::vector<std::string> names;
            if(GameMode != 2 || !lpCurPlayer) return names;
            const char* self=lpCurPlayer->smCharInfo.szName;
            if(self && self[0])names.emplace_back(self);
            for(int i=0;i<OTHER_PLAYER_MAX && names.size()<128;++i) {
                const smCHAR& other=chrOtherPlayer[i];
                if(!other.Flag || !other.dwObjectSerial || !other.smCharInfo.szName[0]) continue;
                const std::string name(other.smCharInfo.szName);
                if(std::find(names.begin(),names.end(),name)==names.end()) names.push_back(name);
            }
            return names;
        });
#ifdef GAME_GM
    settingsWindow->SetOnSpawnEditor([]() {
        if(GameMode!=2 || !lpCurPlayer || smConfig.getSecurity()<AccountTypes::SEC_MODERATOR ||
           !gmSpawnEditorWindow || !runtime.IsInitialized()) return;
        gmSpawnEditorWindow->ResetStatus();
        runtime.Windows().Open(GMSpawnEditorWindowId);
    });
#endif
    settingsWindow->SetOnClose([]()
    {
        if(settingsWindow)settingsWindow->ResetGmTransient();
        runtime.Windows().Close(SettingsWindowId);
    });
    runtime.Windows().Register(SettingsWindowId, std::move(window), UIWindowLayer::NormalWindow);
}

void BuildGuideWindow()
{
    auto window = std::make_unique<GuideWindow>(220);
    guideWindow = window.get();
    guideWindow->SetViewport(WinSizeX, WinSizeY);
    guideWindow->SetOnClose([]() { runtime.Windows().Close(GuideWindowId); });
    runtime.Windows().Register(GuideWindowId, std::move(window), UIWindowLayer::NormalWindow);
}

void BuildSocialWindow()
{
    auto window = std::make_unique<SocialWindow>(230);
    socialWindow = window.get();
    socialWindow->SetViewport(WinSizeX, WinSizeY);
    socialWindow->SetOnClose([]() { runtime.Windows().Close(SocialWindowId); });
    runtime.Windows().Register(SocialWindowId, std::move(window), UIWindowLayer::NormalWindow);
}

void BuildPartyStatusHud()
{
    auto window = std::make_unique<PartyStatusHud>(231);
    partyStatusHud = window.get();
    partyStatusHud->SetViewport(WinSizeX, WinSizeY);
    runtime.Windows().Register(PartyStatusHudId, std::move(window), UIWindowLayer::Hud);
    runtime.Windows().Open(PartyStatusHudId);
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

    auto death = std::make_unique<DeathWindow>(186);
    deathWindow = death.get();
    runtime.Windows().Register(DeathWindowId, std::move(death), UIWindowLayer::Modal);

    auto npc = std::make_unique<LegacyNpcDialogWindow>(190);
    npcDialogWindow = npc.get();
    runtime.Windows().Register(NpcDialogWindowId, std::move(npc), UIWindowLayer::Popup);

    auto skillNpc = std::make_unique<LegacySkillNpcWindow>(200);
    skillNpcWindow = skillNpc.get();
    runtime.Windows().Register(SkillNpcWindowId, std::move(skillNpc), UIWindowLayer::Popup);
}

void SyncLegacyDialogUI()
{
    // First-class V2 death screen, backed by the existing restart state machine.
    if (deathWindow)
    {
        if (ReStartFlag && !runtime.Windows().IsOpen(DeathWindowId))
            runtime.Windows().Open(DeathWindowId);
        else if (!ReStartFlag && runtime.Windows().IsOpen(DeathWindowId))
            runtime.Windows().Close(DeathWindowId);
    }
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
    const bool wantsCrafting = wantsCovenants && characterWindow->ActiveTab() == CharacterTab::Crafting; // SUMMIT392_B_WANTS
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
    else if (wantsCrafting)
        desired = CharacterTab::Crafting; // SUMMIT392_B_DESIRED
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
    BuildMixGuideUI();
    BuildProfessionTest387UI(); // SUMMIT_387_INIT
    BuildProfessionCraft378UI();
    BuildSettingsWindow();
#ifdef GAME_GM
    BuildGMWarpListWindow();
    BuildGMSpawnEditorWindow();
#endif
    BuildGuideWindow();
    BuildSocialWindow();
    BuildPartyStatusHud();
    BuildLegacyDialogUI();
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
    mixGuideWindow = nullptr;
    professionTestPanel387 = nullptr; // SUMMIT_387_SHUTDOWN
    professionCraftWindow378=nullptr;
    legacyMessageWindow = nullptr;
    legacyNotificationHud = nullptr;
    deathWindow = nullptr;
    npcDialogWindow = nullptr;
    skillNpcWindow = nullptr;
    settingsWindow = nullptr;
#ifdef GAME_GM
    gmWarpListWindow = nullptr;
    gmSpawnEditorWindow = nullptr;
#endif
    guideWindow = nullptr;
    socialWindow = nullptr;
    partyStatusHud = nullptr;
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

    // Drain server-authored receipts on the UI thread before drawing.
    std::deque<std::string> craftLive;
    {
        std::lock_guard<std::mutex> lock(craftMutex407);
        craftLive.swap(craftMessages407);
    }
    if(characterWindow)for(const std::string& frame:craftLive)
        characterWindow->AcceptCraftingLive407(frame);
    SyncCharacterWindow();
    SyncQuestJournal();
    SyncLegacyDialogUI();
    if (playerHud)
        playerHud->SetLegacyControlState(cInterFace.sInterFlags.RunFlag != 0,
                                         cInterFace.sInterFlags.CameraAutoFlag,
                                         cInterFace.sInterFlags.MapOnFlag != 0);
    if (cMixGuide::GetInstance()->IsOpen())
    {
        cMixGuide::GetInstance()->Close();
        if (mixGuideWindow)
        {
            mixGuideWindow->Reload();
            runtime.Windows().Open(MixGuideWindowId);
        }
    }

    const DWORD now = GetTickCount();
    covenantGateway.Tick(now);
    const DWORD elapsed = lastTick ? (now - lastTick) : 0;
    lastTick = now;
    const float deltaSeconds = static_cast<float>(elapsed) / 1000.0f;
    // SUMMIT350V: transaction/save-ACK pump must run on EVERY client UI tick.
    // Hidden tabs and closed CharacterWindow do not receive Container::Update.
    // Keep this on the main client thread, outside the network packet callback.
    companionGateway.PumpTradeExecution();
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

// SUMMIT462A: use the existing polled legacy key events while a Crafting
// lookup dialog is actually open; do not dispatch game/chat shortcuts then.
static bool CaptureCraftLookup462(){
#if UI_V2_CHARACTER_WINDOW
    if(!runtime.IsInitialized() || !characterWindow ||
       !runtime.Windows().IsOpen(CharacterWindowId) ||
       !characterWindow->HasCraftingSearch462())return false;
    static const int keys[]={VK_RETURN,VK_ESCAPE,VK_BACK,
        'A','B','C','D','E','F','G','H','I','J','K','L','M',
        'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
        '0','1','2','3','4','5','6','7','8','9'};
    for(int key:keys)if(sinGetKeyClick(key)){
        characterWindow->HandleCraftingSearchKey462(static_cast<std::uint32_t>(key));
        break;
    }
    return true;
#else
    return false;
#endif
}

bool HandleCharacterHotkeys()
{
#if UI_V2_CHARACTER_WINDOW
    if (!runtime.IsInitialized() || !characterWindow || RequiresLegacyInventoryUI())
        return false;
    if(CaptureCraftLookup462())return true;

    // SUMMIT_COMMAND_HOTKEYS_V17: simple player shortcuts do not leak through modified chords.
    if (VRKeyBuff[VK_CONTROL] || VRKeyBuff[VK_SHIFT] || VRKeyBuff[VK_MENU])
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

    if (VRKeyBuff[VK_CONTROL] || VRKeyBuff[VK_SHIFT] || VRKeyBuff[VK_MENU])
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
    if (VRKeyBuff[VK_CONTROL] || VRKeyBuff[VK_SHIFT] || VRKeyBuff[VK_MENU])
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
    // Death modal has no keyboard dismiss path; only explicit respawn buttons
    // and the original server-backed automatic timeout are allowed.
    if (ReStartFlag && deathWindow) return true;
    if(CaptureCraftLookup462())return true;
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
    if (VRKeyBuff[VK_CONTROL] && VRKeyBuff[VK_SHIFT] && !VRKeyBuff[VK_MENU] &&
        sinGetKeyClick('Y') && professionCraftWindow378) {
        if(runtime.Windows().IsOpen(ProfessionCraft378Id))runtime.Windows().Close(ProfessionCraft378Id);
        else runtime.Windows().Open(ProfessionCraft378Id);
        return true;
    }
    // SUMMIT_388A_WM_ONLY: WinMain handles this combination directly on WM_KEYDOWN.
    // Never toggle it again via sinGetKeyClick, which can be skipped/consumed.
    if (VRKeyBuff[VK_CONTROL] || VRKeyBuff[VK_SHIFT] || VRKeyBuff[VK_MENU]) return false;
    if (sinGetKeyClick('X')) { ExecuteHudShortcut(HudShortcutAction::System); return true; }
    if (sinGetKeyClick('R')) { ExecuteHudShortcut(HudShortcutAction::RunWalk); return true; }
    if (sinGetKeyClick(VK_TAB)) { ExecuteHudShortcut(HudShortcutAction::Map); return true; }
    if (sinGetKeyClick('Z')) { ExecuteHudShortcut(HudShortcutAction::Camera); return true; }
    if (sinGetKeyClick('D')) { ExecuteHudShortcut(HudShortcutAction::Party); return true; }
    if (sinGetKeyClick('Q')) { ExecuteHudShortcut(HudShortcutAction::Quests); return true; }
#endif
    return false;
}

// SUMMIT_388A_PANEL_DISPATCH: called at WM_KEYDOWN, independent from HUD shortcuts.
// The Windows input layer validates the Ctrl+Shift+T chord and ignores repeats.
bool HandleProfessionTestPanelKey(unsigned int key)
{
    if (!runtime.IsInitialized() || !professionTestPanel387) return false;
    if (key == VK_ESCAPE)
    {
        if (!runtime.Windows().IsOpen(ProfessionTest387Id)) return false;
        runtime.Windows().Close(ProfessionTest387Id);
        return true;
    }
    // 406A: legacy profession TEST window has no T opening path.
    // Crafting belongs to CharacterWindow; Escape can still close an
    // already-open test window safely.
    return false;
}

bool IsSettingsWindowOpen()
{
    return runtime.IsInitialized() && settingsWindow != nullptr && runtime.Windows().IsOpen(SettingsWindowId);
}

// SUMMIT_PATCH_214_GM_WARP_LIST_V2_TOGGLE
void ToggleGMWarpListWindow()
{
#ifdef GAME_GM
    if (!runtime.IsInitialized() || !gmWarpListWindow ||
        smConfig.getSecurity() < AccountTypes::SEC_MODERATOR)
        return;
    if (runtime.Windows().IsOpen(GMWarpListWindowId))
    {
        runtime.Windows().Close(GMWarpListWindowId);
        return;
    }
    gmWarpListWindow->Reload();
    runtime.Windows().Open(GMWarpListWindowId);
#endif
}

bool HandleGMEditorKey(unsigned int key)
{
    if(!runtime.IsInitialized() || !settingsWindow || !runtime.Windows().IsOpen(SettingsWindowId))return false;
    return settingsWindow->HandleGmKey(key);
}

bool HandleGMEditorChar(unsigned int character)
{
    if(!runtime.IsInitialized() || !settingsWindow || !runtime.Windows().IsOpen(SettingsWindowId))return false;
    return settingsWindow->HandleGmChar(character);
}

void ToggleSettingsWindow()
{
    if (!runtime.IsInitialized() || !settingsWindow)
        return;

    if (runtime.Windows().IsOpen(SettingsWindowId))
    {
        settingsWindow->ResetGmTransient();
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

bool OpenGuideWindow()
{
    if (!runtime.IsInitialized() || !guideWindow) return false;
    guideWindow->Reload();
    runtime.Windows().Open(GuideWindowId);
    return true;
}

bool OpenSocialWindow()
{
    if (!runtime.IsInitialized() || !socialWindow) return false;
    socialWindow->SetTab(SocialWindow::Tab::Party);
    socialWindow->Refresh();
    runtime.Windows().Open(SocialWindowId);
    return true;
}

bool OpenFriendsWindow()
{
    if (!runtime.IsInitialized() || !socialWindow) return false;
    socialWindow->SetTab(SocialWindow::Tab::Friends);
    socialWindow->Refresh();
    runtime.Windows().Open(SocialWindowId);
    return true;
}

bool OpenQuestJournalWindow()
{
#if UI_V2_QUEST_JOURNAL
    if (!runtime.IsInitialized() || !questJournal) return false;
    if (!questGateway.EnsureBackendOpen()) return false;
    runtime.Windows().Open(QuestJournalWindowId);
    return true;
#else
    return false;
#endif
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
    if (professionTestPanel387) professionTestPanel387->SetViewport(width, height); // SUMMIT_387_RESIZE
    if (mixGuideWindow)
        mixGuideWindow->SetViewport(width, height);
    if (legacyMessageWindow)
        legacyMessageWindow->SetBounds({0, 0, width, height});
    if (legacyNotificationHud)
        legacyNotificationHud->SetViewport(width, height);
    if (deathWindow)
        deathWindow->SetViewport(width, height);
    if (settingsWindow)
        settingsWindow->SetViewport(width, height);
#ifdef GAME_GM
    if (gmWarpListWindow)
        gmWarpListWindow->SetViewport(width, height);
#endif
    if (guideWindow)
        guideWindow->SetViewport(width, height);
    if (socialWindow)
        socialWindow->SetViewport(width, height);
    if (partyStatusHud)
        partyStatusHud->SetViewport(width, height);
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
    if (professionTestPanel387 && runtime.Windows().IsOpen(ProfessionTest387Id) &&
        professionTestPanel387->GlobalBounds().Contains(Point{x, y})) return true; // SUMMIT_387_BLOCK_WORLD
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

bool OwnsLegacyDeathUI()
{
    return runtime.IsInitialized() && deathWindow != nullptr;
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
