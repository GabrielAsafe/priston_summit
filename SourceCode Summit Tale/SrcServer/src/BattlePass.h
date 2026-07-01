#pragma once
#include <Windows.h>


#define smTRANSCODE_OPEN_BATTLEPASS   0x49470023
#define BP_PACKET_SEND_ITEM_GAME   0x49470024
#define BP_PACKET_SEND_ITEM_SERVER 0x49470025
#define BP_PACKET_SEND_BPINFO_GAME   0x49470026

struct BP_INFO
{
	int size;
	int code;
	int exp;
	int lvl;
	int itemResg[41];
	int type;
};

struct BP_ITEM
{
	int  Tag;
	char Name[32];
	char itCode[32];
	int  Level_Req;
	int Item_Qtd;
	int w;
	int h;
	int spec;
	LPDIRECTDRAWSURFACE4 Image;
};

struct BP_PACK
{
	int size;
	int code;
	BP_ITEM item[70];
};

struct BP_RESGATE
{
	int size;
	int code;
	BP_ITEM item;
};


class BattlePass
{
public:
	static              BattlePass*	GetInstance() { static BattlePass instance; return &instance; }
	void                SendItemToClient(rsPLAYINFO * Player);
	void                SendBPInfoToGame(rsPLAYINFO * Player);
	void                RecvItemToClient(rsPLAYINFO * Player, BP_RESGATE * Shop);
	void				AddExpBattlePass(rsPLAYINFO * Player, int valor);
};

