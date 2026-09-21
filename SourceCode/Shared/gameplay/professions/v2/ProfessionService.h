#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace gameplay
{
namespace professions
{
enum class ProfessionType
{
    None = 0,
    Arcanist,
    Blacksmith,
    Lumberjack,
    Miner,
    Artisan,
    Fisher,
    Hunter,
    Custom
};

struct ProfessionProgress
{
    int level;
    std::uint64_t experience;

    ProfessionProgress() : level(0), experience(0) {}
    ProfessionProgress(int levelValue, std::uint64_t experienceValue)
        : level(std::max(0, levelValue)), experience(experienceValue) {}
};

class ProfessionService
{
public:
    void SetProgress(const std::string& playerName, ProfessionType type,
                     const ProfessionProgress& progress)
    {
        if (playerName.empty() || type == ProfessionType::None)
            return;
        progressByPlayer_[playerName][type] = ProfessionProgress(progress.level, progress.experience);
    }

    bool GetProgress(const std::string& playerName, ProfessionType type,
                     ProfessionProgress& out) const
    {
        const PlayerMap::const_iterator playerIt = progressByPlayer_.find(playerName);
        if (playerIt == progressByPlayer_.end())
            return false;
        const ProgressMap::const_iterator progressIt = playerIt->second.find(type);
        if (progressIt == playerIt->second.end())
            return false;
        out = progressIt->second;
        return true;
    }

    int GetLevel(const std::string& playerName, ProfessionType type) const
    {
        ProfessionProgress progress;
        return GetProgress(playerName, type, progress) ? progress.level : 0;
    }

    std::uint64_t GetExperience(const std::string& playerName, ProfessionType type) const
    {
        ProfessionProgress progress;
        return GetProgress(playerName, type, progress) ? progress.experience : 0;
    }

    bool AddExperience(const std::string& playerName, ProfessionType type, std::uint64_t amount)
    {
        if (playerName.empty() || type == ProfessionType::None || amount == 0)
            return false;
        ProfessionProgress& progress = progressByPlayer_[playerName][type];
        progress.experience += amount;
        return true;
    }

    void ClearPlayer(const std::string& playerName)
    {
        progressByPlayer_.erase(playerName);
    }

    void Clear()
    {
        progressByPlayer_.clear();
    }

private:
    struct ProfessionTypeHash
    {
        std::size_t operator()(ProfessionType type) const
        {
            return static_cast<std::size_t>(type);
        }
    };

    typedef std::unordered_map<ProfessionType, ProfessionProgress, ProfessionTypeHash> ProgressMap;
    typedef std::unordered_map<std::string, ProgressMap> PlayerMap;
    PlayerMap progressByPlayer_;
};
}
}
