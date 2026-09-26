#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "content/v1/common/Sha256.h"
#include "content/v1/stage_spawn/content/IStageSpawnContentSource.h"
#include "content/v1/stage_spawn/content/StageSpawnPath.h"

namespace summit
{
namespace content
{
namespace stage_spawn
{

class CompiledStageSpawnContentSource : public IStageSpawnContentSource
{
public:
    explicit CompiledStageSpawnContentSource(const char *packPath)
        : valid_(false)
    {
        if(packPath) packPath_=packPath;
        LoadPack();
    }

    bool IsValid() const { return valid_; }
    const std::string& PackPath() const { return packPath_; }
    const std::string& LastError() const { return lastError_; }

    virtual bool Load(const char *logicalPath,
                      StageSpawnDefinition &out,
                      std::string *error = 0) const
    {
        if(!valid_) return Fail(error,lastError_.empty() ? "compiled stage spawn pack is invalid" : lastError_.c_str());
        const std::string wanted=NormalizeStageSpawnPath(logicalPath);
        for(std::uint32_t index=0; index<header_.recordCount; ++index)
        {
            Record record;
            if(!ReadRecord(index,record)) return Fail(error,"compiled stage spawn record is corrupt");
            std::string stored;
            if(!ReadString(record.pathOffset,stored)) return Fail(error,"compiled stage spawn path is corrupt");
            if(stored != wanted) continue;
            StageSpawnDefinition staged;
            if(!ReadString(record.idOffset,staged.id)) return Fail(error,"compiled stage spawn id is corrupt");
            staged.logicalPath=logicalPath ? logicalPath : "";
            staged.limitMax=record.limitMax;
            staged.openInterval=record.openInterval;
            staged.intervalMilliseconds=record.intervalMilliseconds;
            staged.openLimit=record.openLimit;
            for(std::uint32_t a=0; a<record.actorCount; ++a)
            {
                Actor row;
                if(!ReadActor(record.actorStart+a,row)) return Fail(error,"compiled stage spawn actor is corrupt");
                StageSpawnActorDefinition actor;
                if(!ReadString(row.nameOffset,actor.legacyNameBytes) || !ReadString(row.idOffset,actor.monsterId)) return Fail(error,"compiled actor identity is corrupt");
                actor.weight=row.weight;
                staged.actors.push_back(actor);
            }
            for(std::uint32_t b=0; b<record.bossCount; ++b)
            {
                Boss row;
                if(!ReadBoss(record.bossStart+b,row)) return Fail(error,"compiled stage spawn boss is corrupt");
                StageSpawnBossDefinition boss;
                if(!ReadString(row.masterOffset,boss.masterLegacyNameBytes) ||
                   !ReadString(row.slaveOffset,boss.slaveLegacyNameBytes) ||
                   !ReadString(row.masterIdOffset,boss.masterMonsterId) ||
                   !ReadString(row.slaveIdOffset,boss.slaveMonsterId))
                    return Fail(error,"compiled boss identity is corrupt");
                boss.slaveCount=row.slaveCount;
                for(std::uint32_t h=0; h<row.hourCount; ++h)
                {
                    std::uint32_t hour=0;
                    if(!ReadHour(row.hourStart+h,hour)) return Fail(error,"compiled boss hour is corrupt");
                    boss.openHours.push_back(hour);
                }
                staged.bosses.push_back(boss);
            }
            out=staged;
            if(error) error->clear();
            return true;
        }
        return Fail(error,"stage spawn path is not present in compiled pack");
    }

private:
    struct Header
    {
        std::uint16_t version;
        std::uint16_t flags;
        std::uint32_t recordCount,recordOffset;
        std::uint32_t actorCount,actorOffset;
        std::uint32_t bossCount,bossOffset;
        std::uint32_t hourCount,hourOffset;
        std::uint32_t stringOffset,stringSize;
        unsigned char digest[32];
    };
    struct Record
    {
        std::uint32_t pathOffset,idOffset;
        std::int32_t limitMax,openInterval;
        std::uint32_t intervalMilliseconds;
        std::int32_t openLimit;
        std::uint32_t actorStart,actorCount,bossStart,bossCount;
    };
    struct Actor { std::uint32_t nameOffset,idOffset; std::int32_t weight; };
    struct Boss { std::uint32_t masterOffset,slaveOffset,masterIdOffset,slaveIdOffset; std::int32_t slaveCount; std::uint32_t hourStart,hourCount; };

    static std::uint16_t U16(const unsigned char *p)
    { return static_cast<std::uint16_t>(p[0] | (static_cast<std::uint16_t>(p[1])<<8)); }
    static std::uint32_t U32(const unsigned char *p)
    { return static_cast<std::uint32_t>(p[0]) | (static_cast<std::uint32_t>(p[1])<<8) | (static_cast<std::uint32_t>(p[2])<<16) | (static_cast<std::uint32_t>(p[3])<<24); }
    static std::int32_t I32(const unsigned char *p) { return static_cast<std::int32_t>(U32(p)); }

    bool Range(std::uint64_t off,std::uint64_t len) const
    { return off <= data_.size() && len <= data_.size()-static_cast<std::size_t>(off); }

    bool LoadPack()
    {
        valid_=false; data_.clear(); lastError_.clear();
        if(packPath_.empty()) { lastError_="compiled stage spawn pack path is empty"; return false; }
        FILE *fp=std::fopen(packPath_.c_str(),"rb");
        if(!fp) { lastError_="compiled stage spawn pack not found"; return false; }
        if(std::fseek(fp,0,SEEK_END)!=0) { std::fclose(fp); lastError_="cannot size compiled stage spawn pack"; return false; }
        long size=std::ftell(fp);
        if(size < 80 || std::fseek(fp,0,SEEK_SET)!=0) { std::fclose(fp); lastError_="compiled stage spawn pack is too small"; return false; }
        data_.resize(static_cast<std::size_t>(size));
        if(std::fread(&data_[0],1,data_.size(),fp)!=data_.size()) { std::fclose(fp); data_.clear(); lastError_="cannot read compiled stage spawn pack"; return false; }
        std::fclose(fp);
        const unsigned char *p=&data_[0];
        if(std::memcmp(p,"SSPB",4)!=0) { lastError_="compiled stage spawn magic mismatch"; return false; }
        header_.version=U16(p+4); header_.flags=U16(p+6);
        if(header_.version!=2) { lastError_="unsupported compiled stage spawn version"; return false; }
        header_.recordCount=U32(p+8); header_.recordOffset=U32(p+12);
        header_.actorCount=U32(p+16); header_.actorOffset=U32(p+20);
        header_.bossCount=U32(p+24); header_.bossOffset=U32(p+28);
        header_.hourCount=U32(p+32); header_.hourOffset=U32(p+36);
        header_.stringOffset=U32(p+40); header_.stringSize=U32(p+44);
        std::memcpy(header_.digest,p+48,32);
        const std::uint64_t actorOffset=80ull+static_cast<std::uint64_t>(header_.recordCount)*40ull;
        const std::uint64_t bossOffset=actorOffset+static_cast<std::uint64_t>(header_.actorCount)*12ull;
        const std::uint64_t hourOffset=bossOffset+static_cast<std::uint64_t>(header_.bossCount)*28ull;
        const std::uint64_t stringOffset=hourOffset+static_cast<std::uint64_t>(header_.hourCount)*4ull;
        const std::uint64_t end=stringOffset+header_.stringSize;
        if(header_.flags!=0 || header_.recordOffset!=80u || header_.actorOffset!=actorOffset ||
           header_.bossOffset!=bossOffset || header_.hourOffset!=hourOffset || header_.stringOffset!=stringOffset ||
           end!=data_.size())
        { lastError_="compiled stage spawn layout is non-canonical"; return false; }
        if(!Range(header_.recordOffset,static_cast<std::uint64_t>(header_.recordCount)*40u) ||
           !Range(header_.actorOffset,static_cast<std::uint64_t>(header_.actorCount)*12u) ||
           !Range(header_.bossOffset,static_cast<std::uint64_t>(header_.bossCount)*28u) ||
           !Range(header_.hourOffset,static_cast<std::uint64_t>(header_.hourCount)*4u) ||
           !Range(header_.stringOffset,header_.stringSize))
        { lastError_="compiled stage spawn section is outside file"; return false; }
        unsigned char digest[32];
        summit::content::common::Sha256(&data_[80],data_.size()-80,digest);
        if(std::memcmp(digest,header_.digest,32)!=0)
        { lastError_="compiled stage spawn SHA-256 mismatch"; return false; }
        valid_=true; return true;
    }

    bool ReadRecord(std::uint32_t index,Record &r) const
    {
        if(index>=header_.recordCount) return false;
        const std::uint64_t off=header_.recordOffset+static_cast<std::uint64_t>(index)*40u;
        if(!Range(off,40)) return false;
        const unsigned char *p=&data_[static_cast<std::size_t>(off)];
        r.pathOffset=U32(p); r.idOffset=U32(p+4); r.limitMax=I32(p+8); r.openInterval=I32(p+12);
        r.intervalMilliseconds=U32(p+16); r.openLimit=I32(p+20); r.actorStart=U32(p+24); r.actorCount=U32(p+28); r.bossStart=U32(p+32); r.bossCount=U32(p+36);
        return r.actorStart<=header_.actorCount && r.actorCount<=header_.actorCount-r.actorStart && r.bossStart<=header_.bossCount && r.bossCount<=header_.bossCount-r.bossStart;
    }
    bool ReadActor(std::uint32_t index,Actor &r) const
    {
        if(index>=header_.actorCount) return false;
        const std::uint64_t off=header_.actorOffset+static_cast<std::uint64_t>(index)*12u;
        if(!Range(off,12)) return false;
        const unsigned char *p=&data_[static_cast<std::size_t>(off)];
        r.nameOffset=U32(p); r.idOffset=U32(p+4); r.weight=I32(p+8); return true;
    }
    bool ReadBoss(std::uint32_t index,Boss &r) const
    {
        if(index>=header_.bossCount) return false;
        const std::uint64_t off=header_.bossOffset+static_cast<std::uint64_t>(index)*28u;
        if(!Range(off,28)) return false;
        const unsigned char *p=&data_[static_cast<std::size_t>(off)];
        r.masterOffset=U32(p);r.slaveOffset=U32(p+4);r.masterIdOffset=U32(p+8);r.slaveIdOffset=U32(p+12);r.slaveCount=I32(p+16);r.hourStart=U32(p+20);r.hourCount=U32(p+24);
        return r.hourStart<=header_.hourCount && r.hourCount<=header_.hourCount-r.hourStart;
    }
    bool ReadHour(std::uint32_t index,std::uint32_t &hour) const
    {
        if(index>=header_.hourCount) return false;
        const std::uint64_t off=header_.hourOffset+static_cast<std::uint64_t>(index)*4u;
        if(!Range(off,4)) return false;
        hour=U32(&data_[static_cast<std::size_t>(off)]); return true;
    }
    bool ReadString(std::uint32_t offset,std::string &out) const
    {
        if(offset>=header_.stringSize) return false;
        std::size_t pos=static_cast<std::size_t>(header_.stringOffset)+offset;
        const std::size_t end=static_cast<std::size_t>(header_.stringOffset)+header_.stringSize;
        std::size_t stop=pos; while(stop<end && data_[stop]!=0) ++stop;
        if(stop>=end) return false;
        out.assign(reinterpret_cast<const char*>(&data_[pos]),stop-pos); return true;
    }
    static bool Fail(std::string *error,const char *message) { if(error) *error=message?message:"compiled stage spawn load failed"; return false; }

    std::string packPath_;
    mutable std::string lastError_;
    std::vector<unsigned char> data_;
    Header header_={};
    bool valid_;
};

} // namespace stage_spawn
} // namespace content
} // namespace summit
