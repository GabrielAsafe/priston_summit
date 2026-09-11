#define		ROT_PLAYER_MAX		128
#define		ROT_PLAYER_MIN		80

smCHAR	*lpChrPlayers[ROT_PLAYER_MAX];



smCHAR *InitRotPlayer()
{
	int cnt;
	for (cnt = 0; cnt < ROT_PLAYER_MAX; cnt++)
		lpChrPlayers[cnt] = 0;

	return SelectRotPlayer(0);
}


int	CloseRotPlayer()
{
	for (int cnt = 0; cnt < ROT_PLAYER_MAX; cnt++)
	{
		if (lpChrPlayers[cnt])
		{
			delete	lpChrPlayers[cnt];
			lpChrPlayers[cnt] = nullptr;
		}
	}

	return TRUE;
}


smCHAR *SelectRotPlayer(smCHAR *lpCurrentChar)
{
	int rnd;
	smCHAR *lpChar;

	rnd = rand() % ROT_PLAYER_MIN;
	lpChar = lpChrPlayers[rnd];
	if (!lpChar)
	{
		lpChar = new smCHAR;
		lpChrPlayers[rnd] = lpChar;
	}

	if (!lpChar) return NULL;


	if (lpCurrentChar && lpCurrentChar != lpChar)
	{
		memcpy(lpChar, lpCurrentChar, sizeof(smCHAR));
	}

	if (!lpCurrentChar && lpChar)
	{
		ZeroMemory(lpChar, sizeof(smCHAR));
	}

	return lpChar;
}
