#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>

#define rsSTG_MONSTER_MAX 64
#define rsBOSS_MONSTER_MAX 16
typedef unsigned char BYTE;
struct smCHAR_INFO { char szName[64]; int marker; };
struct rsSTG_MONSTER { char szMonsterName[64]; smCHAR_INFO *lpCharInfo; int NumOpenStart; int OpenPercentage; };
struct sBOSS_MONSTER { smCHAR_INFO *MasterMonster; smCHAR_INFO *SlaveMonster; int SlaveCount; BYTE bOpenTime[32]; int OpenTimeCount; };
struct rsSTG_MONSTER_LIST { int OpenInterval; int OpenLimit; int LimitMax; std::uint32_t dwIntervalTime; rsSTG_MONSTER rsMonster[rsSTG_MONSTER_MAX]; sBOSS_MONSTER sBossMonsters[rsBOSS_MONSTER_MAX]; int BossMonsterCount; int Counter; int PecetageCount; };

#include "content/v1/stage_spawn/legacy/LegacyStageSpawnAdapter.h"

static int Fail(const char *message)
{
    std::cerr << "FAIL: " << message << "\n";
    return 1;
}

int main(int argc, char **argv)
{
    if(argc != 2) return Fail("usage SSPB");
    const std::filesystem::path pack = std::filesystem::absolute(argv[1]);
    setenv("SUMMIT_STAGE_SPAWN_PACK", pack.string().c_str(), 1);
    setenv("SUMMIT_STAGE_SPAWN_STRICT", "1", 1);

    smCHAR_INFO characters[1] = {};
    std::strcpy(characters[0].szName, "Bargon");
    rsSTG_MONSTER_LIST output = {};

    // Deliberately use a logical legacy path that does not exist in the current
    // working directory. Strict mode must succeed from SSPB without filesystem fallback.
    if(!summit::content::stage_spawn::DecodeOpenMonsterViaContent(
           "GameServer/Field/dun-3.ase.spm", &output, characters, 1))
        return Fail("strict SSPB decode without legacy filesystem");

    if(output.OpenInterval == 0 && output.LimitMax == 0 && output.Counter == 0 && output.BossMonsterCount == 0)
        return Fail("decoded stage spawn is unexpectedly empty");

    std::cout << "PASS strict compiled stage spawn without legacy filesystem reads\n";
    return 0;
}
