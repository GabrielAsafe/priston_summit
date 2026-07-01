#pragma once
#include <Windows.h>

#include "sinbaram/sinLinkHeader.h"
#include "Database/Database.h"

#define	smTRANSCODE_OPEN_RANK		 0x49470046
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
	void                RecvRankclassToServer(rsPLAYINFO* Player, Rank_Job* infos);
	void                SendRankclassToGame(rsPLAYINFO* Player);
	int	ranktype;
	int	rankjob_code;
};

