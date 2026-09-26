#pragma once

#include <string>
#include "content/v1/map/domain/MapDefinition.h"

namespace summit
{
namespace content
{
namespace map
{

class IMapContentSource
{
public:
    virtual ~IMapContentSource() {}
    virtual bool LoadCatalog(MapCatalogDefinition &out,std::string *error=0) const = 0;
    virtual bool LoadField(std::int32_t legacyFieldCode,MapDefinition &out,std::string *error=0) const = 0;
};

} // namespace map
} // namespace content
} // namespace summit
