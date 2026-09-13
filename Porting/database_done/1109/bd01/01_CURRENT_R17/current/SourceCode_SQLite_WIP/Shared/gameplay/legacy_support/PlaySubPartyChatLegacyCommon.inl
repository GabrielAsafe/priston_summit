smCHAR	chrPartyPlayer;


int OpenPartyPlayer(char *szName, char *szModelFile1, char *szModelFile2)
{

	AddLoaderPattern(&chrPartyPlayer, szModelFile1, szModelFile2);

	chrPartyPlayer.SetMotionFromCode(CHRMOTION_STATE_STAND);
	chrPartyPlayer.ActionPattern = 101;
	chrPartyPlayer.Flag = TRUE;
	chrPartyPlayer.lpStage = 0;
	chrPartyPlayer.OnStageField = lpCurPlayer->OnStageField;

	strcpy_s(chrPartyPlayer.smCharInfo.szName, szName);

	OpenEachPlayer(&chrPartyPlayer);

	return TRUE;
}



char	szLastWhisperName[32] = { 0 };
int		LastWhisperLen = 0;


int	WhisperPartyPlayer(char *szName)
{
	char szBuff[128];
	int len;

	strcpy_s(szLastWhisperName, szName);

	chatlistSPEAKERflagChg(FALSE);
	cInterFace.ChatFlag = TRUE;
	hFocusWnd = hTextWnd;
	wsprintf(szBuff, "/:%s ", szName);
	len = lstrlen(szBuff);
	LastWhisperLen = len;
	SetWindowText(hFocusWnd, szBuff);
	SendMessage(hFocusWnd, EM_SETSEL, len, len);


	SetIME_Mode(TRUE);

	return TRUE;
}


int	SetClanChatting()
{
	char szBuff[128];
	int len;

	szLastWhisperName[0] = 0;
	hFocusWnd = hTextWnd;
	cInterFace.ChatFlag = TRUE;
	wsprintf(szBuff, "/CLAN> ");
	len = lstrlen(szBuff);
	LastWhisperLen = len;
	SetWindowText(hFocusWnd, szBuff);
	SendMessage(hFocusWnd, EM_SETSEL, len, len);

	SetIME_Mode(TRUE);

	return TRUE;
}


int IsClanChatting()
{
	if (lpCurPlayer->smCharInfo.ClassClan)
	{
		if (!chatlistSPEAKERflag())
		{
			chatlistSPEAKERflagChg(TRUE);
			SetClanChatting();
			return TRUE;
		}
		else
		{
			chatlistSPEAKERflagChg(FALSE);
			SetWindowText(hFocusWnd, "");
			return TRUE;
		}
		szLastWhisperName[0] = 0;
	}

	return FALSE;
}
