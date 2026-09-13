#pragma once
#include <windows.h>


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
	LPDIRECT3DTEXTURE9 Image;
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


extern BP_PACK BPList;
extern BP_ITEM BPselectitem;

class BattlePass
{
public:
	static              BattlePass*	GetInstance() { static BattlePass instance; return &instance; }
	void                RecvItemToServer(BP_PACK * Shop);
	void                SendItemToServer(BP_ITEM item);
	void                RecvBPInfoToServer(BP_INFO * BattlePass);
	bool                MouseAction(int x, int y, int w, int h);
	bool                IsOpen() { return fOpen; }
	void                Open();
	void                Init();
	void                Draw();
	void                Close();
	void                Button();
	int                Exp;
	int					tipoBP;
private:
	int                Image[109];
	bool               fOpen;
	int                Scroll;
	int                Tag;
	int                Coin;
	int				   Lvl;
	int				   Itselect;
	int				   page;
	int				   itemResg[41];
};





