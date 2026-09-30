#pragma once

#include "content/v1/gameplay_reference/content/CompiledGameplayReferenceSource.h"
#include "content/v1/runtime/RuntimeContentPath.h"

namespace summit { namespace content { namespace gameplay_reference {

inline CompiledGameplayReferenceSource& GameplayReferenceRuntime()
{
    static const std::string packPath = summit::content::runtime::ResolveRuntimePackPath("gameplay_reference.sgrf");
    static CompiledGameplayReferenceSource source(packPath.c_str());
    return source;
}

}}}
