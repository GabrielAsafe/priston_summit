#pragma once
#include <cstdlib>
#include <string>
#include "content/v1/gameplay_config/content/CompiledGameplayConfigSource.h"
namespace summit { namespace content { namespace gameplay_config {
inline const char*GameplayConfigPackPath(){const char*p=std::getenv("SUMMIT_GAMEPLAY_CONFIG_PACK");return(p&&*p)?p:0;}inline bool GameplayConfigStrictMode(){const char*p=std::getenv("SUMMIT_GAMEPLAY_CONFIG_STRICT");return p&&p[0]=='1';}
inline const CompiledGameplayConfigSource*ActiveGameplayConfigSource(){struct H{std::string p;CompiledGameplayConfigSource*s;H():s(0){const char*x=GameplayConfigPackPath();if(x){p=x;s=new CompiledGameplayConfigSource(p.c_str());}}~H(){delete s;}};static H h;return h.s;}
inline bool GetGameplayConfigValue(const char*path,const char*section,const char*key,std::string&out){const CompiledGameplayConfigSource*s=ActiveGameplayConfigSource();return s&&s->IsValid()&&s->Get(path,section,key,out);}
inline bool TryGameplayConfigInt(const char*path,const char*section,const char*key,int&out){std::string raw;if(!GetGameplayConfigValue(path,section,key,raw))return false;char*end=0;long v=std::strtol(raw.c_str(),&end,0);if(end==raw.c_str())return false;out=static_cast<int>(v);return true;}
inline int GetGameplayConfigInt(const char*path,const char*section,const char*key,int fallback){int value=0;if(TryGameplayConfigInt(path,section,key,value))return value;return fallback;}
inline bool CompiledGameplayConfigEnabled(){return GameplayConfigPackPath()!=0;}
}}}
