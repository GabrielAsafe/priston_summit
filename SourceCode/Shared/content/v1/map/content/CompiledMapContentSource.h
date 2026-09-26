#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "content/v1/map/content/IMapContentSource.h"
#include "content/v1/common/Sha256.h"

namespace summit
{
namespace content
{
namespace map
{

class CompiledMapContentSource : public IMapContentSource
{
public:
    explicit CompiledMapContentSource(const char *packPath)
        : valid_(false)
    {
        if(packPath) packPath_=packPath;
        LoadPack();
    }

    bool IsValid() const { return valid_; }
    const std::string& PackPath() const { return packPath_; }
    const std::string& LastError() const { return lastError_; }

    virtual bool LoadCatalog(MapCatalogDefinition &out,std::string *error=0) const
    {
        if(!valid_) return Fail(error,lastError_.empty()?"compiled map pack is invalid":lastError_.c_str());
        MapCatalogDefinition staged;
        staged.profile=static_cast<MapProfile>(header_.profile);
        staged.featureMask=header_.featureMask;
        staged.dynamicFieldCode=static_cast<std::int32_t>(header_.dynamicFieldCode);
        staged.staticFieldCount=header_.staticFieldCount;
        staged.maps.reserve(header_.mapCount);
        for(std::uint32_t i=0;i<header_.mapCount;++i)
        {
            MapDefinition map;
            if(!ReadMap(i,map,error)) return false;
            staged.maps.push_back(map);
        }
        out=staged;
        if(error) error->clear();
        return true;
    }

    virtual bool LoadField(std::int32_t legacyFieldCode,MapDefinition &out,std::string *error=0) const
    {
        if(!valid_) return Fail(error,lastError_.empty()?"compiled map pack is invalid":lastError_.c_str());
        for(std::uint32_t i=0;i<header_.mapCount;++i)
        {
            const std::uint64_t off=header_.mapOffset+static_cast<std::uint64_t>(i)*112u;
            if(!Range(off,112)) return Fail(error,"compiled map record is outside file");
            const unsigned char *p=&data_[static_cast<std::size_t>(off)];
            if(I32(p)!=legacyFieldCode) continue;
            return ReadMap(i,out,error);
        }
        return Fail(error,"legacy field code is not present in compiled map pack");
    }

private:
    static const std::uint32_t kNullStringOffset=0xFFFFFFFFu;

    struct Header
    {
        std::uint32_t profile,flags;
        std::uint32_t mapCount,mapOffset;
        std::uint32_t stageObjectCount,stageObjectOffset;
        std::uint32_t startPointCount,startPointOffset;
        std::uint32_t gateCount,gateOffset;
        std::uint32_t warpGateCount,warpGateOffset;
        std::uint32_t warpOutCount,warpOutOffset;
        std::uint32_t ambientCount,ambientOffset;
        std::uint32_t stringOffset,stringSize;
        std::uint32_t dynamicFieldCode,staticFieldCount;
        std::uint32_t featureMask,reserved;
    };

    static std::uint16_t U16(const unsigned char *p)
    { return static_cast<std::uint16_t>(p[0] | (static_cast<std::uint16_t>(p[1])<<8)); }
    static std::uint32_t U32(const unsigned char *p)
    { return static_cast<std::uint32_t>(p[0]) | (static_cast<std::uint32_t>(p[1])<<8) | (static_cast<std::uint32_t>(p[2])<<16) | (static_cast<std::uint32_t>(p[3])<<24); }
    static std::int32_t I32(const unsigned char *p) { return static_cast<std::int32_t>(U32(p)); }

    bool Range(std::uint64_t off,std::uint64_t len) const
    { return off<=data_.size() && len<=data_.size()-static_cast<std::size_t>(off); }

    bool LoadPack()
    {
        valid_=false; data_.clear(); lastError_.clear();
        if(packPath_.empty()) { lastError_="compiled map pack path is empty"; return false; }
        FILE *fp=std::fopen(packPath_.c_str(),"rb");
        if(!fp) { lastError_="compiled map pack not found"; return false; }
        if(std::fseek(fp,0,SEEK_END)!=0) { std::fclose(fp); lastError_="cannot size compiled map pack"; return false; }
        long size=std::ftell(fp);
        if(size<128 || std::fseek(fp,0,SEEK_SET)!=0) { std::fclose(fp); lastError_="compiled map pack is too small"; return false; }
        data_.resize(static_cast<std::size_t>(size));
        if(std::fread(&data_[0],1,data_.size(),fp)!=data_.size()) { std::fclose(fp); data_.clear(); lastError_="cannot read compiled map pack"; return false; }
        std::fclose(fp);

        const unsigned char *p=&data_[0];
        if(std::memcmp(p,"SMAP",4)!=0) { lastError_="compiled map magic mismatch"; return false; }
        if(U16(p+4)!=1 || U16(p+6)!=128) { lastError_="unsupported compiled map version/header"; return false; }
        header_.profile=U32(p+8); header_.flags=U32(p+12);
        header_.mapCount=U32(p+16); header_.mapOffset=U32(p+20);
        header_.stageObjectCount=U32(p+24); header_.stageObjectOffset=U32(p+28);
        header_.startPointCount=U32(p+32); header_.startPointOffset=U32(p+36);
        header_.gateCount=U32(p+40); header_.gateOffset=U32(p+44);
        header_.warpGateCount=U32(p+48); header_.warpGateOffset=U32(p+52);
        header_.warpOutCount=U32(p+56); header_.warpOutOffset=U32(p+60);
        header_.ambientCount=U32(p+64); header_.ambientOffset=U32(p+68);
        header_.stringOffset=U32(p+72); header_.stringSize=U32(p+76);
        header_.dynamicFieldCode=U32(p+80); header_.staticFieldCount=U32(p+84);
        header_.featureMask=U32(p+88); header_.reserved=U32(p+92);
        if(header_.profile!=MapProfileClient && header_.profile!=MapProfileServer)
        { lastError_="compiled map profile is invalid"; return false; }
        if(header_.flags!=0 || header_.reserved!=0)
        { lastError_="compiled map reserved flags are nonzero"; return false; }
        if(!Range(header_.mapOffset,static_cast<std::uint64_t>(header_.mapCount)*112u) ||
           !Range(header_.stageObjectOffset,static_cast<std::uint64_t>(header_.stageObjectCount)*16u) ||
           !Range(header_.startPointOffset,static_cast<std::uint64_t>(header_.startPointCount)*8u) ||
           !Range(header_.gateOffset,static_cast<std::uint64_t>(header_.gateCount)*16u) ||
           !Range(header_.warpGateOffset,static_cast<std::uint64_t>(header_.warpGateCount)*36u) ||
           !Range(header_.warpOutOffset,static_cast<std::uint64_t>(header_.warpOutCount)*16u) ||
           !Range(header_.ambientOffset,static_cast<std::uint64_t>(header_.ambientCount)*20u) ||
           !Range(header_.stringOffset,header_.stringSize))
        { lastError_="compiled map section is outside file"; return false; }

        std::uint64_t expected=128u;
        if(header_.mapOffset!=expected) { lastError_="compiled map layout is not canonical"; return false; }
        expected+=static_cast<std::uint64_t>(header_.mapCount)*112u;
        if(header_.stageObjectOffset!=expected) { lastError_="compiled map layout is not canonical"; return false; }
        expected+=static_cast<std::uint64_t>(header_.stageObjectCount)*16u;
        if(header_.startPointOffset!=expected) { lastError_="compiled map layout is not canonical"; return false; }
        expected+=static_cast<std::uint64_t>(header_.startPointCount)*8u;
        if(header_.gateOffset!=expected) { lastError_="compiled map layout is not canonical"; return false; }
        expected+=static_cast<std::uint64_t>(header_.gateCount)*16u;
        if(header_.warpGateOffset!=expected) { lastError_="compiled map layout is not canonical"; return false; }
        expected+=static_cast<std::uint64_t>(header_.warpGateCount)*36u;
        if(header_.warpOutOffset!=expected) { lastError_="compiled map layout is not canonical"; return false; }
        expected+=static_cast<std::uint64_t>(header_.warpOutCount)*16u;
        if(header_.ambientOffset!=expected) { lastError_="compiled map layout is not canonical"; return false; }
        expected+=static_cast<std::uint64_t>(header_.ambientCount)*20u;
        if(header_.stringOffset!=expected || expected+header_.stringSize!=data_.size())
        { lastError_="compiled map layout is not canonical"; return false; }

        unsigned char digest[32];
        summit::content::common::Sha256(data_.data()+128,data_.size()-128,digest);
        if(std::memcmp(digest,p+96,32)!=0)
        { lastError_="compiled map body hash mismatch"; return false; }
        valid_=true;
        return true;
    }

    bool ReadString(std::uint32_t offset,std::string &out) const
    {
        if(offset>=header_.stringSize) return false;
        std::size_t pos=static_cast<std::size_t>(header_.stringOffset)+offset;
        const std::size_t end=static_cast<std::size_t>(header_.stringOffset)+header_.stringSize;
        std::size_t stop=pos;
        while(stop<end && data_[stop]!=0) ++stop;
        if(stop>=end) return false;
        out.assign(reinterpret_cast<const char*>(&data_[pos]),stop-pos);
        return true;
    }

    bool ReadMap(std::uint32_t index,MapDefinition &out,std::string *error) const
    {
        if(index>=header_.mapCount) return Fail(error,"compiled map index is out of range");
        const std::uint64_t off=header_.mapOffset+static_cast<std::uint64_t>(index)*112u;
        if(!Range(off,112)) return Fail(error,"compiled map record is outside file");
        const unsigned char *p=&data_[static_cast<std::size_t>(off)];
        MapDefinition staged;
        staged.legacyFieldCode=I32(p);
        if(!ReadString(U32(p+4),staged.id) || !ReadString(U32(p+8),staged.stagePath))
            return Fail(error,"compiled map identity string is corrupt");
        const std::uint32_t imageOffset=U32(p+12);
        if(imageOffset!=kNullStringOffset)
        {
            if(!ReadString(imageOffset,staged.mapImageId)) return Fail(error,"compiled map image id is corrupt");
            staged.hasMapImageId=true;
        }
        staged.state=I32(p+16);
        staged.backImageCode[0]=I32(p+20); staged.backImageCode[1]=I32(p+24); staged.backImageCode[2]=I32(p+28);
        staged.backMusicCode=I32(p+32); staged.fieldEvent=I32(p+36); staged.serverCode=I32(p+40);
        staged.limitLevel=I32(p+44); staged.fieldSight=I32(p+48); staged.centerX=I32(p+52); staged.centerZ=I32(p+56);
        const std::uint32_t stageStart=U32(p+60),stageCount=U32(p+64);
        const std::uint32_t startStart=U32(p+68),startCount=U32(p+72);
        const std::uint32_t gateStart=U32(p+76),gateCount=U32(p+80);
        const std::uint32_t warpStart=U32(p+84),warpCount=U32(p+88);
        const std::uint32_t ambientStart=U32(p+92),ambientCount=U32(p+96);
        if(stageStart>header_.stageObjectCount || stageCount>header_.stageObjectCount-stageStart ||
           startStart>header_.startPointCount || startCount>header_.startPointCount-startStart ||
           gateStart>header_.gateCount || gateCount>header_.gateCount-gateStart ||
           warpStart>header_.warpGateCount || warpCount>header_.warpGateCount-warpStart ||
           ambientStart>header_.ambientCount || ambientCount>header_.ambientCount-ambientStart)
            return Fail(error,"compiled map child range is corrupt");

        staged.hasLegacyOrphanWarpLimitLevel=U32(p+100)!=0;
        staged.legacyOrphanWarpLimitLevel=I32(p+104);
        if(U32(p+108)!=0) return Fail(error,"compiled map reserved word is nonzero");

        for(std::uint32_t i=0;i<stageCount;++i)
        {
            const std::uint64_t rowOff=header_.stageObjectOffset+static_cast<std::uint64_t>(stageStart+i)*16u;
            const unsigned char *r=&data_[static_cast<std::size_t>(rowOff)];
            MapStageObjectDefinition item;
            if(!ReadString(U32(r),item.assetPath)) return Fail(error,"compiled map stage-object path is corrupt");
            item.bipAnimation=I32(r+4); item.requireFeatureMask=U32(r+8); item.forbidFeatureMask=U32(r+12);
            staged.stageObjects.push_back(item);
        }
        for(std::uint32_t i=0;i<startCount;++i)
        {
            const unsigned char *r=&data_[static_cast<std::size_t>(header_.startPointOffset+static_cast<std::uint64_t>(startStart+i)*8u)];
            MapStartPointDefinition item; item.x=I32(r); item.z=I32(r+4); staged.startPoints.push_back(item);
        }
        for(std::uint32_t i=0;i<gateCount;++i)
        {
            MapGateDefinition item;
            if(!ReadGate(header_.gateOffset,gateStart+i,item)) return Fail(error,"compiled map gate is corrupt");
            staged.gates.push_back(item);
        }
        for(std::uint32_t i=0;i<warpCount;++i)
        {
            const std::uint64_t rowOff=header_.warpGateOffset+static_cast<std::uint64_t>(warpStart+i)*36u;
            const unsigned char *r=&data_[static_cast<std::size_t>(rowOff)];
            MapWarpGateDefinition item;
            item.x=I32(r); item.z=I32(r+4); item.y=I32(r+8); item.size=I32(r+12); item.height=I32(r+16);
            item.limitLevel=I32(r+20); item.specialEffect=I32(r+24);
            const std::uint32_t outStart=U32(r+28),outCount=U32(r+32);
            if(outStart>header_.warpOutCount || outCount>header_.warpOutCount-outStart)
                return Fail(error,"compiled map warp-out range is corrupt");
            for(std::uint32_t g=0;g<outCount;++g)
            {
                MapGateDefinition outGate;
                if(!ReadGate(header_.warpOutOffset,outStart+g,outGate)) return Fail(error,"compiled map warp-out gate is corrupt");
                item.outGates.push_back(outGate);
            }
            staged.warpGates.push_back(item);
        }
        for(std::uint32_t i=0;i<ambientCount;++i)
        {
            const unsigned char *r=&data_[static_cast<std::size_t>(header_.ambientOffset+static_cast<std::uint64_t>(ambientStart+i)*20u)];
            MapAmbientDefinition item;
            item.x=I32(r); item.y=I32(r+4); item.z=I32(r+8); item.radius=I32(r+12); item.soundCode=I32(r+16);
            staged.ambient.push_back(item);
        }

        out=staged;
        if(error) error->clear();
        return true;
    }

    bool ReadGate(std::uint32_t sectionOffset,std::uint32_t index,MapGateDefinition &out) const
    {
        const std::uint64_t off=sectionOffset+static_cast<std::uint64_t>(index)*16u;
        if(!Range(off,16)) return false;
        const unsigned char *p=&data_[static_cast<std::size_t>(off)];
        out.targetLegacyFieldCode=I32(p); out.x=I32(p+4); out.z=I32(p+8); out.y=I32(p+12);
        return true;
    }

    static bool Fail(std::string *error,const char *message)
    { if(error) *error=message?message:"compiled map load failed"; return false; }

    std::string packPath_;
    bool valid_;
    std::string lastError_;
    std::vector<unsigned char> data_;
    Header header_;
};

} // namespace map
} // namespace content
} // namespace summit
