#pragma once
#include <string>
#include "content/v1/stage_spawn/content/IStageSpawnContentSource.h"

namespace summit
{
namespace content
{
namespace stage_spawn
{

class StageSpawnService
{
public:
    explicit StageSpawnService(const IStageSpawnContentSource &source) : source_(source) {}

    bool Load(const char *logicalPath,StageSpawnDefinition &out,std::string *error=0) const
    {
        StageSpawnDefinition staged;
        if(!source_.Load(logicalPath,staged,error)) return false;
        if(staged.openInterval < 0 || staged.openLimit < 0 || staged.limitMax < 0)
        { if(error) *error="stage spawn contains negative runtime limits"; return false; }
        for(std::size_t i=0;i<staged.actors.size();++i)
            if(staged.actors[i].legacyNameBytes.empty() && staged.actors[i].monsterId.empty())
            { if(error) *error="stage spawn actor has no identity"; return false; }
        out=staged; if(error) error->clear(); return true;
    }

private:
    const IStageSpawnContentSource &source_;
};

} // namespace stage_spawn
} // namespace content
} // namespace summit
