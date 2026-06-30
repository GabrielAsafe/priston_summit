#pragma once
#include <Windows.h>


#define smTRANSCODE_OPEN_JOBS  0x49470040
#define JOBS_PACKET_SEND_MARKET   0x49470041
#define JOBS_PACKET_SEND_SELL_SERVER   0x49470042
#define JOBS_PACKET_SEND_ITEM_SERVER 0x49470043
#define JOBS_PACKET_SEND_BPINFO_GAME   0x49470044
#define JOBS_PACKET_SEND_BUY_SERVER   0x49470045

struct JOBS_INFO
{
	int size;
	int code;
	int exp;
	int VTNClvl;
	int Job;
	int m_a;
	int m_f;
	int m_l;
	int m_m;
	int m_r;
	int itemResg[41];
};

struct JOBS_ITEM
{
	char ID[32];
	char Name[32];
	int  mine_type;
	int  mine_qtd;
	int  mine_price;
	int  checksum;
	int  job_code;
	int  Forje_code;
	int	 job_auxmine;
	int	 job_contsellqtd;
	int	 job_contsellprice;
};

struct JOBS_PACK
{
	int size;
	int code;
	JOBS_ITEM item[70];
};

struct JOBS_RESGATE
{
	int size;
	int code;
	JOBS_ITEM item;
};



class JOBS
{
public:
	static              JOBS* GetInstance() { static JOBS instance; return &instance; }
	void                SendJOBInfoToGame(rsPLAYINFO* Player);
	void                SendJOBmarketToGame(rsPLAYINFO* Player);
	void				RecvJOBItemToClient(rsPLAYINFO* Player, JOBS_RESGATE* item);
	void                RecvJOBbuyToClient(rsPLAYINFO* Player, JOBS_PACK* Buy);
	void				RecvJOBsellToClient(rsPLAYINFO* Player, JOBS_RESGATE* infos);
	void				AddExpJOBS(rsPLAYINFO* Player, int valor);
	void				AddminJOBS(rsPLAYINFO* Player, int valor, int type);

};

#pragma once
