#include "systems/ui/v2/legacy/LegacySkillNpcWindow.h"

#include <string>
#include <vector>
#include <windows.h>

#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/ui/messages/legacy_sin/sinMsg.h"
#include "systems/ui/v2/widgets/Button.h"
#include "systems/ui/v2/widgets/Label.h"

namespace ui::v2::legacy
{
namespace
{
std::string LegacyToUtf8(const char* text)
{
    if (!text || !*text) return {};
    const int wideLen = MultiByteToWideChar(CP_ACP, 0, text, -1, nullptr, 0);
    if (wideLen <= 1) return text;
    std::vector<wchar_t> wide(static_cast<std::size_t>(wideLen));
    MultiByteToWideChar(CP_ACP, 0, text, -1, wide.data(), wideLen);
    const int utfLen = WideCharToMultiByte(CP_UTF8, 0, wide.data(), -1, nullptr, 0, nullptr, nullptr);
    if (utfLen <= 1) return text;
    std::string out(static_cast<std::size_t>(utfLen), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), -1, out.data(), utfLen, nullptr, nullptr);
    if (!out.empty() && out.back() == '\0') out.pop_back();
    return out;
}
}

LegacySkillNpcWindow::LegacySkillNpcWindow(UIId id) : Window(id, "Mestre")
{
    SetBounds({300, 190, 360, 245});
    SetMovable(true);
    SetClosable(false);

    auto& description = AddChild<Label>(id == InvalidUIId ? InvalidUIId : id + 1,
        "Escolha o servico do mestre.");
    description.SetBounds({20, 48, 320, 26});
    description_ = &description;

    auto& skills = AddChild<Button>(id == InvalidUIId ? InvalidUIId : id + 2, "Habilidades");
    skills.SetBounds({35, 92, 290, 32});
    skills.SetOnClick([this](const UIEvent&) { ChooseSkills(); });
    skillButton_ = &skills;

    auto& changeJob = AddChild<Button>(id == InvalidUIId ? InvalidUIId : id + 3, "Mudar classe");
    changeJob.SetBounds({35, 134, 290, 32});
    changeJob.SetOnClick([this](const UIEvent&) { ChooseChangeJob(); });
    changeJobButton_ = &changeJob;

    auto& exit = AddChild<Button>(id == InvalidUIId ? InvalidUIId : id + 4, "Sair");
    exit.SetBounds({35, 176, 290, 32});
    exit.SetOnClick([this](const UIEvent&) { ChooseExit(); });
    exitButton_ = &exit;
}

void LegacySkillNpcWindow::Update(float deltaSeconds)
{
    if (skillButton_)
    {
        skillButton_->SetText(LegacyToUtf8(SkillMaster[0]));
        skillButton_->SetEnabled(SkillMasterOkFlag[0] != 0);
    }
    if (changeJobButton_)
    {
        changeJobButton_->SetText(LegacyToUtf8(SkillMaster[1]));
        changeJobButton_->SetEnabled(SkillMasterOkFlag[1] != 0);
    }
    if (exitButton_)
        exitButton_->SetText(LegacyToUtf8(SkillMaster[2]));
    Window::Update(deltaSeconds);
}

void LegacySkillNpcWindow::ChooseSkills()
{
    if (!SkillMasterOkFlag[0]) return;
    SkillMasterFlag = 1;
    SkillNpcFlag = 0;
    cSkill.OpenFlag = SIN_OPEN;
    cInterFace.CheckAllBox(SIN_SKILL);
}

void LegacySkillNpcWindow::ChooseChangeJob()
{
    if (!SkillMasterOkFlag[1]) return;
    SkillMasterFlag = 2;
    SkillNpcFlag = 0;
    sinJobList = GetJobDataCode(sinChar->JOB_CODE, sinChar->ChangeJob + 1);
    cMessageBox.ShowMessage3(MESSAGE_CHANGE_JOB, sinJobList ? sinJobList->szName2 : const_cast<char*>(""));
}

void LegacySkillNpcWindow::ChooseExit()
{
    // Preserve the legacy state transition exactly. The surrounding game loop
    // owns the actual exit/restart behaviour associated with this option.
    SkillMasterFlag = 3;
    SkillNpcFlag = 0;
}
}
