void CloseBindSock()
{
	closesocket(BindSock);
}




BOOL FillAddr(
	HWND hWnd,
	PSOCKADDR_IN psin,
	BOOL bClient)
{
	DWORD dwSize;
	PHOSTENT phe;
	char szTemp[200];


	psin->sin_family = AF_INET;
	psin->sin_port = htons(BindPort);


	if(bClient)
	{
		phe = gethostbyname(szBuff);
		if(phe == NULL)
		{
			sprintf(szTemp, "%d is the error. Make sure '%s' is listed in the hosts file.", WSAGetLastError(), szBuff);

			MessageBox(hWnd, szTemp, "gethostbyname() failed.", MB_OK);
			return FALSE;
		}
		memcpy((char FAR *)&(psin->sin_addr), phe->h_addr,
			phe->h_length);

	}
	else
	{



		dwSize = sizeof(szBuff);
		gethostname(szBuff, dwSize);

		psin->sin_addr.s_addr = INADDR_ANY;
	}

	return TRUE;
}
