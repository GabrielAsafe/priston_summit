#pragma once
#include <string>
#include <vector>
#include "content/v1/localization/content/CompiledLocalizationSource.h"
#include "content/v1/runtime/RuntimeContentPath.h"

namespace summit { namespace content { namespace localization {

inline bool LocalizationStrictMode(){return false;}
inline const char* LocalizationPackPath(){static const std::string p=summit::content::runtime::ResolveRuntimePackPath("localization.sloc");return p.c_str();}
inline const char* LocalizationLocale(){
#ifdef _LANGUAGE_BRAZIL
 return "pt-BR";
#else
 return "en";
#endif
}
inline bool LocalizationOverrideEnabled(){
#ifdef _LANGUAGE_BRAZIL
 return true;
#else
 return false;
#endif
}

inline const CompiledLocalizationSource* ActiveLocalizationSource(){
    struct Holder{std::string path;CompiledLocalizationSource*source;Holder():source(0){const char*configured=LocalizationPackPath();if(configured){path=configured;source=new CompiledLocalizationSource(path.c_str());}}~Holder(){delete source;}};
    static Holder holder;return holder.source;
}
inline bool CompiledLocalizationEnabled(){return LocalizationPackPath()!=0;}
inline bool CompiledLocalizationReady(){const CompiledLocalizationSource*source=ActiveLocalizationSource();return source&&source->IsValid();}
inline bool GetLocalizationValue(const char*key,std::string&out,std::uint32_t*flags=0){const CompiledLocalizationSource*source=ActiveLocalizationSource();return source&&source->IsValid()&&source->Get(LocalizationLocale(),key,out,flags);}
inline bool GetLocalizationValueForLocale(const char*locale,const char*key,std::string&out,std::uint32_t*flags=0){const CompiledLocalizationSource*source=ActiveLocalizationSource();return source&&source->IsValid()&&source->Get(locale,key,out,flags);}
inline bool GetLocalizationEntries(const char*locale,std::vector<LocalizationEntry>&out){const CompiledLocalizationSource*source=ActiveLocalizationSource();return source&&source->IsValid()&&source->Entries(locale,out);}

}}} // namespace summit::content::localization
