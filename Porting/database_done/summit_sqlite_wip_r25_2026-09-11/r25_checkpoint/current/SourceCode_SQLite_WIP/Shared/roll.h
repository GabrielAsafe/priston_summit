#pragma once

#include <windows.h>
#ifdef _SERVER
#include "systems/inventory/legacy/sinItem.h"
#else
#include "systems/inventory/legacy/sinItem.h"
#endif

#ifndef STRINGCOPY
#define STRINGCOPY(dst, src) lstrcpynA((dst), (src), sizeof(dst))
#endif

#ifndef EItemID
typedef DWORD EItemID;
#endif

struct PacketRollInfo
{
    int size;
    int code;
    sITEMINFO sItemInfo;
    DWORD iRollID;
};

#define MAX_ROLLDICEDROP_TIME        15
#define ROLLDICE_DAMAGE_PERCENT      2
#define ROLLDICE_DAMAGE_COUNT        10

#define MAX_ROLLDICERANKING_USERS    10

class RollDiceRankingUser
{
public:
    char szCharacterName[32];
    UINT uNumber;

    RollDiceRankingUser(char * pszCharacterName, UINT _uNumber)
    {
        STRINGCOPY(szCharacterName, pszCharacterName);
        uNumber = _uNumber;
    }
};

struct PacketRollDiceDrop
{
    int iID;
    EItemID eItemID;
    BOOL bAcceptItem;
};

struct PacketRollDiceRanking
{
    char szCharacterName[32];
    UINT uNumber;
};
