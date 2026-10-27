#include "systems/ui/v2/windows/SettingsWindow.h"
#include "systems/ui/v2/windows/GmChoiceCatalog.h"

// SUMMIT_COMMAND_HOTKEYS_V17
// SUMMIT_COMMAND_HOTKEYS_SAFETY_V17_1

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>
#include <cstdlib>

#include "systems/configuration/client/Settings.h"
#include "content/v1/localization/content/LocalizationRuntime.h"
#include "systems/gameplay/loot/filter/CLootHandler.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/Button.h"
#include "systems/ui/v2/widgets/Label.h"
#include "systems/ui/v2/core/Container.h"

// Dxwav.cpp exports SetVolumeBGM(DWORD). DWORD is an unsigned long on the
// Win32 target; using int here changes the C++ mangled symbol and therefore
// compiles but fails at link time.
extern void SetVolumeBGM(unsigned long Volume);
extern void PlayWav_UpdateVolume();

namespace ui::v2
{
namespace
{
constexpr int kWindowWidth = 760;
constexpr int kWindowHeight = 540;
constexpr int kPageTop = 78;
constexpr int kFooterTop = 494;
constexpr int kRowHeight = 34;
constexpr int kButtonWidth = 225;

void SetButton(Button* button, const std::string& label, bool selected = false)
{
    if (!button)
        return;
    button->SetText(label);
    button->SetSelected(selected);
}

std::string ResolutionText(int ratio, int resolution)
{
    static const char* k43[] = {"800x600", "1024x768", "1280x960", "1400x1050"};
    static const char* k169[] = {"1280x720", "1366x768", "1600x900", "1920x1080"};
    const int index = std::clamp(resolution, 1, 4) - 1;
    return ratio == 43 ? k43[index] : k169[index];
}
}

SettingsWindow::SettingsWindow(UIId id)
    : Window(id, "Configuracoes")
{
    SetBounds({24, 24, kWindowWidth, kWindowHeight});
    SetTitleBarHeight(34);
    SetMovable(true);
    SetClosable(true);

    BuildChrome();
    BuildVideoPage();
    BuildAudioPage();
    BuildGeneralPage();
    BuildLootPage();
    BuildCommandsPage();
    BuildGMPage();
    Layout();
    ReloadFromRuntime();
    SetActiveTab(SettingsTab::Video);
}

void SettingsWindow::BuildChrome()
{
    auto& video = AddChild<Button>(700, "Video");
    video.SetOnClick([this](const UIEvent&) { SetActiveTab(SettingsTab::Video); });
    videoTab_ = &video;

    auto& audio = AddChild<Button>(701, "Audio");
    audio.SetOnClick([this](const UIEvent&) { SetActiveTab(SettingsTab::Audio); });
    audioTab_ = &audio;

    auto& general = AddChild<Button>(702, "Geral");
    general.SetOnClick([this](const UIEvent&) { SetActiveTab(SettingsTab::General); });
    generalTab_ = &general;

    auto& loot = AddChild<Button>(703, "Loot");
    loot.SetOnClick([this](const UIEvent&) { SetActiveTab(SettingsTab::Loot); });
    lootTab_ = &loot;

    auto& commands = AddChild<Button>(707, "Comandos");
    commands.SetOnClick([this](const UIEvent&) { SetActiveTab(SettingsTab::Commands); });
    commandsTab_ = &commands;

    auto& gmTab = AddChild<Button>(708, "GM");
    gmTab.SetOnClick([this](const UIEvent&) { SetActiveTab(SettingsTab::GM); RefreshGMPage(); });
    gmTab_ = &gmTab;

    auto& close = AddChild<Button>(704, "X");
    close.SetOnClick([this](const UIEvent&) { if (onClose_) onClose_(); });
    closeButton_ = &close;

    auto& reset = AddChild<Button>(705, "Padrao");
    reset.SetOnClick([this](const UIEvent&) { ResetDefaults(); });
    resetButton_ = &reset;

    auto& save = AddChild<Button>(706, "Salvar");
    save.SetOnClick([this](const UIEvent&) { Save(); if (onClose_) onClose_(); });
    saveButton_ = &save;
}

void SettingsWindow::BuildVideoPage()
{
    auto& page = AddChild<Container>(720);
    videoPage_ = &page;

    const char* names[8] = {
        "Modo de tela", "Auto ajuste", "Iniciar maximizado", "Aspecto",
        "Resolucao", "Qualidade textura", "Profundidade", "Camera"
    };
    for (int i = 0; i < 8; ++i)
    {
        auto& label = page.AddChild<Label>(721 + i * 2, names[i]);
        label.SetBounds({18, 18 + i * kRowHeight, 250, 28});
        label.SetStyle(TextStyle{UITheme::TextMuted, false, TextAlign::Left});

        auto& button = page.AddChild<Button>(722 + i * 2, "");
        button.SetBounds({286, 14 + i * kRowHeight, kButtonWidth, 28});
        videoButtons_[static_cast<std::size_t>(i)] = &button;
    }

    videoButtons_[0]->SetOnClick([this](const UIEvent&) { state_.windowed = !state_.windowed; RefreshLabels(); });
    videoButtons_[1]->SetOnClick([this](const UIEvent&) { state_.autoAdjust = !state_.autoAdjust; RefreshLabels(); });
    videoButtons_[2]->SetOnClick([this](const UIEvent&) { state_.startMaximized = !state_.startMaximized; RefreshLabels(); });
    videoButtons_[3]->SetOnClick([this](const UIEvent&) { state_.ratio = state_.ratio == 43 ? 169 : 43; state_.resolution = 1; RefreshLabels(); });
    videoButtons_[4]->SetOnClick([this](const UIEvent&) { state_.resolution = state_.resolution % 4 + 1; RefreshLabels(); });
    videoButtons_[5]->SetOnClick([this](const UIEvent&) { state_.textureQuality = state_.textureQuality == 1 ? 2 : 1; RefreshLabels(); });
    videoButtons_[6]->SetOnClick([this](const UIEvent&) { state_.bitDepth = state_.bitDepth == 32 ? 16 : 32; RefreshLabels(); });
    videoButtons_[7]->SetOnClick([this](const UIEvent&)
    {
        // Keep both camera flags on one compact control: normal -> long range -> inverted -> normal.
        if (!state_.cameraSight && !state_.cameraInverted)
            state_.cameraSight = true;
        else if (state_.cameraSight && !state_.cameraInverted)
            state_.cameraInverted = true;
        else
        {
            state_.cameraSight = false;
            state_.cameraInverted = false;
        }
        RefreshLabels();
    });
}

void SettingsWindow::BuildAudioPage()
{
    auto& page = AddChild<Container>(740);
    audioPage_ = &page;

    const char* names[2] = {"Musica", "Efeitos"};
    for (int i = 0; i < 2; ++i)
    {
        auto& label = page.AddChild<Label>(741 + i * 8, names[i]);
        label.SetBounds({18, 22 + i * 106, 200, 28});
        label.SetStyle(TextStyle{UITheme::Text, true, TextAlign::Left});

        auto& toggle = page.AddChild<Button>(742 + i * 8, "");
        toggle.SetBounds({220, 18 + i * 106, 160, 30});
        audioButtons_[static_cast<std::size_t>(i * 3)] = &toggle;

        auto& minus = page.AddChild<Button>(743 + i * 8, "-10");
        minus.SetBounds({220, 58 + i * 106, 76, 30});
        audioButtons_[static_cast<std::size_t>(i * 3 + 1)] = &minus;

        auto& plus = page.AddChild<Button>(744 + i * 8, "+10");
        plus.SetBounds({304, 58 + i * 106, 76, 30});
        audioButtons_[static_cast<std::size_t>(i * 3 + 2)] = &plus;
    }

    audioButtons_[0]->SetOnClick([this](const UIEvent&) { state_.music = !state_.music; RefreshLabels(); });
    audioButtons_[1]->SetOnClick([this](const UIEvent&) { state_.musicVolume = std::max(0, state_.musicVolume - 10); RefreshLabels(); });
    audioButtons_[2]->SetOnClick([this](const UIEvent&) { state_.musicVolume = std::min(100, state_.musicVolume + 10); RefreshLabels(); });
    audioButtons_[3]->SetOnClick([this](const UIEvent&) { state_.effects = !state_.effects; RefreshLabels(); });
    audioButtons_[4]->SetOnClick([this](const UIEvent&) { state_.effectsVolume = std::max(0, state_.effectsVolume - 10); RefreshLabels(); });
    audioButtons_[5]->SetOnClick([this](const UIEvent&) { state_.effectsVolume = std::min(100, state_.effectsVolume + 10); RefreshLabels(); });
}

void SettingsWindow::BuildGeneralPage()
{
    auto& page = AddChild<Container>(770);
    generalPage_ = &page;

    const char* names[9] = {
        "Clima", "Lembrar ID", "Pedidos de grupo", "Pedidos de troca",
        "Mensagens privadas", "Exibir dano", "Barra de HP", "Mostrar FPS", "Idioma"
    };
    for (int i = 0; i < 9; ++i)
    {
        const int col = i >= 5 ? 1 : 0;
        const int row = i >= 5 ? i - 5 : i;
        const int x = 18 + col * 350;
        auto& label = page.AddChild<Label>(771 + i * 2, names[i]);
        label.SetBounds({x, 22 + row * 70, 180, 24});
        label.SetStyle(TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        auto& button = page.AddChild<Button>(772 + i * 2, "");
        button.SetBounds({x, 48 + row * 70, 220, 30});
        generalButtons_[static_cast<std::size_t>(i)] = &button;
    }

    generalButtons_[0]->SetOnClick([this](const UIEvent&) { state_.weather = !state_.weather; RefreshLabels(); });
    generalButtons_[1]->SetOnClick([this](const UIEvent&) { state_.rememberId = !state_.rememberId; RefreshLabels(); });
    generalButtons_[2]->SetOnClick([this](const UIEvent&) { state_.partyRequest = !state_.partyRequest; RefreshLabels(); });
    generalButtons_[3]->SetOnClick([this](const UIEvent&) { state_.tradeRequest = !state_.tradeRequest; RefreshLabels(); });
    generalButtons_[4]->SetOnClick([this](const UIEvent&) { state_.whisper = !state_.whisper; RefreshLabels(); });
    generalButtons_[5]->SetOnClick([this](const UIEvent&) { state_.damageDebug = !state_.damageDebug; RefreshLabels(); });
    generalButtons_[6]->SetOnClick([this](const UIEvent&) { state_.hpBar = !state_.hpBar; RefreshLabels(); });
    generalButtons_[7]->SetOnClick([this](const UIEvent&) { state_.fps = !state_.fps; RefreshLabels(); });
    generalButtons_[8]->SetOnClick([this](const UIEvent&) { state_.locale = state_.locale == "pt-BR" ? "en" : "pt-BR"; RefreshLabels(); });
}

void SettingsWindow::BuildLootPage()
{
    auto& page = AddChild<Container>(800);
    lootPage_ = &page;

    static const char* names[23] = {
        "Filtro de loot", "Pocao HP", "Pocao MP", "Pocao RES", "Gold", "Amuletos",
        "Aneis", "Sheltoms", "Force Orbs", "Premiums", "Cristais", "Defensivos",
        "Ofensivos", "Todo o resto", "Sem Spec", "Fighter", "Mechanician", "Archer",
        "Pikeman", "Atalanta", "Knight", "Magician", "Priestess"
    };

    for (int i = 0; i < 23; ++i)
    {
        const int col = i >= 12 ? 1 : 0;
        const int row = i % 12;
        const int x = 16 + col * 350;
        auto& button = page.AddChild<Button>(801 + i, names[i]);
        button.SetBounds({x, 12 + row * 32, 324, 28});
        lootButtons_[static_cast<std::size_t>(i)] = &button;
    }

    for (int i = 0; i < 23; ++i)
    {
        lootButtons_[static_cast<std::size_t>(i)]->SetOnClick([this, i](const UIEvent&)
        {
            if (i == 0) state_.lootEnabled = !state_.lootEnabled;
            else state_.loot[static_cast<std::size_t>(i - 1)] = !state_.loot[static_cast<std::size_t>(i - 1)];
            RefreshLabels();
        });
    }
}


void SettingsWindow::BuildCommandsPage()
{
    auto& page = AddChild<Container>(840);
    commandsPage_ = &page;

    static const char* groups[4] = {"Jogo", "Chat / UI", "GM / Debug", "Testes"};
    for (int i = 0; i < 4; ++i)
    {
        auto& button = page.AddChild<Button>(841 + i, groups[i]);
        button.SetBounds({16 + i * 168, 10, 156, 28});
        button.SetOnClick([this, i](const UIEvent&) { SetCommandGroup(i); });
        commandGroupButtons_[static_cast<std::size_t>(i)] = &button;
    }

    auto& note = page.AddChild<Label>(849, "Comandos perigosos de render/pattern/edicao persistente foram removidos do teclado.");
    note.SetBounds({18, 42, 690, 22});
    note.SetStyle(TextStyle{UITheme::TextMuted, false, TextAlign::Left});

    for (int i = 0; i < 24; ++i)
    {
        const int col = i >= 12 ? 1 : 0;
        const int row = i % 12;
        auto& label = page.AddChild<Label>(850 + i, "");
        label.SetBounds({18 + col * 360, 68 + row * 27, 344, 24});
        label.SetStyle(TextStyle{UITheme::Text, false, TextAlign::Left});
        commandLabels_[static_cast<std::size_t>(i)] = &label;
    }

    SetCommandGroup(0);
}

void SettingsWindow::SetCommandGroup(int group)
{
    commandGroup_ = std::clamp(group, 0, 3);
    for (int i = 0; i < 4; ++i)
        commandGroupButtons_[static_cast<std::size_t>(i)]->SetSelected(i == commandGroup_);
    RefreshCommandPage();
}

void SettingsWindow::RefreshCommandPage()
{
    static const std::array<const char*, 24> kGame = {{
        "C  - Atributos", "S  - Skills", "V  - Inventario", "N  - Covenants",
        "U  - Diario de quests", "Q  - Quests", "D  - Party", "X  - Configuracoes",
        "R  - Correr / caminhar", "Tab  - Mini-mapa", "M  - Mapa mundial", "Z  - Modo da camera",
        "K  - War Mode", "T  - Mostrar/ocultar traje", "L  - Travar inventario", "E  - Equipamento A/B",
        "W  - Costume set 1/2", "1/2/3  - Usar pocao", "Shift+1/2/3  - Atribuir pocao", "F1-F8  - Skills rapidas",
        "F10  - TimeShop", "F11  - Configuracoes", "F12  - Janela/tela cheia", "Space  - Fechar interfaces"
    }};
    static const std::array<const char*, 24> kChat = {{
        "Ctrl+F9  - CoinShop", "Ctrl+1..0  - Macros de chat 1..10", "Enter  - Abrir/enviar chat", "Esc  - Fechar/voltar",
        "Shift+Esc  - Sair do jogo", "Tab (chat)  - Chat de clan", "Up/Down (chat)  - Historico", "PgUp/PgDn (chat)  - Scroll",
        "Left/Right  - Rodar camera", "Up/Down  - Zoom da camera", "Ctrl+Up/Down  - Inclinar camera", "PgUp/PgDn  - Inclinar camera",
        "Ctrl+Home  - Screenshot", "Ctrl+End  - Screenshot", "Numpad +/-  - Field-view step", "M (mapa)  - Fechar mapa",
        "Ctrl+clique item  - Link no chat", "Space (Yahoo)  - Emote/evento", "", "",
        "", "", "", ""
    }};
    // SUMMIT_V17_2_DANGEROUS_COMMANDS_REMOVED_FROM_MENU
    // Removed from the visible command list and keyboard dispatch: Shift+F3/F4/F7,
    // Shift+Home/End, persistent Insert/Delete editing, Numpad7/8 and effect tests.
    static const std::array<const char*, 24> kGm = {{
        "Shift+F9  - Debug/coordenadas", "Shift+F5  - Skip field 1", "Shift+F6  - Skip field 2", "Shift+F8  - Draw info",
        "Ctrl+F11  - Interface on/off", "Ctrl+T  - Test flag", "Ctrl+F  - Shop", "Ctrl+G  - Craft",
        "Ctrl+H  - Aging", "Ctrl+J  - Mixture Reset", "Ctrl+Shift+F1  - Talk zoom ON", "Ctrl+Shift+F2  - Talk zoom OFF",
        "Ctrl+Shift+F3  - Talk repeat ON", "Ctrl+Shift+F4  - Talk repeat OFF", "Ctrl+Shift+F9  - Proximo ceu", "Ctrl+Shift+F10  - Ceu normal",
        "", "", "", "", "", "", "", ""
    }};
    static const std::array<const char*, 24> kTest = {{
        "Ctrl+Shift+F5  - Facial repeat ON", "Ctrl+Shift+F6  - Facial repeat OFF", "Shift+N  - VIP no-collision", "O + ataque  - Berserker test [PERIGOSO]",
        "Ctrl+N+M  - Park player [TEST SERVER]", "Ctrl+N+B  - Durabilidade [TEST SERVER]", "P+O  - Potion qty [PERIGOSO]", "Ctrl+Shift+B (status)  - +9 pontos [GM]",
        "", "", "", "", "", "", "", "",
        "", "", "", "", "", "", "", ""
    }};

    const std::array<const char*, 24>* list = &kGame;
    if (commandGroup_ == 1) list = &kChat;
    else if (commandGroup_ == 2) list = &kGm;
    else if (commandGroup_ == 3) list = &kTest;

    for (std::size_t i = 0; i < commandLabels_.size(); ++i)
        commandLabels_[i]->SetText((*list)[i]);
}

// SUMMIT 355 — clicavel, mas sem substituir autorizacao do servidor.
void SettingsWindow::SetGmBridge(SecurityProvider security, GmDispatch dispatch, NameProvider names)
{
    gmSecurity_ = std::move(security);
    gmDispatch_ = std::move(dispatch);
    gmNames_ = std::move(names);
}

void SettingsWindow::BuildGMPage()
{
    auto& page = AddChild<Container>(880);
    gmPage_ = &page;
    static const char* sections[] = {"Acesso / Testes", "Moderator 2+", "Game Master 3+", "Admin 4+"};
    for (int i=0;i<4;++i) {
        auto& b=page.AddChild<Button>(881+i,sections[i]);
        b.SetBounds({12+i*169,9,160,28});
        b.SetOnClick([this,i](const UIEvent&){SelectGMGroup(i);});
        gmGroups_[static_cast<size_t>(i)]=&b;
    }
    for(int i=0;i<10;++i) {
        auto& b=page.AddChild<Button>(890+i,"");
        b.SetBounds({12,45+i*30,258,27});
        b.SetOnClick([this,i](const UIEvent&){ChooseGMListEntry(i);});
        gmCommands_[static_cast<size_t>(i)]=&b;
    }
    auto& previous=page.AddChild<Button>(902,"< Anterior");
    previous.SetBounds({12,349,120,27});
    previous.SetOnClick([this](const UIEvent&){if(gmPickerField_>=0){if(gmPickerPage_>0)--gmPickerPage_;else if(gmPickerStage_>0){gmPickerStage_=0;gmPickerPrefix_.clear();}}else if(gmPageIndex_>0){--gmPageIndex_;SelectGMCommand(-1);}RefreshGMPage();});
    gmPrevious_=&previous;
    auto& next=page.AddChild<Button>(903,"Seguinte >");
    next.SetBounds({150,349,120,27});
    next.SetOnClick([this](const UIEvent&){if(gmPickerField_>=0)++gmPickerPage_;else{++gmPageIndex_;SelectGMCommand(-1);}RefreshGMPage();});
    gmNext_=&next;
    auto& title=page.AddChild<Label>(904,""); title.SetBounds({280,43,405,26});
    title.SetStyle(TextStyle{UITheme::Text,true,TextAlign::Left}); gmTitle_=&title;
    auto& hint=page.AddChild<Label>(905,""); hint.SetBounds({280,72,405,26});
    hint.SetStyle(TextStyle{UITheme::TextMuted,false,TextAlign::Left});gmHint_=&hint;
    auto& permission=page.AddChild<Label>(906,"");permission.SetBounds({280,98,405,26});
    permission.SetStyle(TextStyle{UITheme::TextMuted,false,TextAlign::Left});gmPermission_=&permission;
    for(int i=0;i<3;++i){
        const int y=148+i*60;
        auto& label=page.AddChild<Label>(910+i,"");label.SetBounds({280,y-19,350,18});
        label.SetStyle(TextStyle{UITheme::TextMuted,false,TextAlign::Left});
        gmFieldLabels_[static_cast<size_t>(i)]=&label;
        auto& b=page.AddChild<Button>(914+i,"");b.SetBounds({280,y,290,29});
        b.SetOnClick([this,i](const UIEvent&){CycleGMField(i);});
        gmFields_[static_cast<size_t>(i)]=&b;
        auto& edit=page.AddChild<Button>(918+i,"Editar");edit.SetBounds({575,y,111,29});
        edit.SetOnClick([this,i](const UIEvent&){EditGMField(i);});
        gmEditButtons_[static_cast<size_t>(i)]=&edit;
    }
    auto& preview=page.AddChild<Label>(922,"");preview.SetBounds({280,327,405,19});
    preview.SetStyle(TextStyle{UITheme::TextMuted,false,TextAlign::Left});gmPreview_=&preview;
    auto& execute=page.AddChild<Button>(923,"Executar");execute.SetBounds({280,350,406,29});
    execute.SetOnClick([this](const UIEvent&){ExecuteGM();});gmExecute_=&execute;
    auto& feedback=page.AddChild<Label>(924,"");feedback.SetBounds({12,382,678,19});
    feedback.SetStyle(TextStyle{UITheme::TextMuted,false,TextAlign::Left});gmFeedback_=&feedback;
    SelectGMGroup(0);
}

std::vector<std::string> SettingsWindow::GMOptions(gm::Arg arg) const
{
    using gm::Arg;
    std::vector<std::string> options;
    if (gm::Optional(arg)) options.emplace_back("(omitir)");
    if(arg==Arg::Player || arg==Arg::OptionalPlayer) {
        if(gmNames_) {
            for(const std::string& n:gmNames_())
                if(!n.empty() && std::find(options.begin(),options.end(),n)==options.end()) options.push_back(n);
        }
    }
    if(arg==Arg::Item) {const auto& v=gm::ItemSuggestions();options.insert(options.end(),v.begin(),v.end());return options;}
    if(arg==Arg::Monster) {const auto& v=gm::MonsterSuggestions();options.insert(options.end(),v.begin(),v.end());return options;}
    const char* values="";
    switch(arg) {
        case Arg::Level: values="1,10,20,40,60,80,90,100,110,120,130,140,150";break;
        case Arg::Days: values="-30,-7,-1,0,1,7,15,30";break;
        case Arg::Minutes: case Arg::OptionalMinutes: values="1,5,10,15,30,60,120,180";break;
        case Arg::Seconds: values="0,1,3,5,10,30,60";break;
        case Arg::Amount: values="1,10,100,1000,10000,100000,1000000";break;
        case Arg::Quantity: case Arg::OptionalQuantity: values="1,2,3,5,8,10,12";break;
        case Arg::OptionalMonsterCount: values="0,1,2,5,10,15,20";break;
        case Arg::PotionCount: values="2,10,20,50,100,200,300";break;
        case Arg::BossMinute: values="0,5,10,15,20,30,45,59";break;
        case Arg::Permil: values="0,10,50,100,250,500,750,1000";break;
        case Arg::Job: values="1,2,3,4,5,6,7,8";break;
        case Arg::Number: values="0,1,2,3,5,10,20,30,50,100";break;
        case Arg::OptionalMode: values="0,1,2";break;
        case Arg::Id: case Arg::OptionalId: case Arg::Code: case Arg::OptionalCode: values="0,1,2,3,10,100";break;
        default: break;
    }
    std::string s=values;
    size_t begin=0;
    while(begin<s.size()) {
        const size_t end=s.find(',',begin);
        options.emplace_back(s.substr(begin,end==std::string::npos?std::string::npos:end-begin));
        if(end==std::string::npos)break;
        begin=end+1;
    }
    return options;
}

void SettingsWindow::SelectGMGroup(int group)
{
    gmGroup_=std::clamp(group,0,3);
    gmPageIndex_=0;
    gmPickerField_=-1;
    gmStatus_.clear();
    SelectGMCommand(-1);
    RefreshGMPage();
}

void SettingsWindow::SelectGMCommand(int index)
{
    gmSelected_=index;
    gmEditing_=-1;
    gmConfirmArmed_=false;
    gmValues_={};
    gmTyped_={};
    gmPickerField_=-1;
    gmPickerPage_=0;
    gmPickerStage_=0;
    gmPickerPrefix_.clear();
    gmStatus_.clear();
    if(index>=0){
        const auto& cmd=gm::Catalog()[static_cast<size_t>(index)];
        for(int i=0;i<3;++i){
            const auto v=GMOptions(cmd.args[static_cast<size_t>(i)]);
            // Nenhuma acao perigosa arranca com valores arbitrarios preselecionados.
            if(gm::Optional(cmd.args[static_cast<size_t>(i)]))gmValues_[static_cast<size_t>(i)]="(omitir)";
        }
    }
    RefreshGMPage();
}

std::vector<std::string> SettingsWindow::GMPickerOptions() const
{
    if(gmPickerField_<0 || gmSelected_<0)return {};
    const auto& cmd=gm::Catalog()[static_cast<size_t>(gmSelected_)];
    const auto kind=cmd.args[static_cast<size_t>(gmPickerField_)];
    std::vector<std::string> values=GMOptions(kind);
    const bool category=(kind==gm::Arg::Item || kind==gm::Arg::Monster);
    if(gmPickerStage_==0 && category) {
        std::vector<std::string> prefixes;
        const size_t chars=kind==gm::Arg::Item?2:1;
        for(const auto& v:values) {
            if(v.empty())continue;
            const std::string prefix=v.substr(0,chars);
            if(std::find(prefixes.begin(),prefixes.end(),prefix)==prefixes.end())prefixes.push_back(prefix);
        }
        return prefixes;
    }
    if(gmPickerStage_>0 && !gmPickerPrefix_.empty()) {
        values.erase(std::remove_if(values.begin(),values.end(),[this](const std::string& v){
            return v.compare(0,gmPickerPrefix_.size(),gmPickerPrefix_)!=0;
        }),values.end());
    }
    if(gmTyped_[static_cast<size_t>(gmPickerField_)] && !gmValues_[static_cast<size_t>(gmPickerField_)].empty()) {
        const std::string query=gmValues_[static_cast<size_t>(gmPickerField_)];
        values.erase(std::remove_if(values.begin(),values.end(),[&query](const std::string& v){
            return v.find(query)==std::string::npos;
        }),values.end());
    }
    return values;
}

void SettingsWindow::ChooseGMListEntry(int index)
{
    if(index<0 || index>=10)return;
    if(gmPickerField_<0){
        int position=0;
        for(size_t j=0;j<gm::Catalog().size();++j){
            if(gm::Catalog()[j].group!=gmGroup_)continue;
            if(position++==gmPageIndex_*10+index){SelectGMCommand(static_cast<int>(j));return;}
        }
        return;
    }
    const auto options=GMPickerOptions();
    const int offset=gmPickerPage_*10+index;
    if(offset>=static_cast<int>(options.size()))return;
    const auto& cmd=gm::Catalog()[static_cast<size_t>(gmSelected_)];
    const auto kind=cmd.args[static_cast<size_t>(gmPickerField_)];
    if(gmPickerStage_==0 && (kind==gm::Arg::Item || kind==gm::Arg::Monster)) {
        gmPickerPrefix_=options[static_cast<size_t>(offset)];
        gmPickerStage_=1;
        gmPickerPage_=0;
        RefreshGMPage();
        return;
    }
    gmValues_[static_cast<size_t>(gmPickerField_)]=options[static_cast<size_t>(offset)];
    gmTyped_[static_cast<size_t>(gmPickerField_)]=false;
    gmPickerField_=-1;
    gmPickerPage_=0;
    gmPickerPrefix_.clear();
    gmPickerStage_=0;
    gmConfirmArmed_=false;
    gmStatus_="Opcao selecionada; podes executar ou alterar parametros";
    RefreshGMPage();
}

void SettingsWindow::CycleGMField(int index)
{
    if(gmSelected_<0 || index<0 || index>=3)return;
    const auto& cmd=gm::Catalog()[static_cast<size_t>(gmSelected_)];
    if(cmd.args[static_cast<size_t>(index)]==gm::Arg::Secret ||
       cmd.args[static_cast<size_t>(index)]==gm::Arg::Message ||
       cmd.args[static_cast<size_t>(index)]==gm::Arg::Account ||
       cmd.args[static_cast<size_t>(index)]==gm::Arg::OptionalText) {
        EditGMField(index);return;
    }
    gmPickerField_=index;
    gmPickerPage_=0;
    gmPickerStage_=0;
    gmPickerPrefix_.clear();
    gmEditing_=-1;
    gmStatus_="Escolhe uma opcao na lista a esquerda (ou clica Digitar)";
    gmConfirmArmed_=false;
    RefreshGMPage();
}

void SettingsWindow::EditGMField(int index)
{
    if(gmSelected_<0 || index<0 || index>=3)return;
    const auto& cmd=gm::Catalog()[static_cast<size_t>(gmSelected_)];
    if(cmd.args[static_cast<size_t>(index)]==gm::Arg::None)return;
    if((gmSecurity_?gmSecurity_():0)<cmd.level)return;
    gmEditing_=index;
    gmPickerField_=-1;
    gmValues_[static_cast<size_t>(index)].clear();
    gmTyped_[static_cast<size_t>(index)]=true;
    gmStatus_="A escrever: Enter confirma, Esc cancela, Backspace apaga";
    gmConfirmArmed_=false;RefreshGMPage();
}

void SettingsWindow::ResetGmTransient()
{
    gmEditing_=-1;
    gmPickerField_=-1;
    gmConfirmArmed_=false;
    gmStatus_.clear();
    if(gmSelected_>=0 && gm::Catalog()[static_cast<size_t>(gmSelected_)].args[0]==gm::Arg::Secret)
        gmValues_[0].clear();
}

bool SettingsWindow::HandleGmKey(std::uint32_t key)
{
    if(!EditingGmField())return false;
    if(key==13 || key==27) {
        if(key==27)gmValues_[static_cast<size_t>(gmEditing_)].clear();
        gmEditing_=-1;gmStatus_=key==13?"Campo atualizado":"Edicao cancelada";
        RefreshGMPage();return true;
    }
    if(key==8) {auto& value=gmValues_[static_cast<size_t>(gmEditing_)];if(!value.empty())value.pop_back();RefreshGMPage();return true;}
    return true; // consumir qualquer tecla enquanto se edita (sem atalhos do mundo)
}

bool SettingsWindow::HandleGmChar(std::uint32_t ch)
{
    if(!EditingGmField())return false;
    if(ch==8 || ch==13 || ch==27)return true;
    if(ch>=32 && ch<127) {
        auto& value=gmValues_[static_cast<size_t>(gmEditing_)];
        if(value.size()<74)value.push_back(static_cast<char>(ch));
        RefreshGMPage();
    }
    return true;
}

void SettingsWindow::ExecuteGM()
{
    if(gmSelected_<0 || !gmDispatch_)return;
    const auto& cmd=gm::Catalog()[static_cast<size_t>(gmSelected_)];
    const int level=gmSecurity_?gmSecurity_():0;
    if(level<cmd.level) {gmStatus_="Sem autorizacao GM suficiente";RefreshGMPage();return;}
    if(gmEditing_>=0){gmStatus_="Confirma primeiro o campo com Enter";RefreshGMPage();return;}
    std::string message;
    if(!gm::Build(cmd,gmValues_,message)) {gmStatus_="Seleciona ou edita todos os parametros obrigatorios";gmConfirmArmed_=false;RefreshGMPage();return;}
    if(cmd.confirm && !gmConfirmArmed_) {gmConfirmArmed_=true;gmStatus_="Confirmar: clica EXECUTAR novamente";RefreshGMPage();return;}
    // Evitar conservar credenciais em estado de UI, mesmo se a rede recusar envio.
    gmConfirmArmed_=false;
    const bool ok=gmDispatch_(message,cmd.local);
    if(cmd.args[0]==gm::Arg::Secret) gmValues_[0].clear();
    gmStatus_=ok ? "Comando entregue ao fluxo do jogo; aguarda resposta" : "Comando nao executado / envio recusado";
    RefreshGMPage();
}

void SettingsWindow::RefreshGMPage()
{
    if(!gmPage_)return;
    const int level=gmSecurity_?gmSecurity_():0;
    int shown=0,total=0;
    if(gmPickerField_>=0){
        const auto choices=GMPickerOptions();
        total=static_cast<int>(choices.size());
        for(int i=gmPickerPage_*10;i<total && shown<10;++i) {
            auto* b=gmCommands_[static_cast<size_t>(shown++)];
            b->SetVisible(true);b->SetText(choices[static_cast<size_t>(i)]);b->SetSelected(false);
        }
        gmPrevious_->SetText(gmPickerPage_==0 && gmPickerStage_>0?"< Categorias":"< Anterior");
    } else {
        for(size_t j=0;j<gm::Catalog().size();++j){
            const auto& c=gm::Catalog()[j];
            if(c.group!=gmGroup_)continue;
            if(total>=gmPageIndex_*10 && shown<10){
                auto* b=gmCommands_[static_cast<size_t>(shown)];
                b->SetVisible(true);b->SetText(std::string(c.label)+(level<c.level?" [bloqueado]":""));
                b->SetSelected(gmSelected_==static_cast<int>(j));
                ++shown;
            }
            ++total;
        }
        gmPrevious_->SetText("< Anterior");
    }
    for(int i=shown;i<10;++i)gmCommands_[static_cast<size_t>(i)]->SetVisible(false);
    const int page=gmPickerField_>=0?gmPickerPage_:gmPageIndex_;
    gmPrevious_->SetEnabled(page>0 || (gmPickerField_>=0 && gmPickerStage_>0));
    gmNext_->SetEnabled((page+1)*10<total);
    for(int i=0;i<4;++i)gmGroups_[static_cast<size_t>(i)]->SetSelected(i==gmGroup_);
    const bool selected=gmSelected_>=0;
    const gm::Command* c=selected?&gm::Catalog()[static_cast<size_t>(gmSelected_)]:nullptr;
    gmTitle_->SetText(c?c->label:"Seleciona um comando");
    gmHint_->SetText(c?c->hint:"Escolhe categoria e comando, depois os parametros");
    gmPermission_->SetText(c?"Requer security "+std::to_string(c->level)+"  |  Atual: "+std::to_string(level):"Comandos GM continuam validados pelo servidor");
    for(int i=0;i<3;++i){
        const gm::Arg arg=c?c->args[static_cast<size_t>(i)]:gm::Arg::None;
        const bool visible=arg!=gm::Arg::None;
        gmFieldLabels_[static_cast<size_t>(i)]->SetVisible(visible);
        gmFields_[static_cast<size_t>(i)]->SetVisible(visible);
        gmEditButtons_[static_cast<size_t>(i)]->SetVisible(visible);
        if(!visible)continue;
        gmFieldLabels_[static_cast<size_t>(i)]->SetText(gm::ArgLabel(arg));
        const auto& v=gmValues_[static_cast<size_t>(i)];
        gmFields_[static_cast<size_t>(i)]->SetText(arg==gm::Arg::Secret?(v.empty()?"[escrever senha]":"********"):(v.empty()?"[escolher / editar]":v));
        gmFields_[static_cast<size_t>(i)]->SetEnabled(level>=c->level);
        gmEditButtons_[static_cast<size_t>(i)]->SetEnabled(level>=c->level);
        gmEditButtons_[static_cast<size_t>(i)]->SetText(i==gmEditing_?"A escrever...":"Digitar");
    }
    gmExecute_->SetEnabled(c && level>=c->level);
    gmExecute_->SetText(gmConfirmArmed_?"CONFIRMAR execucao":"Executar comando");
    std::string preview;
    if(c){
        if(c->args[0]==gm::Arg::Secret)preview="Pre-visualizacao: /ativar [senha oculta]";
        else if(!gm::Build(*c,gmValues_,preview)) preview="Faltam parametros obrigatorios";
    }
    gmPreview_->SetText(preview);
    gmFeedback_->SetText(gmStatus_.empty()?"Parametros: clica para alternar; Digitar para editar valores livres.":gmStatus_);
}

void SettingsWindow::Layout()
{
    const Rect bounds = Bounds();
    videoTab_->SetBounds({14, 40, 110, 30});
    audioTab_->SetBounds({130, 40, 110, 30});
    generalTab_->SetBounds({246, 40, 110, 30});
    lootTab_->SetBounds({362, 40, 110, 30});
    commandsTab_->SetBounds({478, 40, 110, 30});
    gmTab_->SetBounds({594, 40, 110, 30});
    closeButton_->SetBounds({bounds.width - 42, 5, 30, 24});

    const Rect pageBounds{8, kPageTop, bounds.width - 16, kFooterTop - kPageTop - 6};
    videoPage_->SetBounds(pageBounds);
    audioPage_->SetBounds(pageBounds);
    generalPage_->SetBounds(pageBounds);
    lootPage_->SetBounds(pageBounds);
    commandsPage_->SetBounds(pageBounds);
    gmPage_->SetBounds(pageBounds);

    resetButton_->SetBounds({bounds.width - 226, kFooterTop, 100, 30});
    saveButton_->SetBounds({bounds.width - 118, kFooterTop, 100, 30});
}

void SettingsWindow::SetViewport(int width, int height)
{
    Rect bounds = Bounds();
    bounds.width = std::min(kWindowWidth, std::max(680, width - 20));
    bounds.height = std::min(kWindowHeight, std::max(500, height - 20));
    bounds.x = std::max(0, (width - bounds.width) / 2);
    bounds.y = std::max(0, (height - bounds.height) / 2);
    SetBounds(bounds);
    Layout();
}

void SettingsWindow::SetActiveTab(SettingsTab tab)
{
    activeTab_ = tab;
    videoPage_->SetVisible(tab == SettingsTab::Video);
    audioPage_->SetVisible(tab == SettingsTab::Audio);
    generalPage_->SetVisible(tab == SettingsTab::General);
    lootPage_->SetVisible(tab == SettingsTab::Loot);
    commandsPage_->SetVisible(tab == SettingsTab::Commands);
    gmPage_->SetVisible(tab == SettingsTab::GM);
    videoTab_->SetSelected(tab == SettingsTab::Video);
    audioTab_->SetSelected(tab == SettingsTab::Audio);
    generalTab_->SetSelected(tab == SettingsTab::General);
    lootTab_->SetSelected(tab == SettingsTab::Loot);
    commandsTab_->SetSelected(tab == SettingsTab::Commands);
    gmTab_->SetSelected(tab == SettingsTab::GM);
    if (tab != SettingsTab::GM) { gmEditing_ = -1; gmConfirmArmed_ = false; if (gmSelected_ >= 0 && gm::Catalog()[static_cast<size_t>(gmSelected_)].args[0] == gm::Arg::Secret) gmValues_[0].clear(); }
}

void SettingsWindow::ReloadFromRuntime()
{
    ResetGmTransient();
    state_.windowed = SETTINGSHANDLER->getWindowMode();
    state_.autoAdjust = SETTINGSHANDLER->getAutoAdjust();
    state_.startMaximized = SETTINGSHANDLER->getStartMaximized();
    state_.ratio = SETTINGSHANDLER->getRatio() == 43 ? 43 : 169;
    state_.resolution = std::clamp(SETTINGSHANDLER->getResolution(), 1, 4);
    state_.textureQuality = SETTINGSHANDLER->getTextureQuality();
    state_.bitDepth = SETTINGSHANDLER->getBitDepth();
    state_.cameraSight = SETTINGSHANDLER->getSight();
    state_.cameraInverted = SETTINGSHANDLER->getCamInv();
    state_.music = SETTINGSHANDLER->hasSound();
    state_.musicVolume = std::clamp(SETTINGSHANDLER->getSoundVolume(), 0, 100);
    state_.effects = SETTINGSHANDLER->hasEffects();
    state_.effectsVolume = std::clamp(SETTINGSHANDLER->getEffectsVolume(), 0, 100);
    state_.weather = SETTINGSHANDLER->getWeather();
    state_.rememberId = SETTINGSHANDLER->getRememberID();
    state_.partyRequest = SETTINGSHANDLER->getPartyRequest();
    state_.tradeRequest = SETTINGSHANDLER->getTradeRequest();
    state_.whisper = SETTINGSHANDLER->getWhisper();
    state_.damageDebug = SETTINGSHANDLER->getDamageDebug();
    state_.hpBar = SETTINGSHANDLER->getHpBar();
    state_.fps = SETTINGSHANDLER->getFps();
    state_.locale = SETTINGSHANDLER->getLocale() == "en" ? "en" : "pt-BR";

    state_.lootEnabled = cLootFilter.getEnabled();
    state_.loot = {{
        cLootFilter.getHpPot(), cLootFilter.getMpPot(), cLootFilter.getSpPot(), cLootFilter.getGold(),
        cLootFilter.getAmulets(), cLootFilter.getRings(), cLootFilter.getSheltoms(), cLootFilter.getForce(),
        cLootFilter.getPremiums(), cLootFilter.getCrystals(), cLootFilter.getDefense(), cLootFilter.getOffense(),
        cLootFilter.getEverything(), cLootFilter.getNonSpec(), cLootFilter.getFighter(), cLootFilter.getMech(),
        cLootFilter.getArcher(), cLootFilter.getPikeman(), cLootFilter.getAtalanta(), cLootFilter.getKnight(),
        cLootFilter.getMagician(), cLootFilter.getPriestess()
    }};
    RefreshLabels();
}

void SettingsWindow::RefreshLabels()
{
    SetButton(videoButtons_[0], state_.windowed ? "Janela" : "Tela cheia", state_.windowed);
    SetButton(videoButtons_[1], BoolText(state_.autoAdjust), state_.autoAdjust);
    SetButton(videoButtons_[2], BoolText(state_.startMaximized), state_.startMaximized);
    SetButton(videoButtons_[3], state_.ratio == 43 ? "4:3" : "16:9", state_.ratio == 169);
    SetButton(videoButtons_[4], ResolutionText(state_.ratio, state_.resolution));
    SetButton(videoButtons_[5], state_.textureQuality == 1 ? "Alta" : "Baixa", state_.textureQuality == 1);
    SetButton(videoButtons_[6], std::to_string(state_.bitDepth) + " bit", state_.bitDepth == 32);
    std::string camera = state_.cameraSight ? "Alcance alto" : "Alcance normal";
    if (state_.cameraInverted) camera += " + invertida";
    SetButton(videoButtons_[7], camera, state_.cameraSight || state_.cameraInverted);

    SetButton(audioButtons_[0], std::string("Musica: ") + BoolText(state_.music), state_.music);
    SetButton(audioButtons_[1], "Volume " + std::to_string(state_.musicVolume) + "% -");
    SetButton(audioButtons_[2], "Volume " + std::to_string(state_.musicVolume) + "% +");
    SetButton(audioButtons_[3], std::string("Efeitos: ") + BoolText(state_.effects), state_.effects);
    SetButton(audioButtons_[4], "Volume " + std::to_string(state_.effectsVolume) + "% -");
    SetButton(audioButtons_[5], "Volume " + std::to_string(state_.effectsVolume) + "% +");

    const bool general[8] = {state_.weather, state_.rememberId, state_.partyRequest, state_.tradeRequest,
                             state_.whisper, state_.damageDebug, state_.hpBar, state_.fps};
    for (int i = 0; i < 8; ++i)
        SetButton(generalButtons_[static_cast<std::size_t>(i)], BoolText(general[i]), general[i]);
    SetButton(generalButtons_[8], state_.locale == "pt-BR" ? "Português (Brasil)" : "English", state_.locale == "pt-BR");

    static const char* names[23] = {
        "Filtro de loot", "Pocao HP", "Pocao MP", "Pocao RES", "Gold", "Amuletos",
        "Aneis", "Sheltoms", "Force Orbs", "Premiums", "Cristais", "Defensivos",
        "Ofensivos", "Todo o resto", "Sem Spec", "Fighter", "Mechanician", "Archer",
        "Pikeman", "Atalanta", "Knight", "Magician", "Priestess"
    };
    SetButton(lootButtons_[0], std::string(names[0]) + ": " + BoolText(state_.lootEnabled), state_.lootEnabled);
    for (int i = 1; i < 23; ++i)
    {
        const bool value = state_.loot[static_cast<std::size_t>(i - 1)];
        SetButton(lootButtons_[static_cast<std::size_t>(i)], std::string(names[i]) + ": " + BoolText(value), value);
    }
}

void SettingsWindow::Save()
{
    SETTINGSHANDLER->setWindowMode(state_.windowed);
    SETTINGSHANDLER->setAutoAdjust(state_.autoAdjust);
    SETTINGSHANDLER->setStartMaximized(state_.startMaximized);
    SETTINGSHANDLER->setTextureQuality(state_.textureQuality);
    SETTINGSHANDLER->setBitDepth(state_.bitDepth);
    SETTINGSHANDLER->setSight(state_.cameraSight);
    SETTINGSHANDLER->setCamInv(state_.cameraInverted);
    SETTINGSHANDLER->setSound(state_.music);
    SETTINGSHANDLER->setSoundVolume(state_.musicVolume);
    SETTINGSHANDLER->setEffects(state_.effects);
    SETTINGSHANDLER->setEffectsVolume(state_.effectsVolume);
    SETTINGSHANDLER->setWheater(state_.weather);
    SETTINGSHANDLER->setRememberID(state_.rememberId);
    SETTINGSHANDLER->setPartyRequest(state_.partyRequest);
    SETTINGSHANDLER->setTradeRequest(state_.tradeRequest);
    SETTINGSHANDLER->setWhisper(state_.whisper);
    SETTINGSHANDLER->setDamageDebug(state_.damageDebug);
    SETTINGSHANDLER->setHpBar(state_.hpBar);
    SETTINGSHANDLER->setFps(state_.fps);
    SETTINGSHANDLER->setLocale(state_.locale);
    summit::content::localization::SetLocalizationLocale(SETTINGSHANDLER->getLocale().c_str());

    cLootFilter.setEnabled(state_.lootEnabled);
    cLootFilter.setHpPot(state_.loot[0]);
    cLootFilter.setMpPot(state_.loot[1]);
    cLootFilter.setSpPot(state_.loot[2]);
    cLootFilter.setGold(state_.loot[3]);
    cLootFilter.setAmulets(state_.loot[4]);
    cLootFilter.setRings(state_.loot[5]);
    cLootFilter.setSheltoms(state_.loot[6]);
    cLootFilter.setForce(state_.loot[7]);
    cLootFilter.setPremiums(state_.loot[8]);
    cLootFilter.setCrystals(state_.loot[9]);
    cLootFilter.setDefense(state_.loot[10]);
    cLootFilter.setOffense(state_.loot[11]);
    cLootFilter.setEverything(state_.loot[12]);
    cLootFilter.setNonSpec(state_.loot[13]);
    cLootFilter.setFighter(state_.loot[14]);
    cLootFilter.setMech(state_.loot[15]);
    cLootFilter.setArcher(state_.loot[16]);
    cLootFilter.setPikeman(state_.loot[17]);
    cLootFilter.setAtalanta(state_.loot[18]);
    cLootFilter.setKnight(state_.loot[19]);
    cLootFilter.setMagician(state_.loot[20]);
    cLootFilter.setPriestess(state_.loot[21]);

    // Apply resolution last because Settings::setResolution may resize the
    // backbuffer/window immediately when the client configuration is loaded.
    SETTINGSHANDLER->setRatio(state_.ratio);
    SETTINGSHANDLER->setResolution(state_.resolution);
    SETTINGSHANDLER->save();
    SetVolumeBGM(0);
    PlayWav_UpdateVolume();
}

void SettingsWindow::ResetDefaults()
{
    state_ = State{};
    state_.windowed = false;
    state_.autoAdjust = true;
    state_.startMaximized = true;
    state_.ratio = 169;
    state_.resolution = 1;
    state_.textureQuality = 1;
    state_.bitDepth = 32;
    state_.cameraSight = true;
    state_.music = true;
    state_.musicVolume = 50;
    state_.effects = true;
    state_.effectsVolume = 50;
    state_.weather = true;
    state_.partyRequest = true;
    state_.tradeRequest = true;
    state_.whisper = true;
    state_.damageDebug = true;
    state_.hpBar = true;
    state_.fps = true;
    state_.locale = "pt-BR";
    state_.lootEnabled = true;
    state_.loot.fill(true);
    RefreshLabels();
}
}
