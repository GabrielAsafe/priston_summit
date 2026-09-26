#pragma once
#include <cstdlib>
#include <string>
#include "content/v1/definition/content/CompiledDefinitionSource.h"
namespace summit { namespace content { namespace definition {
inline const char* DefinitionPackPath(){const char*p=std::getenv("SUMMIT_DEFINITION_PACK");return(p&&*p)?p:0;}
inline bool DefinitionStrictMode(){const char*p=std::getenv("SUMMIT_DEFINITION_STRICT");return p&&p[0]=='1';}
inline const CompiledDefinitionSource* ActiveDefinitionSource(){struct H{std::string path;CompiledDefinitionSource*s;H():s(0){const char*p=DefinitionPackPath();if(p){path=p;s=new CompiledDefinitionSource(path.c_str());}}~H(){delete s;}};static H h;return h.s;}
inline bool CompiledDefinitionsEnabled(){return DefinitionPackPath()!=0;}
inline bool LoadCompiledDefinition(const char*path,CompiledDefinition&out){const CompiledDefinitionSource*s=ActiveDefinitionSource();return s&&s->IsValid()&&s->Load(path,out,0);}
inline bool ListCompiledDefinitions(const char*prefix,const char*extension,bool directOnly,std::vector<DefinitionEntry>&out,std::uint32_t kind=0){const CompiledDefinitionSource*s=ActiveDefinitionSource();return s&&s->IsValid()&&s->List(prefix,extension,directOnly,out,kind,0);}
}}}
