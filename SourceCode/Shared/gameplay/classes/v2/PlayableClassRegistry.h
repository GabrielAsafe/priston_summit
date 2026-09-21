#pragma once

#include <string>
#include <vector>

#include "gameplay/classes/v2/domain/ClassDefinition.h"

namespace gameplay
{
namespace classes
{
class PlayableClassRegistry
{
public:
    bool Register(const PlayableClassDefinition& definition);
    bool RegisterAll(const std::vector<PlayableClassDefinition>& definitions, std::string* error = 0);
    bool Validate(std::string* error = 0) const;

    const PlayableClassDefinition* FindById(PlayableClassId id) const;
    const PlayableClassDefinition* FindByLegacyJobCode(int legacyJobCode) const;
    const PlayableClassDefinition* FindByKey(const std::string& key) const;
    const PlayableClassDefinition* FindByBroodAndSelectionOrder(int brood, int selectionOrder) const;

    bool IsEnabled(int legacyJobCode) const;
    std::vector<PlayableClassDefinition> EnabledClasses() const;
    std::vector<PlayableClassDefinition> EnabledClassesForBrood(int brood) const;
    const std::vector<PlayableClassDefinition>& AllClasses() const { return definitions_; }

private:
    std::vector<PlayableClassDefinition> definitions_;
};

class IClassContentSource;

// Transactional composition boundary. The registry is updated only after the
// complete source payload loads, registers and validates successfully.
bool LoadClassContent(PlayableClassRegistry& registry,
                      const IClassContentSource& source,
                      std::string* error = 0);

}
}
