#pragma once

#include <string>
#include "content/v1/stage_spawn/domain/StageSpawnDefinition.h"

namespace summit
{
namespace content
{
namespace stage_spawn
{

class IStageSpawnContentSource
{
public:
    virtual ~IStageSpawnContentSource() {}

    // logicalPath is the runtime path passed by the existing field loader,
    // e.g. GameServer/Field/fore-1.ase.spm.
    virtual bool Load(const char *logicalPath,
                      StageSpawnDefinition &out,
                      std::string *error = 0) const = 0;
};

} // namespace stage_spawn
} // namespace content
} // namespace summit
