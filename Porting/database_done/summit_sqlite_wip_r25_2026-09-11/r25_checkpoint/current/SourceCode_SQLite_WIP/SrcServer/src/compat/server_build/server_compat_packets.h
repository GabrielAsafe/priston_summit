#pragma once

#ifndef SUMMIT_SERVER_COMPAT_PACKETS_H
#define SUMMIT_SERVER_COMPAT_PACKETS_H

#include <windows.h>

typedef unsigned long long QWORD;


struct smTRANS_COMMAND_QWORD
{
    int size, code;
    DWORD WParam;
    DWORD LParam;
    QWORD SParam;
    QWORD EParam;
};




#define SUMMIT_SMTRANS_VIP_DEFINED
struct smTRANS_VIP
{
    int size, code;
    int Ativo;
};


#define SUMMIT_PACKET_CROWN_DEFINED
struct PACKET_CROWN
{
    int size, code;
    DWORD Silver;
    DWORD Bronze;
};




#define SUMMIT_SERVER_NETPLAY_COMPAT_TYPES

struct smRANKING
{
    int size, code;
    int type;
    int contador;
    char rankName[100][32];
    char rankNameClan[100][32];
    char rankLiderClan[100][32];
    int rankLevel[100];
    int rankClasse[100];
    int rankKills[100];
    int rankDeads[100];
    int rankPointsSod[100];
    int rankmembros[100];
};

struct TRANS_PREMIUM_TIME
{
    int size, code;
    DWORD ThirdEye;
    DWORD ExpUp;
    DWORD VampCuspid;
    DWORD ManaRecharg;
    DWORD StaminaRecharg;
    DWORD ManaReduce;
    DWORD MightofAwell;
    DWORD MightofAwell2;
    DWORD PhenixPet;
    DWORD HelpPet;
    DWORD VampCuspidEx;
    DWORD StaminaReduce;
    DWORD Caravan;
    DWORD GoldLooter;
    DWORD HPBooster;
    DWORD MPBooster;
    DWORD RESBooster;
};

#define smTRANSCODE_OPEN_NPCinfo 0x49470022


#ifndef SUMMIT_PACKET_BUFFS_DEFINED
#define SUMMIT_PACKET_BUFFS_DEFINED

struct PACKET_BUFFS
{
    int size, code;
    int NovatoLevel;
    int NovatoADD;
    int NovatoRegen;
    int AutoJobPromotion;
    int SetBoss1Bonus1;
    int SetBoss1Bonus2;
    int SetBoss2Bonus1;
    int SetBoss2Bonus2;
};

#endif

#ifndef PACKET_KILL
#define PACKET_KILL 0x4847900B
#endif

#endif
