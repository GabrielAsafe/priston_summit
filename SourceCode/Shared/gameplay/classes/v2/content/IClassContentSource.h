#pragma once

#include <string>
#include <vector>

#include "gameplay/classes/v2/domain/ClassDefinition.h"

namespace gameplay
{
namespace classes
{
// Source boundary for class content. The registry/service layers do not need to
// know whether definitions came from compiled legacy parity data, a future file
// loader, a database, or a test fixture.
class IClassContentSource
{
public:
    virtual ~IClassContentSource() {}

    virtual bool Load(std::vector<PlayableClassDefinition>& out,
                      std::string* error = 0) const = 0;
};
}
}
