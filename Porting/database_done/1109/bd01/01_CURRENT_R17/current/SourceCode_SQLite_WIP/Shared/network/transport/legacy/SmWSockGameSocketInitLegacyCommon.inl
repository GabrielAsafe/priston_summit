void InitGameSocket(int ThreadMode)
{
	WSADATA WSAData;

	hMsgWnd = hwnd;

	int status = 0;

	if((status = WSAStartup(MAKEWORD(1, 1), &WSAData)) != 0)
	{
		char szTemp[80];
		sprintf(szTemp, "%d is the err", status);
		MessageBox(hMsgWnd, szTemp, "Error", MB_OK);
	}

	for(int cnt = 0; cnt < CONNECTMAX; cnt++)
	{
		smWSock[cnt].sock = 0;
		smWSock[cnt].MyPoint = &smWSock[cnt];
		smWSock[cnt].MyNum = cnt;
		smWSock[cnt].smRecvState.smMySock = &smWSock[cnt];
		smWSock[cnt].smSendState.smMySock = &smWSock[cnt];
		smWSock[cnt].lpsmRecvRoute = 0;
		smWSock[cnt].lpsmSendRoute = 0;
	}

	if(ThreadMode)
	{
		smTransThreadMode = ThreadMode;
		InitTransThread();
	}
	else
	{
		smTransThreadMode = 0;
	}


	InitializeCriticalSection(&cSocketSection);

	InitializeCriticalSection(&cSendingSection);


	InitPacketSendQue();

}
