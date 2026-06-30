#pragma once
#include <Windows.h>

#define	smTRANSCODE_OPEN_RANK		0x49470046
#define RANK_PACKET_SEND_RANKCLASS   0x49470047
#define RANK_PACKET_RECV_RANKCLASS   0x49470048


struct Rank_Class
{
	char Name[32];
	int  Job_code;
	int  Level;
	int  Kills;
};

struct Rank_PACK
{
	int size;
	int code;
	Rank_Class infos[100];
};

struct Rank_Job
{
	int size;
	int code;
	int type;
	int job_code;
};


extern Rank_PACK RankList;
extern Rank_Class Rselectitem;

class Rank
{
public:
	static              Rank* GetInstance() { static Rank instance; return &instance; }
	void                SendRankclassToServer( int type, int Job_code);
	void                RecvRankclassToGame(Rank_PACK* infos);
	bool                MouseAction(int x, int y, int w, int h);
	bool                IsOpen() { return fOpen; }
	void                Open();
	void                Init();
	void                Draw();
	void                Close();
	void                Button();
	int					Exp;
	bool				 fOpen;
	int					 page;
	int					SubTag;
	int					SubTag2;
	int					classtype;
	int					classtype2;
	int					ranktypevar;
	int					rankjobvar;
private:
	int                Image[109];
	int                Scroll;
	int                Tag;
	int                Coin;
	int				   itemResg[41];
};


