#pragma once

#include <windows.h>

#include "gameplay/core/v2/GameplayIds.h"
#include <vector>

#define MAX_HG_PLAYER 8
#define HG_WAIT_TIME 3*60*1000
#define HG_RUN_TIME 45*60*1000

class cHELLSGATE
{
public:
	cHELLSGATE();

	void Main();
	void SendOpenNpc(gameplay::PlayerId player);
	void RequestJoin(gameplay::PlayerId player);
	void MonsterKilled(gameplay::EntityId monster);

private:
	DWORD dwLastHgHour;
	DWORD dwTickWaiting;
	DWORD dwTick;
	int iState;
	gameplay::EntityId bossId;

	std::vector<gameplay::PlayerId> vPlayers;

	void StartEvent();
	void SpawnBoss();
	void EndEvent(bool bKilled = false);
	void UpdateRunning();
	void UpdateWaiting();

	void SendToPlayers(char* psPacket, int size);
	void SendMessageToPlayers(char* szMessage, bool bToAll=false);

	void SendReward(gameplay::PlayerId player);
	void SendLeaveArena(gameplay::PlayerId player);
};

extern cHELLSGATE cHg;


