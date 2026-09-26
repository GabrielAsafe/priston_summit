#include <cstdint>
#include <cstring>
#define rsSTG_MONSTER_MAX 64
#define rsBOSS_MONSTER_MAX 16
typedef unsigned char BYTE;
struct smCHAR_INFO { char szName[64]; int marker; };
struct rsSTG_MONSTER { char szMonsterName[64]; smCHAR_INFO *lpCharInfo; int NumOpenStart; int OpenPercentage; };
struct sBOSS_MONSTER { smCHAR_INFO *MasterMonster; smCHAR_INFO *SlaveMonster; int SlaveCount; BYTE bOpenTime[32]; int OpenTimeCount; };
struct rsSTG_MONSTER_LIST { int OpenInterval; int OpenLimit; int LimitMax; std::uint32_t dwIntervalTime; rsSTG_MONSTER rsMonster[rsSTG_MONSTER_MAX]; sBOSS_MONSTER sBossMonsters[rsBOSS_MONSTER_MAX]; int BossMonsterCount; int Counter; int PecetageCount; };
#include "content/v1/stage_spawn/legacy/LegacyStageSpawnAdapter.h"
#include <cassert>
#include <iostream>
int main(int argc,char **argv)
{
    if(argc!=3) return 2;
    smCHAR_INFO chars[2] = {};
    std::strcpy(chars[0].szName,"Bargon");
    summit::content::stage_spawn::CompiledStageSpawnContentSource src(argv[1]);
    assert(src.IsValid());
    summit::content::stage_spawn::StageSpawnDefinition def;
    std::string err;
    assert(src.Load(argv[2],def,&err));
    rsSTG_MONSTER_LIST out={};
    assert(summit::content::stage_spawn::MaterializeLegacyStageSpawn(def,out,chars,2,&err));
    std::cout << "PASS bridge defined=" << def.actors.size() << " resolved=" << out.Counter << "\n";
}
