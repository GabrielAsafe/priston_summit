#pragma once

#include <array>
#include <functional>
#include <string>
#include <vector>
#include <cstdint>
#include "systems/ui/v2/windows/GmCommandCatalog.h"
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
    Loot,
    Commands,
    GM
};

class SettingsWindow final : public Window
{
public:
    using CloseHandler = std::function<void()>;

    explicit SettingsWindow(UIId id = InvalidUIId);

    void SetViewport(int width, int height);
    void SetOnClose(CloseHandler handler) { onClose_ = std::move(handler); }
    void ReloadFromRuntime();
    using SecurityProvider = std::function<int()>;
    using GmDispatch = std::function<bool(const std::string&, bool)>;
    using NameProvider = std::function<std::vector<std::string>()>;
    void SetGmBridge(SecurityProvider security, GmDispatch dispatch, NameProvider names);
    void SetOnSpawnEditor(std::function<void()> callback) { onSpawnEditor_ = std::move(callback); }
    void ResetGmTransient();
    bool HandleGmKey(std::uint32_t key);
    bool HandleGmChar(std::uint32_t character);
    bool EditingGmField() const noexcept { return activeTab_ == SettingsTab::GM && gmEditing_ >= 0; }


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
        std::string locale = "pt-BR";

        bool lootEnabled = true;
        std::array<bool, 22> loot{};
    };

    void BuildChrome();
    void BuildVideoPage();
    void BuildAudioPage();
    void BuildGeneralPage();
    void BuildLootPage();
    void BuildCommandsPage();
    void BuildGMPage();
    void SelectGMGroup(int group);
    void SelectGMCommand(int index);
    void RefreshGMPage();
    void CycleGMField(int index);
    void ChooseGMListEntry(int index);
    std::vector<std::string> GMPickerOptions() const;
    void EditGMField(int index);
    void ExecuteGM();
    std::vector<std::string> GMOptions(gm::Arg argument) const;

    void Layout();
    void SetActiveTab(SettingsTab tab);
    void SetCommandGroup(int group);
    void RefreshCommandPage();
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
    Button* commandsTab_ = nullptr;
    Button* gmTab_ = nullptr;
    Button* closeButton_ = nullptr;
    Button* saveButton_ = nullptr;
    Button* resetButton_ = nullptr;

    Container* videoPage_ = nullptr;
    Container* audioPage_ = nullptr;
    Container* generalPage_ = nullptr;
    Container* lootPage_ = nullptr;
    Container* commandsPage_ = nullptr;
    Container* gmPage_ = nullptr;

    std::array<Button*, 8> videoButtons_{};
    std::array<Button*, 6> audioButtons_{};
    std::array<Button*, 9> generalButtons_{};
    std::array<Button*, 23> lootButtons_{};
    std::array<Button*, 4> commandGroupButtons_{};
    std::array<Label*, 24> commandLabels_{};
    int commandGroup_ = 0;

    SecurityProvider gmSecurity_;
    GmDispatch gmDispatch_;
    NameProvider gmNames_;
    std::function<void()> onSpawnEditor_;
    Button* gmSpawnEditor_ = nullptr;
    std::array<Button*, 4> gmGroups_{};
    std::array<Button*, 10> gmCommands_{};
    std::array<Button*, 3> gmFields_{};
    std::array<Button*, 3> gmEditButtons_{};
    std::array<Label*, 3> gmFieldLabels_{};
    Label* gmTitle_ = nullptr;
    Label* gmHint_ = nullptr;
    Label* gmPermission_ = nullptr;
    Label* gmPreview_ = nullptr;
    Label* gmFeedback_ = nullptr;
    Button* gmPrevious_ = nullptr;
    Button* gmNext_ = nullptr;
    Button* gmExecute_ = nullptr;
    int gmGroup_ = 0;
    int gmPageIndex_ = 0;
    int gmSelected_ = -1;
    int gmEditing_ = -1;
    bool gmConfirmArmed_ = false;
    int gmPickerField_ = -1;
    int gmPickerPage_ = 0;
    int gmPickerStage_ = 0;
    std::string gmPickerPrefix_;
    std::array<bool,3> gmTyped_{{false,false,false}};
    std::array<std::string,3> gmValues_{};
    std::string gmStatus_;

};
}
