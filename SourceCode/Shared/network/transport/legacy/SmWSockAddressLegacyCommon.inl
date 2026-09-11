smTHREAD_ROUTE *SendThreadRoute(smWINSOCK *lpsmsock)
{
	smTHREAD_ROUTE *lpRoute = 0;


	EnterCriticalSection(&cTransSection);

	lpRoute = FindTransThread(TRUE);
	if(lpRoute)
	{
		lpRoute->smThreadSock = &lpsmsock->smSendState;
		lpRoute->TimeOut = FALSE;
		lpsmsock->lpsmSendRoute = lpRoute;

		ResumeThread(lpRoute->hThread);
	}
	else
	{
		PushTransSendSocket(lpsmsock);
	}


	LeaveCriticalSection(&cTransSection);

	return lpRoute;
}


char *ConvStrIP(DWORD dwIP)
{
	in_addr  ia;

	ia.S_un.S_addr = dwIP;
	strcpy_s(szIP, inet_ntoa(ia));

	return szIP;
}


char *smGetMyIp()
{

	PHOSTENT phe;
	in_addr  ia;


	phe = gethostbyname("");
	if(phe == NULL)
	{

		return FALSE;
	}
	ia.S_un.S_addr = ((unsigned long *)phe->h_addr)[0];
	strcpy_s(szIP, inet_ntoa(ia));

	return szIP;
}


DWORD smGetMyIp2()
{

	PHOSTENT phe = gethostbyname("");
	if(phe == NULL)
	{

		return FALSE;
	}

	return ((DWORD *)phe->h_addr)[0];
}




smWINSOCK::smWINSOCK()
{

}
smWINSOCK::~smWINSOCK()
{

}
