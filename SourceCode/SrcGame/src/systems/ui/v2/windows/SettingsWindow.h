#pragma once

#include <array>
#include <functional>
#include "systems/ui/v2/core/Window.h"

namespace ui::v2
{
class Button;
class Label;
class Container;

enum class SettingsTab
{
    Video,
    Audio,
    General,
    Loot
};

class SettingsWindow final : public Window
{
public:
    using CloseHandler = std::function<void()>;

    explicit SettingsWindow(UIId id = InvalidUIId);

    void SetViewport(int width, int height);
    void SetOnClose(CloseHandler handler) { onClose_ = std::move(handler); }
    void ReloadFromRuntime();

private:
    struct State
    {
        bool windowed = false;
        bool autoAdjust = true;
        bool startMaximized = true;
        int ratio = 169;
        int resolution = 1;
        int textureQuality = 1;
        int bitDepth = 32;
        bool cameraSight = true;
        bool cameraInverted = false;

        bool music = true;
        int musicVolume = 50;
        bool effects = true;
        int effectsVolume = 50;

        bool weather = true;
        bool rememberId = false;
        bool partyRequest = true;
        bool tradeRequest = true;
        bool whisper = true;
        bool damageDebug = true;
        bool hpBar = true;
        bool fps = true;

        bool lootEnabled = true;
        std::array<bool, 22> loot{};
    };

    void BuildChrome();
    void BuildVideoPage();
    void BuildAudioPage();
    void BuildGeneralPage();
    void BuildLootPage();
    void Layout();
    void SetActiveTab(SettingsTab tab);
    void RefreshLabels();
    void Save();
    void ResetDefaults();

    static const char* BoolText(bool value) noexcept { return value ? "Ligado" : "Desligado"; }

    SettingsTab activeTab_ = SettingsTab::Video;
    State state_{};
    CloseHandler onClose_;

    Button* videoTab_ = nullptr;
    Button* audioTab_ = nullptr;
    Button* generalTab_ = nullptr;
    Button* lootTab_ = nullptr;
    Button* closeButton_ = nullptr;
    Button* saveButton_ = nullptr;
    Button* resetButton_ = nullptr;

    Container* videoPage_ = nullptr;
    Container* audioPage_ = nullptr;
    Container* generalPage_ = nullptr;
    Container* lootPage_ = nullptr;

    std::array<Button*, 8> videoButtons_{};
    std::array<Button*, 6> audioButtons_{};
    std::array<Button*, 8> generalButtons_{};
    std::array<Button*, 23> lootButtons_{};
};
}
