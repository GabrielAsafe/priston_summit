// TEMP_MINGW_CLIENT_STUB
// Stubs mínimos para linkar o cliente sem módulos server-side reais.

#include <windows.h>
#define FLOATNS 8

struct POINT3D
{
    int x, y, z;
};

struct rsPLAYINFO;
struct smCHAR;
struct smWINSOCK;
struct smTRANS_COMMAND;

int ServerMode = 0;
int AgingEvento = 0;
int BabelPlayField = 0;
DWORD dwBabelPlayTime = 0;
DWORD dwPlayServTime = 0;
int Server_DebugCount = 0;
long tServerTime = 0;

unsigned char rsBlessCastle[4096] = {0};
POINT3D rsHardCoreEventOpenPos[7][3] = {
    {
        { 18967, 1154, 3055 },
        { 19995, 1154, 2134 },
        { 19485, 1154, 2615 }
    },
    {
        { 20838, 1154, 3024 },
        { 21671, 1154, 2193 },
        { 21255, 1154, 2615 }
    },
    {
        { 22478, 1154, 3049 },
        { 23437, 1154, 2135 },
        { 22948, 1154, 2615 }
    },
    {
        { 24204, 1154, 3094 },
        { 25173, 1154, 2135 },
        { 24674, 1154, 2615 }
    },
    {
        { 20509, 1621, 5284 },
        { 21544, 1621, 4261 },
        { 21012, 1621, 4773 }
    },
    {
        { 22575, 1621, 5253 },
        { 23610, 1621, 4230 },
        { 23077, 1621, 4742 }
    },
    {
        { 21538, 1638, 7057 },
        { 22527, 1638, 6079 },
        { 22018, 1638, 6569 }
    }
};

int rsHardCoreEventArea[4] = { 18516, 25715, 1541, 7178 };

DWORD GetClanCode(int ClassClanCode)
{
    DWORD dwCode = ClassClanCode % 1000000;
    DWORD dwCode1 = ClassClanCode / 100000000;
    DWORD dwCode2 = dwCode1 * 100000000;

    return dwCode + dwCode2;
}

int rsProcessQuest(rsPLAYINFO*, smCHAR*, int, int) { return 0; }
smCHAR* rsGetLinkChar(smCHAR*) { return 0; }
int rsGetMonsterLifeRegen(smCHAR*) { return 0; }
int rsSkillDamage_AroundMonster(smCHAR*) { return 0; }
int rsGetMonCountFlag(smCHAR*) { return 0; }
smCHAR* rsFindAroundMonster(smCHAR*, int) { return 0; }
int rsSkillMonster_AroundMonster(smCHAR*, int) { return 0; }

int OpenMonsterFromSkill(unsigned long, int, int, int, rsPLAYINFO*, smCHAR*) { return 0; }
int rsProcessAttack_SkillHolyReflection(rsPLAYINFO*, smCHAR*) { return 0; }
int rsOpenSubMonster(smCHAR*, unsigned long, int, int, int, int) { return 0; }
int rsSendRangeDamage(smCHAR*, rsPLAYINFO*, int, int) { return 0; }
int rsSendCommandUser2(rsPLAYINFO*, smTRANS_COMMAND*) { return 0; }
int rsSendTransSkillAttack(smCHAR*, rsPLAYINFO*) { return 0; }

char* rsGetWord(char*, char*) { return 0; }

int Serv_ConnectPlayer(smWINSOCK*) { return 0; }
int Serv_DisconnectPlayer(smWINSOCK*) { return 0; }
int Serv_CheckOtherPlayPosi(int, int, int) { return 0; }

int rsGetSoD_AreaCode(int x, int y, int z)
{
    int mx = x >> FLOATNS;
    int mz = z >> FLOATNS;

    if (y >= rsHardCoreEventOpenPos[0][0].y - 32)
    {
        if (rsHardCoreEventArea[0] < mx &&
            rsHardCoreEventArea[1] > mx &&
            rsHardCoreEventArea[2] < mz &&
            rsHardCoreEventArea[3] > mz)
        {
            for (int cnt = 0; cnt < 7; cnt++)
            {
                if (mx > rsHardCoreEventOpenPos[cnt][0].x &&
                    mx < rsHardCoreEventOpenPos[cnt][1].x &&
                    mz > rsHardCoreEventOpenPos[cnt][1].z &&
                    mz < rsHardCoreEventOpenPos[cnt][0].z)
                {
                    return cnt;
                }
            }
        }
    }

    return -1;
}
int rsAddServerPotion(rsPLAYINFO*, unsigned long, int) { return 0; }
int rsCheckHardCoreEventArea(int x, int y, int z)
{
    int mx = x >> FLOATNS;
    int mz = z >> FLOATNS;

    if (y >= rsHardCoreEventOpenPos[0][0].y - 32)
    {
        if (rsHardCoreEventArea[0] < mx &&
            rsHardCoreEventArea[1] > mx &&
            rsHardCoreEventArea[2] < mz &&
            rsHardCoreEventArea[3] > mz)
        {
            return TRUE;
        }
    }

    return FALSE;
}
int rsUpdateServerParam(rsPLAYINFO*, unsigned long, int, int, int) { return 0; }
int rsGetTradePotionInfo(rsPLAYINFO*, unsigned long) { return 0; }
int rsPutItem_PointTicket(rsPLAYINFO*, int) { return 0; }

int GetNewObjectSerial() { return 1; }

void RecordHackLogFile(rsPLAYINFO*, void*) {}
void RecordDeleteCharacterError(const char*, char*) {}

class rsPLAYINFO_AddServerMoney_Stub {};

struct rsPLAYINFO {
    int AddServerMoney(int, int);
};

int rsPLAYINFO::AddServerMoney(int, int) {
    return 0;
}
