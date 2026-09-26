#pragma once

#include <string>
#include <vector>

namespace gameplay
{
namespace covenants
{
struct CovenantSurveyArea
{
    std::string resourceKey;
    int area;
    int nodeCount;

    CovenantSurveyArea() : area(-1), nodeCount(0) {}
};

struct CovenantChatCommandContext
{
    std::string playerKey;
    int playerLevel;
    int legacyJobCode;
    bool allowAdminCommands;
    bool canSwitch;
    std::string switchBlockReason;
    unsigned int currentTargetSerial;
    bool currentTargetIsHuntTarget;
    std::vector<CovenantSurveyArea> surveyAreas;

    CovenantChatCommandContext()
        : playerLevel(0), legacyJobCode(0), allowAdminCommands(false), canSwitch(true), currentTargetSerial(0), currentTargetIsHuntTarget(false)
    {
    }
};

struct CovenantChatCommandResponse
{
    bool handled;
    bool success;
    std::vector<std::string> lines;

    CovenantChatCommandResponse() : handled(false), success(false) {}
};

// Transport-independent chat bridge. The legacy packet layer only needs to call
// this for incoming text and, when handled=true, send each response line back to
// the same player instead of broadcasting the slash command.
CovenantChatCommandResponse HandleServerCovenantChatCommand(
    const CovenantChatCommandContext& context,
    const std::string& text);
}
}
