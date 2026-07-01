#pragma once

#define MAX_HG_PLAYER 8
#define HG_WAIT_TIME 3*60*1000 
#define HG_RUN_TIME 45*60*1000

struct sHGPLAYER
{
	char szName[32];
	DWORD dwObjectSerial;
};

class cHELLSGATE
{
public:
	cHELLSGATE();

	void Main();
	void SendOpenNpc(rsPLAYINFO* lpPlayInfo);
	void RecvJoinHg(rsPLAYINFO* lpPlayInfo);
	void MonsterKilled(smCHAR* lpChar);

private:
	DWORD dwLastHgHour;
	DWORD dwTickWaiting;
	DWORD dwTick;
	int iState;
	smCHAR* lpBoss;

	vector<sHGPLAYER> vPlayers;

	void StartEvent();
	void SpawnBoss();
	void EndEvent(bool bKilled = false);
	void UpdateRunning();
	void UpdateWaiting();

	void SendToPlayers(char* psPacket, int size);
	void SendMessageToPlayers(char* szMessage, bool bToAll=false);

	void SendReward(rsPLAYINFO* lpPlayInfo);
	void SendLeaveArena(rsPLAYINFO* lpPlayInfo);
};

extern cHELLSGATE cHg;