#pragma once
#include <cstdlib>
#include <string>
#include <vector>
#include "content/v1/localization/content/CompiledLocalizationSource.h"

namespace summit { namespace content { namespace localization {

inline bool LocalizationStrictMode(){const char*value=std::getenv("SUMMIT_LOCALIZATION_STRICT");return value&&value[0]=='1';}
inline const char* LocalizationPackPath(){const char*path=std::getenv("SUMMIT_LOCALIZATION_PACK");return(path&&*path)?path:0;}
inline const char* LocalizationLocale(){const char*locale=std::getenv("SUMMIT_LOCALE");return(locale&&*locale)?locale:"en";}
inline bool LocalizationOverrideEnabled(){const char*locale=std::getenv("SUMMIT_LOCALE");return locale&&*locale;}

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
