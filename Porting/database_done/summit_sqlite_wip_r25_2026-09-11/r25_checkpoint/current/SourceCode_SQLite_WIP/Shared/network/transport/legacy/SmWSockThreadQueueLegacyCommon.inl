void InitTransThread()
{
	DWORD dwRecvMsgId;
	DWORD dwSendMsgId;


	InitializeCriticalSection(&cTransSection);

	smWaitSendQuePop = 0;
	smWaitSendQuePush = 0;
	smWaitRecvQuePop = 0;
	smWaitRecvQuePush = 0;


	for(int cnt = 0; cnt < TRANS_THREAD_SEND_MAX; cnt++)
	{
		smTransSendThreads[cnt].smThreadSock = 0;
		smTransSendThreads[cnt].MyPoint = &smTransSendThreads[cnt];

		smTransSendThreads[cnt].hThread = CreateThread(NULL, 0, smTransSendThreadProc, &smTransSendThreads[cnt], 0, &dwRecvMsgId);

	}

	for(int cnt = 0; cnt < TRANS_THREAD_RECV_MAX; cnt++)
	{
		smTransRecvThreads[cnt].smThreadSock = 0;
		smTransRecvThreads[cnt].MyPoint = &smTransRecvThreads[cnt];

		smTransRecvThreads[cnt].hThread = CreateThread(NULL, 0, smTransRecvThreadProc, &smTransRecvThreads[cnt], 0, &dwSendMsgId);

	}
}


int PushTransSendSocket(smWINSOCK *lpsmsock)
{



	smWaitSendSocketQue[smWaitSendQuePush&TRANS_WAIT_QUE_MASK] = lpsmsock;
	smWaitSendQuePush++;




	return smWaitSendQuePush;
}


smWINSOCK *PopTransSendSocket()
{
	smWINSOCK *lpsmsock = nullptr;

	while(true)
	{
		if(smWaitSendQuePop >= smWaitSendQuePush)
		{
			break;
		}

		lpsmsock = smWaitSendSocketQue[smWaitSendQuePop & TRANS_WAIT_QUE_MASK];
		smWaitSendQuePop++;

		if(lpsmsock)
		{
			break;
		}
	}




	return lpsmsock;
}


int PushTransRecvSocket(smWINSOCK *lpsmsock)
{



	smWaitRecvSocketQue[smWaitRecvQuePush&TRANS_WAIT_QUE_MASK] = lpsmsock;
	smWaitRecvQuePush++;




	return smWaitRecvQuePush;
}


smWINSOCK *PopTransRecvSocket()
{
	smWINSOCK *lpsmsock;




	lpsmsock = NULL;

	while(1)
	{
		if(smWaitRecvQuePop >= smWaitRecvQuePush)
			break;

		lpsmsock = smWaitRecvSocketQue[smWaitRecvQuePop&TRANS_WAIT_QUE_MASK];
		smWaitRecvQuePop++;

		if(lpsmsock)
			break;
	}




	return lpsmsock;
}


int GetUseingSendThreadCount()
{
	int Count = 0;

	for(int cnt = 0; cnt < TRANS_THREAD_SEND_MAX; cnt++)
	{
		if(smTransSendThreads[cnt].smThreadSock)
		{
			Count++;
		}
	}

	return Count;
}


int GetUseingRecvThreadCount()
{
	int Count = 0;

	for(int cnt = 0; cnt < TRANS_THREAD_RECV_MAX; cnt++)
	{
		if(smTransRecvThreads[cnt].smThreadSock)
		{
			Count++;
		}
	}

	return Count;
}


int ClearTransQueSocket(smWINSOCK *lpsmsock)
{
	int cnt;
	int mCnt;


	EnterCriticalSection(&cTransSection);

	for(cnt = smWaitRecvQuePop; cnt < smWaitRecvQuePush; cnt++)
	{
		mCnt = cnt & TRANS_WAIT_QUE_MASK;

		if(smWaitRecvSocketQue[mCnt] == lpsmsock)
		{
			smWaitRecvSocketQue[mCnt] = 0;
		}
	}

	for(cnt = smWaitSendQuePop; cnt < smWaitSendQuePush; cnt++)
	{
		mCnt = cnt & TRANS_WAIT_QUE_MASK;

		if(smWaitSendSocketQue[mCnt] == lpsmsock)
		{
			smWaitSendSocketQue[mCnt] = 0;
		}
	}


	if(lpsmsock->lpsmRecvRoute)
	{


		if(lpsmsock->lpsmRecvRoute->smThreadSock &&
			lpsmsock->lpsmRecvRoute->smThreadSock->smMySock == lpsmsock)
		{

			lpsmsock->lpsmRecvRoute->TimeOut = TRUE;
			ResumeThread(lpsmsock->lpsmRecvRoute->hThread);
		}
		else
		{
			lpsmsock->lpsmRecvRoute = 0;
			smTransClearErrorCnt++;
		}
	}


	if(lpsmsock->lpsmSendRoute)
	{


		if(lpsmsock->lpsmSendRoute->smThreadSock &&
			lpsmsock->lpsmSendRoute->smThreadSock->smMySock == lpsmsock)
		{

			lpsmsock->lpsmSendRoute->TimeOut = TRUE;
			ResumeThread(lpsmsock->lpsmSendRoute->hThread);
		}
		else
		{
			lpsmsock->lpsmSendRoute = 0;
			smTransClearErrorCnt++;
		}
	}



	LeaveCriticalSection(&cTransSection);

	return TRUE;
}


int CheckLostTransThread()
{
	int cnt;
	DWORD dwTime;
	DWORD dwAcessTime;
	smWINSOCK *lpLostSock[TRANS_THREAD_SEND_MAX + TRANS_THREAD_RECV_MAX];
	int LostCount = 0;


	EnterCriticalSection(&cTransSection);

	dwTime = GetCurrentTime();

	for(cnt = 0; cnt < TRANS_THREAD_RECV_MAX; cnt++)
	{
		if(smTransRecvThreads[cnt].smThreadSock)
		{

			if(smTransRecvThreads[cnt].TimeOut && smTransRecvThreads[cnt].Sleep)
			{
				ResumeThread(smTransRecvThreads[cnt].hThread);
			}

			dwAcessTime = smTransRecvThreads[cnt].smThreadSock->dwAcessTime;
			if(dwAcessTime && (dwAcessTime + TRANS_LOST_TIME) < dwTime)
			{




				if(smTransRecvThreads[cnt].smThreadSock->smMySock)
				{

					lpLostSock[LostCount++] = smTransRecvThreads[cnt].smThreadSock->smMySock;

				}
			}
			else
			{

				if(smTransRecvThreads[cnt].Sleep &&
					smTransRecvThreads[cnt].smThreadSock->smMySock &&
					(&smTransRecvThreads[cnt]) != smTransRecvThreads[cnt].smThreadSock->smMySock->lpsmRecvRoute)
				{
					if(dwTime > (smTransRecvThreads[cnt].dwSleepTime + (TRANS_LOST_TIME * 2)))
					{

						smTransRecvThreads[cnt].TimeOut = TRUE;
						ResumeThread(smTransRecvThreads[cnt].hThread);

					}
				}
			}
		}
	}

	for(cnt = 0; cnt < TRANS_THREAD_SEND_MAX; cnt++)
	{
		if(smTransSendThreads[cnt].smThreadSock)
		{

			if(smTransSendThreads[cnt].TimeOut && smTransSendThreads[cnt].Sleep)
			{
				ResumeThread(smTransSendThreads[cnt].hThread);
			}

			dwAcessTime = smTransSendThreads[cnt].smThreadSock->dwAcessTime;
			if(dwAcessTime && (dwAcessTime + TRANS_LOST_TIME) < dwTime)
			{




				if(smTransSendThreads[cnt].smThreadSock->smMySock)
				{
					lpLostSock[LostCount++] = smTransSendThreads[cnt].smThreadSock->smMySock;

				}

			}
			else
			{

				if(smTransSendThreads[cnt].Sleep &&
					smTransSendThreads[cnt].smThreadSock->smMySock &&
					(&smTransSendThreads[cnt]) != smTransSendThreads[cnt].smThreadSock->smMySock->lpsmSendRoute)
				{
					if(dwTime > (smTransSendThreads[cnt].dwSleepTime + (TRANS_LOST_TIME * 2)))
					{

						smTransSendThreads[cnt].TimeOut = TRUE;
						ResumeThread(smTransSendThreads[cnt].hThread);

					}
				}
			}
		}
	}

	LeaveCriticalSection(&cTransSection);

	for(cnt = 0; cnt < LostCount; cnt++)
	{
		if(lpLostSock[cnt]->sock)
			lpLostSock[cnt]->CloseSocket();
	}

	return LostCount;
}


smTHREAD_ROUTE *FindTransThread(int SRMode)
{
	int cnt;

	if(SRMode)
	{
		for(cnt = 0; cnt < TRANS_THREAD_SEND_MAX; cnt++)
		{
			if(!smTransSendThreads[cnt].smThreadSock && smTransSendThreads[cnt].Sleep)
			{
				return &smTransSendThreads[cnt];
			}
		}
	}
	else
	{
		for(cnt = 0; cnt < TRANS_THREAD_RECV_MAX; cnt++)
		{
			if(!smTransRecvThreads[cnt].smThreadSock && smTransRecvThreads[cnt].Sleep)
			{
				return &smTransRecvThreads[cnt];
			}
		}
	}

	return FALSE;
}
