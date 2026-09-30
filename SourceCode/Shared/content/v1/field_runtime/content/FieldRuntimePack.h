#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "content/v1/common/Sha256.h"

namespace summit
{
namespace content
{
namespace field_runtime
{

class FieldRuntimePack
{
public:
    explicit FieldRuntimePack(const char *path) : valid_(false)
    {
        if(path) path_=path;
        Load();
    }

    bool IsValid() const { return valid_; }
    const std::string& LastError() const { return lastError_; }
    const std::string& Path() const { return path_; }

    bool GetSection(const char tag[5],std::vector<unsigned char> &out,std::uint16_t *version=0) const
    {
        out.clear();
        if(!valid_ || !tag) return false;
        for(std::uint32_t i=0;i<sectionCount_;++i)
        {
            const std::size_t off=sectionTableOffset_+static_cast<std::size_t>(i)*48u;
            if(off+48u>data_.size()) return false;
            const unsigned char *p=&data_[off];
            if(std::memcmp(p,tag,4)!=0) continue;
            const std::uint16_t sectionVersion=U16(p+4);
            const std::uint16_t flags=U16(p+6);
            const std::uint32_t sectionOffset=U32(p+8);
            const std::uint32_t sectionSize=U32(p+12);
            if(flags!=0 || !Range(sectionOffset,sectionSize)) return false;
            unsigned char digest[32];
            summit::content::common::Sha256(&data_[sectionOffset],sectionSize,digest);
            if(std::memcmp(digest,p+16,32)!=0) return false;
            out.assign(data_.begin()+sectionOffset,data_.begin()+sectionOffset+sectionSize);
            if(version) *version=sectionVersion;
            return true;
        }
        return false;
    }

private:
    static std::uint16_t U16(const unsigned char *p)
    { return static_cast<std::uint16_t>(p[0] | (static_cast<std::uint16_t>(p[1])<<8)); }
    static std::uint32_t U32(const unsigned char *p)
    { return static_cast<std::uint32_t>(p[0]) | (static_cast<std::uint32_t>(p[1])<<8) | (static_cast<std::uint32_t>(p[2])<<16) | (static_cast<std::uint32_t>(p[3])<<24); }

    bool Range(std::uint64_t off,std::uint64_t size) const
    { return off<=data_.size() && size<=data_.size()-static_cast<std::size_t>(off); }

    bool Load()
    {
        valid_=false; data_.clear(); lastError_.clear();
        if(path_.empty()) { lastError_="field runtime pack path is empty"; return false; }
        FILE *fp=std::fopen(path_.c_str(),"rb");
        if(!fp) { lastError_="field runtime pack not found"; return false; }
        if(std::fseek(fp,0,SEEK_END)!=0) { std::fclose(fp); lastError_="cannot size field runtime pack"; return false; }
        long size=std::ftell(fp);
        if(size<64 || std::fseek(fp,0,SEEK_SET)!=0) { std::fclose(fp); lastError_="field runtime pack is too small"; return false; }
        data_.resize(static_cast<std::size_t>(size));
        if(std::fread(&data_[0],1,data_.size(),fp)!=data_.size()) { std::fclose(fp); data_.clear(); lastError_="cannot read field runtime pack"; return false; }
        std::fclose(fp);
        const unsigned char *p=&data_[0];
        if(std::memcmp(p,"SFLD",4)!=0) { lastError_="field runtime magic mismatch"; return false; }
        const std::uint16_t version=U16(p+4),flags=U16(p+6);
        sectionCount_=U32(p+8); sectionTableOffset_=U32(p+12); dataOffset_=U32(p+16);
        const std::uint32_t fileSize=U32(p+20),reserved0=U32(p+24),reserved1=U32(p+28);
        if(version!=1 || flags!=0 || reserved0!=0 || reserved1!=0) { lastError_="unsupported field runtime header"; return false; }
        if(sectionTableOffset_!=64u || dataOffset_!=64u+sectionCount_*48u || fileSize!=data_.size()) { lastError_="field runtime layout is non-canonical"; return false; }
        if(!Range(sectionTableOffset_,static_cast<std::uint64_t>(sectionCount_)*48u) || dataOffset_>data_.size()) { lastError_="field runtime section table is outside file"; return false; }
        unsigned char digest[32];
        summit::content::common::Sha256(&data_[64],data_.size()-64,digest);
        if(std::memcmp(digest,p+32,32)!=0) { lastError_="field runtime SHA-256 mismatch"; return false; }
        for(std::uint32_t i=0;i<sectionCount_;++i)
        {
            const unsigned char *s=&data_[sectionTableOffset_+static_cast<std::size_t>(i)*48u];
            const std::uint32_t off=U32(s+8),len=U32(s+12);
            if(U16(s+6)!=0 || off<dataOffset_ || !Range(off,len)) { lastError_="field runtime section is outside file"; return false; }
        }
        valid_=true; return true;
    }

    std::string path_;
    std::string lastError_;
    std::vector<unsigned char> data_;
    std::uint32_t sectionCount_=0,sectionTableOffset_=0,dataOffset_=0;
    bool valid_;
};

} // namespace field_runtime
} // namespace content
} // namespace summit
