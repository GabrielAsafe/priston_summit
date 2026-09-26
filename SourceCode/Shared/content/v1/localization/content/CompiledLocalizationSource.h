#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "content/v1/common/Sha256.h"

namespace summit { namespace content { namespace localization {

enum LocalizationFlags {
    LocalizationTranslated = 1u,
    LocalizationUntranslatedFallback = 2u,
    LocalizationLegacyFallback = 4u,
    LocalizationLegacySource = 8u
};

struct LocalizationEntry {
    std::string locale;
    std::string key;
    std::string text;
    std::string source;
    std::uint32_t flags=0;
};

class CompiledLocalizationSource {
public:
    explicit CompiledLocalizationSource(const char *path):valid_(false){if(path)path_=path;Load();}
    bool IsValid()const{return valid_;}
    const std::string& LastError()const{return error_;}

    bool GetExact(const char *locale,const char *key,std::string &out,std::uint32_t *flags=0)const {
        if(!valid_||!locale||!key) return false;
        for(std::uint32_t i=0;i<h_.recordCount;++i){
            Record r;if(!ReadRecord(i,r))return false;
            std::string storedLocale,storedKey;
            if(!ReadString(r.localeOffset,storedLocale)||!ReadString(r.keyOffset,storedKey))return false;
            if(storedLocale==locale&&storedKey==key){
                if(!ReadString(r.textOffset,out))return false;
                if(flags)*flags=r.flags;
                return true;
            }
        }
        return false;
    }

    bool Get(const char *locale,const char *key,std::string &out,std::uint32_t *flags=0)const {
        if(GetExact(locale,key,out,flags)) return true;
        if(locale && std::strcmp(locale,"en")!=0) return GetExact("en",key,out,flags);
        return false;
    }

    bool Entries(const char *locale,std::vector<LocalizationEntry>&out)const {
        if(!valid_||!locale)return false;
        out.clear();
        for(std::uint32_t i=0;i<h_.recordCount;++i){
            Record r;if(!ReadRecord(i,r))return false;
            LocalizationEntry e;
            if(!ReadString(r.localeOffset,e.locale)||!ReadString(r.keyOffset,e.key)||!ReadString(r.textOffset,e.text)||!ReadString(r.sourceOffset,e.source))return false;
            if(e.locale!=locale)continue;
            e.flags=r.flags;out.push_back(e);
        }
        std::sort(out.begin(),out.end(),EntryLess);
        return true;
    }

private:
    struct Header { std::uint16_t version=0,flags=0; std::uint32_t recordCount=0,recordOffset=0,recordSize=0,stringOffset=0,stringSize=0,z0=0,z1=0,z2=0,z3=0,z4=0; unsigned char digest[32]; } h_;
    struct Record { std::uint32_t localeOffset=0,keyOffset=0,textOffset=0,sourceOffset=0,flags=0; };
    static std::uint16_t U16(const unsigned char*p){return static_cast<std::uint16_t>(p[0]|(static_cast<std::uint16_t>(p[1])<<8));}
    static std::uint32_t U32(const unsigned char*p){return static_cast<std::uint32_t>(p[0])|(static_cast<std::uint32_t>(p[1])<<8)|(static_cast<std::uint32_t>(p[2])<<16)|(static_cast<std::uint32_t>(p[3])<<24);}
    bool Range(std::uint64_t o,std::uint64_t n)const{return o<=data_.size()&&n<=data_.size()-static_cast<std::size_t>(o);}
    static bool EntryLess(const LocalizationEntry&a,const LocalizationEntry&b){return a.key<b.key;}

    bool Load(){
        valid_=false;error_.clear();data_.clear();
        FILE*f=std::fopen(path_.c_str(),"rb");if(!f){error_="localization pack not found";return false;}
        std::fseek(f,0,SEEK_END);long size=std::ftell(f);if(size<80||std::fseek(f,0,SEEK_SET)!=0){std::fclose(f);error_="localization pack too small";return false;}
        data_.resize(static_cast<std::size_t>(size));if(std::fread(&data_[0],1,data_.size(),f)!=data_.size()){std::fclose(f);error_="cannot read localization pack";return false;}std::fclose(f);
        const unsigned char*p=&data_[0];if(std::memcmp(p,"SLOC",4)!=0){error_="localization magic mismatch";return false;}
        h_.version=U16(p+4);h_.flags=U16(p+6);h_.recordCount=U32(p+8);h_.recordOffset=U32(p+12);h_.recordSize=U32(p+16);h_.stringOffset=U32(p+20);h_.stringSize=U32(p+24);h_.z0=U32(p+28);h_.z1=U32(p+32);h_.z2=U32(p+36);h_.z3=U32(p+40);h_.z4=U32(p+44);std::memcpy(h_.digest,p+48,32);
        const std::uint64_t strings=80ull+static_cast<std::uint64_t>(h_.recordCount)*20ull;
        if(h_.version!=2||h_.flags||h_.recordOffset!=80||h_.recordSize!=20||h_.stringOffset!=strings||h_.stringOffset+h_.stringSize!=data_.size()||h_.z0||h_.z1||h_.z2||h_.z3||h_.z4){error_="localization layout is non-canonical";return false;}
        unsigned char digest[32];summit::content::common::Sha256(&data_[80],data_.size()-80,digest);if(std::memcmp(digest,h_.digest,32)!=0){error_="localization SHA-256 mismatch";return false;}
        for(std::uint32_t i=0;i<h_.recordCount;++i){Record r;std::string tmp;if(!ReadRecord(i,r)||!ReadString(r.localeOffset,tmp)||!ReadString(r.keyOffset,tmp)||!ReadString(r.textOffset,tmp)||!ReadString(r.sourceOffset,tmp)){error_="localization record is invalid";return false;}}
        valid_=true;return true;
    }
    bool ReadRecord(std::uint32_t i,Record&r)const{if(i>=h_.recordCount)return false;std::uint64_t o=h_.recordOffset+static_cast<std::uint64_t>(i)*20;if(!Range(o,20))return false;const unsigned char*p=&data_[static_cast<std::size_t>(o)];r.localeOffset=U32(p);r.keyOffset=U32(p+4);r.textOffset=U32(p+8);r.sourceOffset=U32(p+12);r.flags=U32(p+16);return true;}
    bool ReadString(std::uint32_t o,std::string&out)const{if(o>=h_.stringSize)return false;std::size_t b=h_.stringOffset+o,e=h_.stringOffset+h_.stringSize,p=b;while(p<e&&data_[p])++p;if(p>=e)return false;out.assign(reinterpret_cast<const char*>(&data_[b]),p-b);return true;}

    std::string path_,error_;std::vector<unsigned char>data_;bool valid_;
};

}}} // namespace summit::content::localization
