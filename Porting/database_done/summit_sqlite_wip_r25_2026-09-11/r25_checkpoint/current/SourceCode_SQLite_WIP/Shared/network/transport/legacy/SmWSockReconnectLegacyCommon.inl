int smReconnectSock(smWINSOCK *lpsmSock)
{
	int res;


	EnterCriticalSection(&cSocketSection);

	res = lpsmSock->ReconnectSock();


	LeaveCriticalSection(&cSocketSection);

	return res;
}
