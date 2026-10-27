#pragma once

#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "gameplay/professions/v2/IProfessionRepository.h"
#include "gameplay/professions/v2/IProfessionService.h"
#include "gameplay/professions/v2/ProfessionLevelRules.h"
#include "gameplay/events/v2/GameplayEventHub.h"
#include "gameplay/events/v2/ProfessionProgressedEvent.h"

namespace gameplay
{
namespace professions
{
class ProfessionService : public IProfessionService
{
public:
    explicit ProfessionService(IProfessionRepository* repository = 0,
                               events::GameplayEventHub* eventHub = 0)
        : repository_(repository), eventHub_(eventHub)
    {
    }

    void SetRepository(IProfessionRepository* repository)
    {
        repository_ = repository;
        Clear();
    }

    void SetEventHub(events::GameplayEventHub* eventHub)
    {
        eventHub_ = eventHub;
    }

    bool LoadPlayer(const std::string& playerName) override
    {
        return EnsureLoaded(playerName);
    }

    void SetProgress(const std::string& playerName, ProfessionType type,
                     const ProfessionProgress& progress) override
    {
        if (playerName.empty() || type == ProfessionType::None)
            return;
        if (!EnsureLoaded(playerName))
            return;

        ProfessionProgress previous;
        const ProgressMap& existing = progressByPlayer_[playerName];
        const ProgressMap::const_iterator found = existing.find(type);
        if (found != existing.end())
            previous = found->second;

        const ProfessionProgress normalized(progress.level, progress.experience);
        if (repository_ && !repository_->SaveProgress(playerName, type, normalized))
            return;
        progressByPlayer_[playerName][type] = normalized;
        PublishProgressed(playerName, type, previous, normalized);
    }

    bool GetProgress(const std::string& playerName, ProfessionType type,
                     ProfessionProgress& out) const override
    {
        if (!EnsureLoaded(playerName))
            return false;
        const PlayerMap::const_iterator playerIt = progressByPlayer_.find(playerName);
        if (playerIt == progressByPlayer_.end())
            return false;
        const ProgressMap::const_iterator progressIt = playerIt->second.find(type);
        if (progressIt == playerIt->second.end())
            return false;
        out = progressIt->second;
        return true;
    }

    int GetLevel(const std::string& playerName, ProfessionType type) const override
    {
        ProfessionProgress progress;
        return GetProgress(playerName, type, progress) ? progress.level : 0;
    }

    std::uint64_t GetExperience(const std::string& playerName, ProfessionType type) const override
    {
        ProfessionProgress progress;
        return GetProgress(playerName, type, progress) ? progress.experience : 0;
    }

    bool AddExperience(const std::string& playerName, ProfessionType type, std::uint64_t amount) override
    {
        if (playerName.empty() || type == ProfessionType::None || amount == 0 || !EnsureLoaded(playerName))
            return false;

        ProfessionProgress current;
        const ProgressMap& existing = progressByPlayer_[playerName];
        const ProgressMap::const_iterator found = existing.find(type);
        if (found != existing.end())
            current = found->second;
        if (current.experience > std::numeric_limits<std::uint64_t>::max() - amount)
            return false;

        const std::uint64_t newExperience = current.experience + amount;
        const int earnedLevel = ProfessionLevelForExperience(newExperience);
        const int newLevel = current.level > earnedLevel ? current.level : earnedLevel;
        ProfessionProgress next(newLevel, newExperience);
        if (repository_ && !repository_->SaveProgress(playerName, type, next))
            return false;
        progressByPlayer_[playerName][type] = next;
        PublishProgressed(playerName, type, current, next);
        return true;
    }

    void ClearPlayer(const std::string& playerName) override
    {
        progressByPlayer_.erase(playerName);
        loadedPlayers_.erase(playerName);
    }

    void Clear()
    {
        progressByPlayer_.clear();
        loadedPlayers_.clear();
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

    bool EnsureLoaded(const std::string& playerName) const
    {
        if (playerName.empty())
            return false;
        if (loadedPlayers_.find(playerName) != loadedPlayers_.end())
            return true;

        if (!repository_)
        {
            loadedPlayers_.insert(playerName);
            return true;
        }

        std::vector<ProfessionRecord> records;
        if (!repository_->LoadPlayer(playerName, records))
            return false;

        ProgressMap loaded;
        for (std::vector<ProfessionRecord>::const_iterator it = records.begin(); it != records.end(); ++it)
        {
            if (it->type != ProfessionType::None)
                loaded[it->type] = ProfessionProgress(it->progress.level, it->progress.experience);
        }
        if (!loaded.empty())
            progressByPlayer_[playerName] = loaded;
        loadedPlayers_.insert(playerName);
        return true;
    }

    void PublishProgressed(const std::string& playerName, ProfessionType type,
                           const ProfessionProgress& previous,
                           const ProfessionProgress& current) const
    {
        if (!eventHub_)
            return;
        if (previous.level == current.level && previous.experience == current.experience)
            return;
        eventHub_->Publish(events::ProfessionProgressedEvent(playerName, type, previous, current));
    }

    IProfessionRepository* repository_;
    events::GameplayEventHub* eventHub_;
    mutable PlayerMap progressByPlayer_;
    mutable std::unordered_set<std::string> loadedPlayers_;
};
}
}
