#include <cstring>
#include "systems/network/protocol/server/opcodes.h"
#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "systems/gameplay/events/hells_gate/cHellsGate.h"

extern int SendQuickMessage2(rsPLAYINFO *lpPlayInfo, char *szMessage);
extern void SendWarpField(rsPLAYINFO* lpPlayInfo, int Field, int x, int z);
extern smCHAR *OpenMonsterFromName(char *szName, int x, int y, int z, rsPLAYINFO *lpPlayInfo = 0);
extern rsPLAYINFO *FindUserFromCode(DWORD dwUserCode);
extern int CreateCommandItem(rsPLAYINFO *lpPlayInfo, rsPLAYINFO *lpPlayInfo2, char *szItem);
extern int spJobCode;
extern int SendToAll(char* Buff, int Size);
extern int Event_HellsGate;

cHELLSGATE cHg;


cHELLSGATE::cHELLSGATE()
{
	//dwLastHgHour = 0;
	iState = 0;
	dwTickWaiting = 0;
	dwTick = 0;
}

void cHELLSGATE::Main()
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	/*if (st.wHour % 6 == 0 && Event_HellsGate == 1 != st.wMinute == 0)
	{
		if (iState == 0)
			StartEvent();
	}*/

	if (iState == 1)
	{
		if (dwTickWaiting && dwTickWaiting + HG_WAIT_TIME < GetCurrentTime())
		{
			SpawnBoss();
			SendMessageToPlayers("> O Hell's Gate come\347ou, derrote o boss e obtenha a sua recompensa.");
			return;
		}

		UpdateWaiting();
	}
	else if (iState == 2)
	{
		if (dwTick && dwTick + HG_RUN_TIME < GetCurrentTime())
		{
			EndEvent();
			return;
		}

		UpdateRunning();
	}
}

void cHELLSGATE::SendOpenNpc(rsPLAYINFO * lpPlayInfo)
{
	if (vPlayers.size() >= MAX_HG_PLAYER)
	{
		SendQuickMessage2(lpPlayInfo, "> O evento est\341 cheio.");
		return;
	}

	if (iState != 1)
	{
		if (iState == 0)
			SendQuickMessage2(lpPlayInfo, "> O evento ainda n\343o abriu.");

		if (iState == 2)
			SendQuickMessage2(lpPlayInfo, "> O evento j\341 est\341 em andamento.");

		return;
	}

	smTRANS_COMMAND sPacket;
	sPacket.code = OpCode::OPCODE_OPEN_HG;
	sPacket.size = sizeof(smTRANS_COMMAND);
	lpPlayInfo->lpsmSock->Send((char*)&sPacket, sPacket.size, TRUE);
}

void cHELLSGATE::RecvJoinHg(rsPLAYINFO * lpPlayInfo)
{
	if (vPlayers.size() >= MAX_HG_PLAYER)
	{
		SendQuickMessage2(lpPlayInfo, "> O evento est\341 cheio.");
		return;
	}

	if (iState != 1)
	{
		if (iState == 0)
			SendQuickMessage2(lpPlayInfo, "> O evento ainda n\343o abriu.");

		if (iState == 2)
			SendQuickMessage2(lpPlayInfo, "> O evento j\341 est\341 em andamento.");

		return;
	}

	sHGPLAYER sPlayer;
	lstrcpy(sPlayer.szName, lpPlayInfo->getName());
	sPlayer.dwObjectSerial = lpPlayInfo->dwObjectSerial;
	vPlayers.push_back(sPlayer);

	SendWarpField(lpPlayInfo, 23, -3624, -43172);
}

void cHELLSGATE::MonsterKilled(smCHAR * lpChar)
{
	if (iState != 2)
		return;

	if (lpBoss && lpChar && lpBoss->dwObjectSerial == lpChar->dwObjectSerial)
	{
		EndEvent(true);
	}  
}

void cHELLSGATE::StartEvent()
{
	iState = 1;
	dwTick = 0;
	dwTickWaiting = GetCurrentTime();

	//SendMessageToPlayers("> O Hell's Gate est� aberto.", true);
}

void cHELLSGATE::SpawnBoss()
{
	lpBoss = OpenMonsterFromName("Esp\355rito do Mal", -3624 * fONE, 484 * fONE, -43172 * fONE);

	dwTick = GetCurrentTime();
	iState = 2;
}

void cHELLSGATE::EndEvent(bool bKilled)
{
	lpBoss = 0;
	iState = 0;
	dwTick = 0;
	dwTickWaiting = 0;

	for (auto player : vPlayers)
	{
		rsPLAYINFO* lpPlayInfo = FindUserFromCode(player.dwObjectSerial);
		if (lpPlayInfo)
		{
			if (bKilled)
				SendReward(lpPlayInfo);

			SendLeaveArena(lpPlayInfo);
		}
	}

	SendMessageToPlayers("> O Hell's Gate foi finalizado.");

	vPlayers.clear();
}

void cHELLSGATE::UpdateRunning()
{
	smTRANS_COMMAND sPacket;
	sPacket.code = OpCode::OPCODE_UPDATE_HG;
	sPacket.size = sizeof(smTRANS_COMMAND);
	sPacket.WParam = ((dwTick + HG_RUN_TIME) - GetCurrentTime()) / 1000;
	sPacket.LParam = iState;
	SendToPlayers((char*)&sPacket, sPacket.size);
}

void cHELLSGATE::UpdateWaiting()
{
	smTRANS_COMMAND sPacket;
	sPacket.code = OpCode::OPCODE_UPDATE_HG;
	sPacket.size = sizeof(smTRANS_COMMAND);
	sPacket.WParam = ((dwTickWaiting + HG_WAIT_TIME) - GetCurrentTime()) / 1000;
	sPacket.LParam = iState;
	SendToPlayers((char*)&sPacket, sPacket.size);
}

void cHELLSGATE::SendToPlayers(char * psPacket, int size)
{
	for (auto player : vPlayers)
	{
		rsPLAYINFO* lpPlayInfo = FindUserFromCode(player.dwObjectSerial);

		if (lpPlayInfo && lpPlayInfo->lpsmSock)
			lpPlayInfo->lpsmSock->Send(psPacket, size, TRUE);
	}
}

void cHELLSGATE::SendMessageToPlayers(char * szMessage, bool bToAll)
{
	TRANS_CHATMESSAGE TransChatMessage;
	lstrcpy(TransChatMessage.szMessage, szMessage);
	TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
	TransChatMessage.size = lstrlen(TransChatMessage.szMessage) + 32;
	TransChatMessage.dwIP = 3;
	TransChatMessage.dwObjectSerial = 0;
	if (bToAll)
		SendToAll((char*)&TransChatMessage, TransChatMessage.size);
	else
		SendToPlayers((char*)&TransChatMessage, TransChatMessage.size);
}

void cHELLSGATE::SendReward(rsPLAYINFO * lpPlayInfo)
{
	spJobCode = lpPlayInfo->smCharInfo.JOB_CODE;
	CreateCommandItem(lpPlayInfo, lpPlayInfo, "CX101");
	//CreateCommandItem(lpPlayInfo, lpPlayInfo, "os102");
	spJobCode = 0;
}

void cHELLSGATE::SendLeaveArena(rsPLAYINFO * lpPlayInfo)
{
	smTRANS_COMMAND sPacket;
	sPacket.code = OpCode::OPCODE_LEAVE_HG;
	sPacket.size = sizeof(smTRANS_COMMAND);
	lpPlayInfo->lpsmSock->Send((char*)&sPacket, sPacket.size, TRUE);
}


