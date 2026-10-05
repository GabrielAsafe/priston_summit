#pragma once
#include <cstdlib>
#include <string>
#include <vector>
#include "content/v1/localization/content/CompiledLocalizationSource.h"
#include "content/v1/runtime/RuntimeContentPath.h"

namespace summit { namespace content { namespace localization {

inline bool LocalizationStrictMode(){const char*value=std::getenv("SUMMIT_LOCALIZATION_STRICT");return value&&value[0]=='1';}
inline const char* LocalizationPackPath(){const char*overridePath=std::getenv("SUMMIT_LOCALIZATION_PACK");if(overridePath&&*overridePath)return overridePath;static const std::string p=summit::content::runtime::ResolveRuntimePackPath("localization.sloc");return p.c_str();}
inline std::string& LocalizationLocaleOverrideStorage(){static std::string value;return value;}
inline bool IsSupportedLocalizationLocale(const char*locale){return locale&&(std::string(locale)=="pt-BR"||std::string(locale)=="en");}
inline void SetLocalizationLocale(const char*locale){LocalizationLocaleOverrideStorage()=IsSupportedLocalizationLocale(locale)?locale:"pt-BR";}
inline void ClearLocalizationLocaleOverride(){LocalizationLocaleOverrideStorage().clear();}
inline const char* LocalizationLocale(){
    const std::string&runtime=LocalizationLocaleOverrideStorage();if(!runtime.empty())return runtime.c_str();
    const char*locale=std::getenv("SUMMIT_LOCALE");if(IsSupportedLocalizationLocale(locale))return locale;
#ifdef _LANGUAGE_BRAZIL
    return "pt-BR";
#else
    return "pt-BR";
#endif
}
inline bool LocalizationOverrideEnabled(){return !LocalizationLocaleOverrideStorage().empty()||(std::getenv("SUMMIT_LOCALE")&&*std::getenv("SUMMIT_LOCALE"));}

inline const CompiledLocalizationSource* ActiveLocalizationSource(){
    struct Holder{
        std::string path; CompiledLocalizationSource*source;
        Holder():source(0){
            const char*configured=LocalizationPackPath();
            if(configured && Try(configured)) return;
            static const char*candidates[]={
                "Content/packs/localization.sloc",
                "content/packs/localization.sloc",
                "packs/localization.sloc",
                "localization.sloc",
                "../Content/packs/localization.sloc",
                "../SourceCode/Content/packs/localization.sloc"
            };
            for(std::size_t i=0;i<sizeof(candidates)/sizeof(candidates[0]);++i)
                if(Try(candidates[i])) return;
        }
        bool Try(const char*candidate){
            CompiledLocalizationSource*next=new CompiledLocalizationSource(candidate);
            if(next->IsValid()){delete source;source=next;path=candidate;return true;}
            delete next;return false;
        }
        ~Holder(){delete source;}
    };
    static Holder holder;return holder.source;
}
inline bool CompiledLocalizationEnabled(){return ActiveLocalizationSource()!=0;}
inline bool CompiledLocalizationReady(){const CompiledLocalizationSource*source=ActiveLocalizationSource();return source&&source->IsValid();}
inline bool GetLocalizationValue(const char*key,std::string&out,std::uint32_t*flags=0){const CompiledLocalizationSource*source=ActiveLocalizationSource();if(!source||!source->IsValid())return false;return LocalizationStrictMode()?source->GetExact(LocalizationLocale(),key,out,flags):source->Get(LocalizationLocale(),key,out,flags);}
inline bool GetLocalizationValueForLocale(const char*locale,const char*key,std::string&out,std::uint32_t*flags=0){const CompiledLocalizationSource*source=ActiveLocalizationSource();return source&&source->IsValid()&&source->Get(locale,key,out,flags);}
inline bool GetLocalizationEntries(const char*locale,std::vector<LocalizationEntry>&out){const CompiledLocalizationSource*source=ActiveLocalizationSource();return source&&source->IsValid()&&source->Entries(locale,out);}

}}} // namespace summit::content::localization
