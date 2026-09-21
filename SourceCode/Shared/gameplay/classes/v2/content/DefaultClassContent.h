#pragma once

#include <string>

#include "gameplay/classes/v2/PlayableClassRegistry.h"

namespace gameplay
{
namespace classes
{
// Default application composition. Registry/service code must not know which
// concrete content packs are installed.
bool BuildDefaultPlayableClassRegistry(PlayableClassRegistry& out,
                                       std::string* error = 0);

const PlayableClassRegistry& DefaultPlayableClassRegistry();
}
}
