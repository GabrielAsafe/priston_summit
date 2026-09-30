#pragma once
#include <string>
#include <vector>
#include <windows.h>
#include "content/v1/localization/content/LocalizationRuntime.h"

namespace summit { namespace content { namespace localization {

inline UINT LocalizationLegacyCodePage(){
#ifdef _LANGUAGE_BRAZIL
 return 1252u;
#else
 return CP_ACP;
#endif
}

inline bool EncodeLocalizationUtf8ForLegacy(const std::string&utf8,std::string&legacyBytes){
 legacyBytes.clear();if(utf8.empty())return true;const UINT cp=LocalizationLegacyCodePage();if(cp==CP_UTF8){legacyBytes=utf8;return true;}
 const int wc=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,utf8.c_str(),static_cast<int>(utf8.size()),0,0);if(wc<=0)return false;
 std::vector<wchar_t>w(static_cast<std::size_t>(wc));if(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,utf8.c_str(),static_cast<int>(utf8.size()),&w[0],wc)!=wc)return false;
 BOOL usedDefault=FALSE;const int bc=WideCharToMultiByte(cp,WC_NO_BEST_FIT_CHARS,&w[0],wc,0,0,0,&usedDefault);if(bc<=0||usedDefault)return false;
 legacyBytes.resize(static_cast<std::size_t>(bc));usedDefault=FALSE;
 if(WideCharToMultiByte(cp,WC_NO_BEST_FIT_CHARS,&w[0],wc,&legacyBytes[0],bc,0,&usedDefault)!=bc||usedDefault){legacyBytes.clear();return false;}return true;
}

inline bool GetLocalizationLegacyValue(const char*key,std::string&legacyBytes,std::uint32_t*flags=0){
 std::string utf8;if(!GetLocalizationValue(key,utf8,flags))return false;return EncodeLocalizationUtf8ForLegacy(utf8,legacyBytes);
}

}}} // namespace summit::content::localization
