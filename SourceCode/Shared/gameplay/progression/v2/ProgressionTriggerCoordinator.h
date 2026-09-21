#pragma once

#include <map>
#include <string>

namespace gameplay
{
namespace progression
{
class ProgressionTriggerCoordinator
{
    struct State;
public:
    explicit ProgressionTriggerCoordinator(int maxReruns = 8)
        : maxReruns_(maxReruns > 0 ? maxReruns : 1)
    {
    }

    bool IsRunning(const std::string& playerKey) const
    {
        std::map<std::string, State>::const_iterator it = states_.find(playerKey);
        return it != states_.end() && it->second.running;
    }

    template <typename Runner>
    bool Request(const std::string& playerKey, Runner runner)
    {
        if (playerKey.empty())
            return false;

        State& state = states_[playerKey];
        if (state.running)
        {
            state.pending = true;
            return true;
        }

        state.running = true;
        bool ok = true;
        int runs = 0;
        do
        {
            state.pending = false;
            ++runs;
            if (!runner())
            {
                ok = false;
                break;
            }
        }
        while (state.pending && runs < maxReruns_);

        if (state.pending)
            ok = false;

        state.running = false;
        state.pending = false;
        states_.erase(playerKey);
        return ok;
    }

private:
    struct State
    {
        bool running;
        bool pending;

        State() : running(false), pending(false) {}
    };

    int maxReruns_;
    std::map<std::string, State> states_;
};
}
}
