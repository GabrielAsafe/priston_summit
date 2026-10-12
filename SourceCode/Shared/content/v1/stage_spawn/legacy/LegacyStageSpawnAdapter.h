#pragma once

#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cstdio>
#include <string>

#include "content/v1/stage_spawn/content/CompiledStageSpawnContentSource.h"
#include "content/v1/field_runtime/content/CompiledFieldRuntimeSource.h"
#include "content/v1/stage_spawn/content/LegacyStageSpawnContentSource.h"
#include "content/v1/stage_spawn/content/StageSpawnService.h"
#include "content/v1/text_resource/legacy/LegacyContentIdRegistry.h"
#include "content/v1/runtime/RuntimeContentPath.h"

namespace summit
{
namespace content
{
namespace stage_spawn
{

inline bool LegacyAsciiEqualNoCase(const char *left,const char *right)
{
    if(!left || !right) return false;
    while(*left && *right)
    {
        const unsigned char a=static_cast<unsigned char>(*left++);
        const unsigned char b=static_cast<unsigned char>(*right++);
        if(a<0x80 && b<0x80)
        {
            if(std::tolower(a)!=std::tolower(b)) return false;
        }
        else if(a!=b) return false;
    }
    return *left==0 && *right==0;
}

inline smCHAR_INFO *FindLegacyCharacterByName(smCHAR_INFO *characters,int count,const std::string &name)
{
    if(!characters || count<=0) return 0;
    for(int i=0;i<count;++i)
        if(LegacyAsciiEqualNoCase(characters[i].szName,name.c_str())) return &characters[i];
    return 0;
}


inline smCHAR_INFO *FindLegacyCharacterByContentIdOrName(smCHAR_INFO *characters,int count,const std::string &contentId,const std::string &legacyName)
{
    if(!contentId.empty())
    {
        void *bound=summit::content::text_resource::FindLegacyContentPointer(contentId);
        if(bound) return static_cast<smCHAR_INFO *>(bound);
    }
    return FindLegacyCharacterByName(characters,count,legacyName);
}

inline bool MaterializeLegacyStageSpawn(const StageSpawnDefinition &definition,
                                        rsSTG_MONSTER_LIST &output,
                                        smCHAR_INFO *characters,
                                        int characterCount,
                                        std::string *error=0)
{
    if(characterCount<0 || (characterCount>0 && !characters))
    { if(error) *error="invalid legacy character list"; return false; }

    rsSTG_MONSTER_LIST staged={};
    staged.OpenInterval=definition.openInterval;
    staged.OpenLimit=definition.openLimit;
    staged.LimitMax=definition.limitMax;
    staged.dwIntervalTime=definition.intervalMilliseconds;
    int monsterCount=0;
    int percentageTotal=0;

    for(std::size_t i=0;i<definition.actors.size();++i)
    {
        if(monsterCount>=rsSTG_MONSTER_MAX)
        { if(error) *error="stage spawn exceeds rsSTG_MONSTER_MAX"; return false; }
        const StageSpawnActorDefinition &src=definition.actors[i];
        rsSTG_MONSTER &dst=staged.rsMonster[monsterCount];
        if(src.legacyNameBytes.size()>=sizeof(dst.szMonsterName))
        { if(error) *error="stage spawn actor name exceeds legacy capacity"; return false; }
        std::memcpy(dst.szMonsterName,src.legacyNameBytes.c_str(),src.legacyNameBytes.size()+1);
        smCHAR_INFO *character=FindLegacyCharacterByContentIdOrName(characters,characterCount,src.monsterId,src.legacyNameBytes);
        if(!character)
        {
            std::printf(
                "[CONTENT][SPAWN] unresolved-actor id=%s legacy=%s\n",
                src.monsterId.c_str(),
                src.legacyNameBytes.c_str());
            std::fflush(stdout);
            continue; // exact legacy observable behavior: this slot is reused by the next actor
        }
        dst.lpCharInfo=character;
        dst.NumOpenStart=percentageTotal;
        dst.OpenPercentage=src.weight;
        percentageTotal+=src.weight;
        ++monsterCount;
    }

    for(std::size_t i=0;i<definition.bosses.size();++i)
    {
        if(staged.BossMonsterCount>=rsBOSS_MONSTER_MAX)
        { if(error) *error="stage spawn exceeds rsBOSS_MONSTER_MAX"; return false; }
        const StageSpawnBossDefinition &src=definition.bosses[i];
        sBOSS_MONSTER &dst=staged.sBossMonsters[staged.BossMonsterCount];
        dst.MasterMonster=FindLegacyCharacterByContentIdOrName(characters,characterCount,src.masterMonsterId,src.masterLegacyNameBytes);
        dst.SlaveMonster=FindLegacyCharacterByContentIdOrName(characters,characterCount,src.slaveMonsterId,src.slaveLegacyNameBytes);
        dst.SlaveCount=src.slaveCount;
        const std::size_t capacity=sizeof(dst.bOpenTime)/sizeof(dst.bOpenTime[0]);
        if(src.openHours.size()>capacity)
        { if(error) *error="boss open-hour list exceeds legacy capacity"; return false; }
        for(std::size_t h=0;h<src.openHours.size();++h)
            dst.bOpenTime[dst.OpenTimeCount++]=static_cast<BYTE>(src.openHours[h]);
        ++staged.BossMonsterCount;
    }

    staged.Counter=monsterCount;
    staged.PecetageCount=percentageTotal;
    output=staged;
    std::printf(
        "[CONTENT][SPAWN] materialized defined=%u resolved=%d weight=%d bosses=%d\n",
        static_cast<unsigned int>(definition.actors.size()),
        monsterCount,
        percentageTotal,
        staged.BossMonsterCount);
    std::fflush(stdout);
    if(error) error->clear();
    return true;
}

inline bool DecodeOpenMonsterViaContent(const char *sourceFile,
                                        rsSTG_MONSTER_LIST *output,
                                        smCHAR_INFO *characters,
                                        int characterCount)
{
    if(!sourceFile || !output) return false;
    static const std::string packPath=summit::content::runtime::ResolveRuntimePackPath("field_runtime.sfld");
    StageSpawnDefinition definition;
    std::string error;
    summit::content::field_runtime::CompiledFieldRuntimeSource field(packPath.c_str());
    if(!field.IsValid())
    {
        std::printf("[CONTENT][SPAWN] field-pack-invalid path=%s error=%s\n",packPath.c_str(),field.LastError().c_str());
        std::fflush(stdout);
        return false;
    }
    std::vector<unsigned char> spawnSection;
    if(!field.LoadStageSpawnSection(spawnSection,&error))
    {
        std::printf("[CONTENT][SPAWN] section-load-fail source=%s error=%s\n",sourceFile,error.c_str());
        std::fflush(stdout);
        return false;
    }
    CompiledStageSpawnContentSource compiled(spawnSection);
    if(!compiled.IsValid())
    {
        std::printf("[CONTENT][SPAWN] compiled-invalid source=%s error=%s\n",sourceFile,compiled.LastError().c_str());
        std::fflush(stdout);
        return false;
    }
    StageSpawnService service(compiled);
    if(!service.Load(sourceFile,definition,&error))
    {
        std::printf("[CONTENT][SPAWN] definition-load-fail source=%s error=%s\n",sourceFile,error.c_str());
        std::fflush(stdout);
        return false;
    }
    const bool ok=MaterializeLegacyStageSpawn(definition,*output,characters,characterCount,&error);
    if(!ok)
    {
        std::printf("[CONTENT][SPAWN] materialize-fail source=%s error=%s\n",sourceFile,error.c_str());
        std::fflush(stdout);
    }
    return ok;
}

} // namespace stage_spawn
} // namespace content
} // namespace summit
