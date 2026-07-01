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
	JOBS_ITEM item[100];
};

struct JOBS_RESGATE
{
	int size;
	int code;
	JOBS_ITEM item;
};

struct JOBS_SELL
{
	char snameseller[32];
	char snamemine[32];
	int sjobqtd;
	int sjobprice;
	int sellday;
	int sellmon;
	int sellyear;
	int sellhour;
	int sellmin;
};

extern JOBS_PACK JOBSList;
extern JOBS_ITEM Jselectitem;

class JOBS
{
public:
	static              JOBS* GetInstance() { static JOBS instance; return &instance; }
	void                SendJOBItemToServer(char name[32], int job_code, int forje_code);
	void                SendSellJOBToServer(int minecode, int qtd, int price);
	void                RecvJOBInfoToServer(JOBS_INFO* JOBS);
	void                SendJOBmarketToServer(JOBS_ITEM item);
	void				RecvJOBmarketToServer(JOBS_PACK* itens);
	bool                MouseAction(int x, int y, int w, int h);
	bool                IsOpen() { return fOpen; }
	void                Open();
	void                Init();
	void                Draw();
	void                Close();
	void                Button();
	int                Exp;
	bool               fOpen;
	int					jobc;
	int					forgejobc;
	char				namemine[32];
	int					jobqtd;
	int					jobprice;
	int					auxmine;
	int					contsellqtd;
	int					contsellprice;
	int					auxmine3;
	int				   page;
	int					subpage;
	int					ajuste[100];
	int					aauxmine[100];
	int TESTEPOHA;
private:
	int                Image[109];
	int                Scroll;
	int                Tag;
	int                Coin;
	int				   VSFlvl;
	int					Job;
	int					m_a;
	int					m_f;
	int					m_l;
	int					m_m;
	int					m_r;
	int					auxmine2;
	int				   Itselect;
	int				   itemResg[41];
};

