#pragma once

#include <string>
#include <vector>

#include "gameplay/covenants/v2/CovenantTypes.h"
#include "gameplay/reputation/v2/ReputationTypes.h"
#include "gameplay/story/v2/StoryTypes.h"

namespace gameplay
{
namespace covenants { class CovenantService; }
namespace story { class StoryService; }
namespace progression
{
struct CovenantContentDefinition
{
    covenants::CovenantDefinition definition;
    std::vector<covenants::CovenantRankDefinition> ranks;
};

struct StoryContentDefinition
{
    story::StoryDefinition definition;
    std::vector<story::StoryArcDefinition> arcs;
    std::vector<story::StoryChapterDefinition> chapters;
    std::vector<story::StoryNodeDefinition> nodes;
};

struct ProgressionContentCatalog
{
    std::vector<reputation::ReputationDefinition> reputations;
    std::vector<CovenantContentDefinition> covenants;
    std::vector<StoryContentDefinition> stories;
};

class ProgressionContentRegistrar
{
public:
    static bool Register(const ProgressionContentCatalog& catalog,
                         covenants::CovenantService& covenantService,
                         story::StoryService& storyService,
                         std::string* error = 0);
};
}
}
