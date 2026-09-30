#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "content/v1/field_runtime/content/FieldRuntimePack.h"
#include "content/v1/field_runtime/domain/FieldPlacementDefinition.h"
#include "content/v1/stage_spawn/content/StageSpawnPath.h"

namespace summit { namespace content { namespace field_runtime {

class CompiledFieldRuntimeSource
{
public:
    explicit CompiledFieldRuntimeSource(const char *packPath) : pack_(packPath) {}
    bool IsValid() const { return pack_.IsValid(); }
    const std::string& LastError() const { return pack_.LastError(); }

    bool LoadStartPoints(const char *logicalPath,StageStartPointSet &out,std::string *error=0) const
    {
        std::vector<unsigned char> data; std::uint16_t version=0;
        if(!pack_.GetSection("SPPT",data,&version) || version!=1) return Fail(error,"field runtime SPPT section missing or invalid");
        return ParseStartPoints(data,logicalPath,out,error);
    }

    bool LoadFixedNpcs(const char *logicalPath,FixedNpcSet &out,std::string *error=0) const
    {
        std::vector<unsigned char> data; std::uint16_t version=0;
        if(!pack_.GetSection("SPCN",data,&version) || version!=1) return Fail(error,"field runtime SPCN section missing or invalid");
        return ParseFixedNpcs(data,logicalPath,out,error);
    }

    bool LoadStageSpawnSection(std::vector<unsigned char> &out,std::string *error=0) const
    {
        std::uint16_t version=0;
        if(!pack_.GetSection("SSPB",out,&version) || version!=2) return Fail(error,"field runtime SSPB section missing or invalid");
        if(error) error->clear(); return true;
    }

private:
    static std::uint16_t U16(const unsigned char *p){return static_cast<std::uint16_t>(p[0]|(static_cast<std::uint16_t>(p[1])<<8));}
    static std::uint32_t U32(const unsigned char *p){return static_cast<std::uint32_t>(p[0])|(static_cast<std::uint32_t>(p[1])<<8)|(static_cast<std::uint32_t>(p[2])<<16)|(static_cast<std::uint32_t>(p[3])<<24);}
    static std::int32_t I32(const unsigned char *p){return static_cast<std::int32_t>(U32(p));}
    static bool Range(const std::vector<unsigned char>&d,std::uint64_t o,std::uint64_t n){return o<=d.size()&&n<=d.size()-static_cast<std::size_t>(o);}
    static bool ReadString(const std::vector<unsigned char>&d,std::uint32_t base,std::uint32_t size,std::uint32_t offset,std::string&out)
    {
        if(offset>=size || !Range(d,base,size)) return false;
        std::size_t pos=static_cast<std::size_t>(base)+offset,end=static_cast<std::size_t>(base)+size,stop=pos;
        while(stop<end&&d[stop]!=0)++stop; if(stop>=end)return false;
        out.assign(reinterpret_cast<const char*>(&d[pos]),stop-pos); return true;
    }
    static bool ValidateHeader(const std::vector<unsigned char>&d,const char magic[5],std::uint32_t itemSize,
                               std::uint32_t&mapCount,std::uint32_t&mapOff,std::uint32_t&itemCount,std::uint32_t&itemOff,
                               std::uint32_t&stringOff,std::uint32_t&stringSize,std::string*error)
    {
        if(d.size()<64||std::memcmp(&d[0],magic,4)!=0||U16(&d[4])!=1||U16(&d[6])!=0) return Fail(error,"field runtime subsection header invalid");
        mapCount=U32(&d[8]);mapOff=U32(&d[12]);itemCount=U32(&d[16]);itemOff=U32(&d[20]);stringOff=U32(&d[24]);stringSize=U32(&d[28]);
        if(mapOff!=64u||itemOff!=mapOff+mapCount*16u||stringOff!=itemOff+itemCount*itemSize||stringOff+stringSize!=d.size()) return Fail(error,"field runtime subsection layout invalid");
        unsigned char digest[32];summit::content::common::Sha256(&d[64],d.size()-64,digest);if(std::memcmp(digest,&d[32],32)!=0)return Fail(error,"field runtime subsection SHA-256 mismatch");
        return true;
    }
    static bool ParseStartPoints(const std::vector<unsigned char>&d,const char*logical,StageStartPointSet&out,std::string*error)
    {
        std::uint32_t mc,mo,pc,po,so,ss;if(!ValidateHeader(d,"SPPT",16,mc,mo,pc,po,so,ss,error))return false;
        const std::string wanted=summit::content::stage_spawn::NormalizeStageSpawnPath(logical);
        for(std::uint32_t i=0;i<mc;++i){const unsigned char*p=&d[mo+i*16u];std::string path;if(!ReadString(d,so,ss,U32(p),path))return Fail(error,"SPPT path corrupt");if(path!=wanted)continue;
            StageStartPointSet staged;if(!ReadString(d,so,ss,U32(p+4),staged.id))return Fail(error,"SPPT id corrupt");staged.logicalPath=logical?logical:"";std::uint32_t start=U32(p+8),count=U32(p+12);if(start>pc||count>pc-start)return Fail(error,"SPPT range corrupt");
            for(std::uint32_t n=0;n<count;++n){const unsigned char*q=&d[po+(start+n)*16u];StageStartPointPlacement x;x.slot=U32(q);x.state=I32(q+4);x.x=I32(q+8);x.z=I32(q+12);staged.points.push_back(x);}out=staged;if(error)error->clear();return true;}
        return Fail(error,"SPP logical path is not present in field runtime pack");
    }
    static bool ParseFixedNpcs(const std::vector<unsigned char>&d,const char*logical,FixedNpcSet&out,std::string*error)
    {
        std::uint32_t mc,mo,nc,no,so,ss;if(!ValidateHeader(d,"SPCN",40,mc,mo,nc,no,so,ss,error))return false;
        const std::string wanted=summit::content::stage_spawn::NormalizeStageSpawnPath(logical);
        for(std::uint32_t i=0;i<mc;++i){const unsigned char*p=&d[mo+i*16u];std::string path;if(!ReadString(d,so,ss,U32(p),path))return Fail(error,"SPCN path corrupt");if(path!=wanted)continue;
            FixedNpcSet staged;if(!ReadString(d,so,ss,U32(p+4),staged.id))return Fail(error,"SPCN id corrupt");staged.logicalPath=logical?logical:"";std::uint32_t start=U32(p+8),count=U32(p+12);if(start>nc||count>nc-start)return Fail(error,"SPCN range corrupt");
            for(std::uint32_t n=0;n<count;++n){const unsigned char*q=&d[no+(start+n)*40u];FixedNpcPlacement x;x.slot=U32(q);if(!ReadString(d,so,ss,U32(q+4),x.npcId)||!ReadString(d,so,ss,U32(q+8),x.definitionPath))return Fail(error,"SPCN NPC identity corrupt");x.x=I32(q+12);x.y=I32(q+16);x.z=I32(q+20);x.ax=I32(q+24);x.ay=I32(q+28);x.az=I32(q+32);x.state=I32(q+36);staged.npcs.push_back(x);}out=staged;if(error)error->clear();return true;}
        return Fail(error,"SPC logical path is not present in field runtime pack");
    }
    static bool Fail(std::string*error,const char*message){if(error)*error=message?message:"field runtime load failed";return false;}
    FieldRuntimePack pack_;
};

}}}
