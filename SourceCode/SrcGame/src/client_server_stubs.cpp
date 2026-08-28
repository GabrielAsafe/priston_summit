// TEMP_MINGW_CLIENT_STUB
// Stubs mínimos para linkar o cliente sem módulos server-side reais.

#include <windows.h>

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
unsigned char rsHardCoreEventOpenPos[4096] = {0};

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

int rsGetSoD_AreaCode(int, int, int) { return 0; }
int rsAddServerPotion(rsPLAYINFO*, unsigned long, int) { return 0; }
int rsCheckHardCoreEventArea(int, int, int) { return 0; }
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
