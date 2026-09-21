#pragma once

#include <map>
#include <string>
#include <vector>

#include "gameplay/events/v2/GameplayEventHub.h"
#include "gameplay/reputation/v2/IReputationRepository.h"
#include "gameplay/reputation/v2/ReputationTypes.h"

namespace gameplay
{
namespace reputation
{
class ReputationService
{
public:
    explicit ReputationService(IReputationRepository& repository,
                               events::GameplayEventHub* eventHub = 0)
        : repository_(repository), eventHub_(eventHub)
    {
    }

    bool RegisterDefinition(const ReputationDefinition& definition);
    bool RegisterDefinitions(const std::vector<ReputationDefinition>& definitions);
    bool ValidateDefinitions(std::string* error = 0) const;
    const ReputationDefinition* FindDefinition(const std::string& key) const;
    std::vector<std::string> ReputationKeys() const;
    void GetCatalog(std::vector<ReputationCatalogEntry>& entries) const;
    bool GetSnapshots(const std::string& playerKey,
                      std::vector<ReputationSnapshot>& snapshots);

    bool GetValue(const std::string& playerKey,
                  const std::string& reputationKey,
                  int& value);

    bool Add(const std::string& playerKey,
             const std::string& reputationKey,
             int amount,
             const std::string& sourceKey);

private:
    int ClampValue(const ReputationDefinition& definition, long long value) const;

    IReputationRepository& repository_;
    events::GameplayEventHub* eventHub_;
    std::map<std::string, ReputationDefinition> definitions_;
};
}
}
