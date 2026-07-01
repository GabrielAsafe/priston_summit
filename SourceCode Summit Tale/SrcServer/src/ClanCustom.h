#pragma once
#include <Windows.h>


#define smTRANSCODE_OPEN_CLANCUSTOM   0x49470012
#define CCPACKET_SEND_ITEM_GAME   0x49470013
#define CCPACKET_SEND_ITEM_SERVER 0x49470014
#define CCPACKET_SEND_COIN_GAME   0x49470015
#define CCPACKET_SEND_CROWN_GAME   0x49470016

struct CCSHOP_ITEM
{
	int  SubTag;
	char Name[32];
	char itCode[32];
	int  Coin;
	int hora;
	int minuto;
	int w;
	int h;
	int dwversion;
	int combobox;
	int spec;
	LPDIRECTDRAWSURFACE4 Image;
};

struct CCSHOP
{
	int size;
	int code;
	CCSHOP_ITEM item[10];
	CCSHOP_ITEM item2[50];
};

struct CCSENDMONEY
{
	int size;
	int code;
	int money;
};

struct CCSHOP_BUY
{
	int size;
	int code;
	CCSHOP_ITEM item;
	CCSHOP_ITEM item2;
};

struct CROWN
{
	int size;
	int code;
	int idnumber;
};

extern CCSHOP CList;
extern CCSHOP_ITEM Cselectitem;

class ClanCustom
{
public:
	static              ClanCustom*	GetInstance() { static ClanCustom instance; return &instance; }
	void                SendItemToClient(rsPLAYINFO * Player);
	void                RecvItemToClient(rsPLAYINFO * Player, CCSHOP_BUY * Shop);
	void				SendCrownToClient(rsPLAYINFO * Player);
};




