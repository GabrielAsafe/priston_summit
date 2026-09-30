#pragma once

#include <string>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace events
{
struct WorldEventStarted
{
    std::string eventKey;

    WorldEventStarted() {}
    explicit WorldEventStarted(const std::string& eventKeyValue)
        : eventKey(eventKeyValue) {}
};

struct EventScoreChanged
{
    std::string eventKey;
    PlayerId player;
    int delta;
    int playerScore;
    int team;
    int teamScore;

    EventScoreChanged()
        : delta(0), playerScore(0), team(-1), teamScore(0) {}
    EventScoreChanged(const std::string& eventKeyValue,
                      PlayerId playerValue,
                      int deltaValue,
                      int playerScoreValue,
                      int teamValue,
                      int teamScoreValue)
        : eventKey(eventKeyValue),
          player(playerValue),
          delta(deltaValue),
          playerScore(playerScoreValue),
          team(teamValue),
          teamScore(teamScoreValue) {}
};

struct WorldEventEnded
{
    std::string eventKey;
    bool completed;

    WorldEventEnded() : completed(false) {}
    WorldEventEnded(const std::string& eventKeyValue, bool completedValue)
        : eventKey(eventKeyValue), completed(completedValue) {}
};
}
}
