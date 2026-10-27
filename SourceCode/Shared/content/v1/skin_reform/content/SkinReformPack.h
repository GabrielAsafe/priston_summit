#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "content/v1/common/Sha256.h"

namespace summit { namespace content { namespace skin_reform {

class SkinReformPack {
public:
    SkinReformPack() : valid_(false) {}
    explicit SkinReformPack(const char *path) : valid_(false) { Load(path); }

    bool IsValid() const { return valid_; }
    const std::string &LastError() const { return error_; }
    bool Lookup(std::uint32_t skin, std::uint32_t &target) const {
        if (!valid_) return false;
        std::size_t lo=0, hi=entries_.size();
        while(lo<hi) {
            const std::size_t mid=lo+(hi-lo)/2;
            if(entries_[mid].skin<skin)lo=mid+1;
            else hi=mid;
        }
        if(lo==entries_.size() || entries_[lo].skin!=skin)return false;
        target=entries_[lo].target;
        return true;
    }

    bool Load(const char *path) {
        valid_=false;
        entries_.clear();
        error_.clear();
        FILE *fp=path?std::fopen(path,"rb"):0;
        if(!fp) { error_="skin reform pack missing";return false; }
        if(std::fseek(fp,0,SEEK_END)!=0) { std::fclose(fp);error_="pack seek failed";return false; }
        const long fileSize=std::ftell(fp);
        if(fileSize<56 || fileSize>48+512*8 || std::fseek(fp,0,SEEK_SET)!=0) {
            std::fclose(fp);error_="invalid pack size";return false;
        }
        std::vector<unsigned char> data(static_cast<std::size_t>(fileSize));
        const bool ok=std::fread(&data[0],1,data.size(),fp)==data.size();
        std::fclose(fp);
        if(!ok) {error_="short pack read";return false;}
        if(std::memcmp(&data[0],"SSKM",4)!=0 || U16(&data[4])!=1 || U16(&data[6])!=0) {
            error_="invalid pack header";return false;
        }
        const std::uint32_t count=U32(&data[8]);
        const std::uint32_t dataBytes=U32(&data[12]);
        if(!count || count>512 || dataBytes!=count*8 || data.size()!=48u+dataBytes) {
            error_="invalid mapping layout";return false;
        }
        unsigned char sha[32];
        summit::content::common::Sha256(&data[48],dataBytes,sha);
        if(std::memcmp(sha,&data[16],32)!=0) {error_="skin mapping SHA-256 mismatch";return false;}
        std::uint32_t previous=0;
        for(std::uint32_t i=0;i<count;++i) {
            const std::size_t pos=48u+static_cast<std::size_t>(i)*8u;
            Entry entry={U32(&data[pos]),U32(&data[pos+4])};
            if(!entry.skin || !entry.target || entry.skin<=previous) {
                entries_.clear();error_="unordered/invalid mapping";return false;
            }
            entries_.push_back(entry);
            previous=entry.skin;
        }
        valid_=true;
        return true;
    }
private:
    struct Entry {std::uint32_t skin,target;};
    static std::uint16_t U16(const unsigned char*p) {
        return static_cast<std::uint16_t>(p[0] | (static_cast<std::uint16_t>(p[1])<<8));
    }
    static std::uint32_t U32(const unsigned char*p) {
        return static_cast<std::uint32_t>(p[0]) |
               (static_cast<std::uint32_t>(p[1])<<8) |
               (static_cast<std::uint32_t>(p[2])<<16) |
               (static_cast<std::uint32_t>(p[3])<<24);
    }
    bool valid_;
    std::string error_;
    std::vector<Entry> entries_;
};

}}}
