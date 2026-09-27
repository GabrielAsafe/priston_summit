#include "systems/ui/v2/windows/SettingsWindow.h"

#include <algorithm>
#include <cstdio>
#include <string>

#include "systems/configuration/client/Settings.h"
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

    const char* names[8] = {
        "Clima", "Lembrar ID", "Pedidos de grupo", "Pedidos de troca",
        "Mensagens privadas", "Exibir dano", "Barra de HP", "Mostrar FPS"
    };
    for (int i = 0; i < 8; ++i)
    {
        const int col = i >= 4 ? 1 : 0;
        const int row = i % 4;
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

void SettingsWindow::Layout()
{
    const Rect bounds = Bounds();
    videoTab_->SetBounds({14, 40, 110, 30});
    audioTab_->SetBounds({130, 40, 110, 30});
    generalTab_->SetBounds({246, 40, 110, 30});
    lootTab_->SetBounds({362, 40, 110, 30});
    closeButton_->SetBounds({bounds.width - 42, 5, 30, 24});

    const Rect pageBounds{8, kPageTop, bounds.width - 16, kFooterTop - kPageTop - 6};
    videoPage_->SetBounds(pageBounds);
    audioPage_->SetBounds(pageBounds);
    generalPage_->SetBounds(pageBounds);
    lootPage_->SetBounds(pageBounds);

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
    videoTab_->SetSelected(tab == SettingsTab::Video);
    audioTab_->SetSelected(tab == SettingsTab::Audio);
    generalTab_->SetSelected(tab == SettingsTab::General);
    lootTab_->SetSelected(tab == SettingsTab::Loot);
}

void SettingsWindow::ReloadFromRuntime()
{
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
    state_.lootEnabled = true;
    state_.loot.fill(true);
    RefreshLabels();
}
}
