#pragma once
#include <cstdlib>
#include <string>
#include <vector>
#include "content/v1/localization/content/CompiledLocalizationSource.h"
#include "content/v1/text_resource/content/TextResourceRuntime.h"

namespace summit { namespace content { namespace localization {

inline bool LocalizationStrictMode(){const char*value=std::getenv("SUMMIT_LOCALIZATION_STRICT");return value&&value[0]=='1';}
inline const char* LocalizationPackPath(){return summit::content::text_resource::TextResourcePackPath();}
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
        CompiledLocalizationSource*source;
        Holder():source(0){
            // Strictly one binary source for text: the shipped SRES pack.
            // Preserve canonical SLOC v2 bytes *inside* SRES v2, in memory only.
            std::vector<unsigned char> bytes;
            std::string contentId;
            std::uint32_t kind=0;
            const summit::content::text_resource::CompiledTextResourceSource* text =
                summit::content::text_resource::ActiveTextResourceSource();
            if(!text || !text->IsValid() ||
               !text->Load("__summit_internal__/localization.sloc", bytes, &contentId, &kind) ||
               contentId != "summit.localization.catalog.v2" || kind != 4u) return;
            CompiledLocalizationSource*next=new CompiledLocalizationSource(bytes);
            if(next->IsValid()) source=next; else delete next;
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
