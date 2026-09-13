struct PACKET_SEND_QUE
{
	smWINSOCK *lpsmSock;
	char PacketQue[smSOCKBUFF_SIZE];
	int	Len;

	PACKET_SEND_QUE()
	{
		lpsmSock = nullptr;
		ZeroMemory(PacketQue, sizeof(PacketQue));
		Len = 0;
	}
};

PACKET_SEND_QUE	PacketSendQue[PACKET_SEND_QUE_MAX];

int	PacketSendQuePush = 0;


int	InitPacketSendQue()
{
	InitializeCriticalSection(&cPacketQueSection);

	return TRUE;
}


int	ClosePacketSendQue()
{
	DeleteCriticalSection(&cPacketQueSection);
	return TRUE;
}


int PushPacketSendQue(smWINSOCK *lpsmSock, char *lpData, int len)
{
	EnterCriticalSection(&cPacketQueSection);

	int QuePos = PacketSendQuePush & PACKET_SEND_QUE_MASK;

	if(PacketSendQue[QuePos].lpsmSock)
	{
		PacketSendQue[QuePos].lpsmSock->PacketSendQueCnt--;
	}


	memcpy(PacketSendQue[QuePos].PacketQue, lpData, len);
	PacketSendQue[QuePos].lpsmSock = lpsmSock;
	PacketSendQue[QuePos].Len = len;

	PacketSendQuePush++;
	lpsmSock->PacketSendQueCnt++;

	LeaveCriticalSection(&cPacketQueSection);

	return TRUE;
}


int ClearPacketSendQue(smWINSOCK *lpsmSock)
{
	int cnt;
	int StartQuePos;
	int QuePos;

	EnterCriticalSection(&cPacketQueSection);

	StartQuePos = PacketSendQuePush - PACKET_SEND_QUE_MAX;
	if(StartQuePos < 0) StartQuePos = 0;

	for(cnt = StartQuePos; cnt < PacketSendQuePush; cnt++)
	{
		QuePos = cnt & PACKET_SEND_QUE_MASK;
		if(PacketSendQue[QuePos].lpsmSock == lpsmSock)
		{
			PacketSendQue[QuePos].lpsmSock = 0;
			lpsmSock->PacketSendQueCnt--;
		}
	}

	LeaveCriticalSection(&cPacketQueSection);

	return TRUE;
}


int PopSendPacketQue(smWINSOCK *lpsmSock)
{
	int cnt;
	int StartQuePos;
	int QuePos;

	EnterCriticalSection(&cPacketQueSection);

	StartQuePos = PacketSendQuePush - PACKET_SEND_QUE_MAX;
	if(StartQuePos < 0) StartQuePos = 0;

	for(cnt = StartQuePos; cnt < PacketSendQuePush; cnt++)
	{
		QuePos = cnt & PACKET_SEND_QUE_MASK;
		if(PacketSendQue[QuePos].lpsmSock == lpsmSock)
		{
			if(lpsmSock->Send(PacketSendQue[QuePos].PacketQue, PacketSendQue[QuePos].Len) == TRUE)
			{

				PacketSendQue[QuePos].lpsmSock = 0;
				lpsmSock->PacketSendQueCnt--;
				LeaveCriticalSection(&cPacketQueSection);
				return TRUE;
			}
			break;
		}
	}

	LeaveCriticalSection(&cPacketQueSection);

	return FALSE;
}
