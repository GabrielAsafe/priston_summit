#pragma once
#include <string>
#include "content/v1/definition/content/CompiledDefinitionSource.h"
#include "content/v1/runtime/RuntimeContentPath.h"
namespace summit { namespace content { namespace definition {
inline const char* DefinitionPackPath(){
#ifdef _W_SERVER
 static const std::string path=summit::content::runtime::ResolveRuntimePackPath("definitions_server.scfg");
 return path.c_str();
#else
 static const std::string path=summit::content::runtime::ResolveRuntimePackPath("definitions_client.scfg");
 return path.c_str();
#endif
}
inline bool DefinitionStrictMode(){return true;}
inline const CompiledDefinitionSource* ActiveDefinitionSource(){struct H{std::string path;CompiledDefinitionSource*s;H():path(DefinitionPackPath()),s(new CompiledDefinitionSource(path.c_str())){}~H(){delete s;}};static H h;return h.s;}
inline bool CompiledDefinitionsEnabled(){return true;}
inline bool CompiledItemDefinitionsEnabled(){return true;}
inline bool LoadCompiledDefinition(const char*path,CompiledDefinition&out){const CompiledDefinitionSource*s=ActiveDefinitionSource();return s&&s->IsValid()&&s->Load(path,out,0);}
inline bool ListCompiledDefinitions(const char*prefix,const char*extension,bool directOnly,std::vector<DefinitionEntry>&out,std::uint32_t kind=0){const CompiledDefinitionSource*s=ActiveDefinitionSource();return s&&s->IsValid()&&s->List(prefix,extension,directOnly,out,kind,0);}
}}}
