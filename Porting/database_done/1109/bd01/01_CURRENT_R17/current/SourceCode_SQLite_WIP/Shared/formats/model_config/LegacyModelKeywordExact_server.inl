void ModelKeyWordEncode(DWORD dwFileCode, smMODELINFO *lpMODELINFO)
{
	char *szFile;
	int   dwCode_1, dwCode_2, dwCode_3, dwCode_4;

	if(lpMODELINFO->szLinkFile[0])
	{
		szFile = ChangeFileExt(lpMODELINFO->szLinkFile, szFileExt);
		DWORD dwLinkCode = GetSpeedSum(szFile);

		dwCode_1 = ((dwFileCode & 0x00ff0000) << 8);
		dwCode_2 = ((dwLinkCode & 0x000000ff) << 16);
		dwCode_3 = ((dwFileCode & 0x000000ff) << 8);
		dwCode_4 = ((dwLinkCode & 0xff000000) >> 24);
		lpMODELINFO->FileTypeKeyWord = (dwCode_1 | dwCode_2 | dwCode_3 | dwCode_4);

		dwCode_1 = ((dwFileCode & 0xff000000));
		dwCode_2 = ((dwLinkCode & 0x0000ff00) << 8);
		dwCode_3 = ((dwFileCode & 0x0000ff00));
		dwCode_4 = ((dwLinkCode & 0x00ff0000) >> 16);
		lpMODELINFO->LinkFileKeyWord = (dwCode_1 | dwCode_2 | dwCode_3 | dwCode_4);
	}
	else
	{
		dwCode_1 = ((dwFileCode & 0x0000ff00) << 16);
		dwCode_2 = ((dwFileCode & 0xff000000) >> 8);
		dwCode_3 = ((dwFileCode & 0x000000ff) << 8);
		dwCode_4 = ((dwFileCode & 0x00ff0000) >> 16);
		lpMODELINFO->FileTypeKeyWord = (dwCode_1 | dwCode_2 | dwCode_3 | dwCode_4);
	}
}


int ModelKeyWordDecode(DWORD dwFileCode, smMODELINFO *lpMODELINFO)
{
	char *szFile;
	int   dwCode_1, dwCode_2, dwCode_3, dwCode_4;

	if(lpMODELINFO->szLinkFile[0])
	{
		szFile = ChangeFileExt(lpMODELINFO->szLinkFile, szFileExt);
		DWORD dwLinkCode = GetSpeedSum(szFile);

		dwCode_1 = ((lpMODELINFO->LinkFileKeyWord & 0xff000000));
		dwCode_2 = ((lpMODELINFO->FileTypeKeyWord & 0xff000000) >> 8);
		dwCode_3 = ((lpMODELINFO->LinkFileKeyWord & 0x0000ff00));
		dwCode_4 = ((lpMODELINFO->FileTypeKeyWord & 0x0000ff00) >> 8);
		if(dwFileCode != (dwCode_1 | dwCode_2 | dwCode_3 | dwCode_4))
			return FALSE;

		dwCode_1 = ((lpMODELINFO->FileTypeKeyWord & 0x000000ff) << 24);
		dwCode_2 = ((lpMODELINFO->LinkFileKeyWord & 0x000000ff) << 16);
		dwCode_3 = ((lpMODELINFO->LinkFileKeyWord & 0x00ff0000) >> 8);
		dwCode_4 = ((lpMODELINFO->FileTypeKeyWord & 0x00ff0000) >> 16);
		if(dwLinkCode != (dwCode_1 | dwCode_2 | dwCode_3 | dwCode_4))
			return FALSE;
	}
	else
	{
		dwCode_1 = ((lpMODELINFO->FileTypeKeyWord & 0xff000000) >> 16);
		dwCode_2 = ((lpMODELINFO->FileTypeKeyWord & 0x00ff0000) << 8);
		dwCode_3 = ((lpMODELINFO->FileTypeKeyWord & 0x0000ff00) >> 8);
		dwCode_4 = ((lpMODELINFO->FileTypeKeyWord & 0x000000ff) << 16);
		if(dwFileCode != (dwCode_2 | dwCode_4 | dwCode_1 | dwCode_3))
			return FALSE;
	}

	return TRUE;
}


void MotionKeyWordEncode(DWORD dwCode, smMODELINFO *lpMODELINFO)
{
	int cnt, i, stFrm, endFrm;
	smMOTIONINFO *lpMotionInfo;


	cnt = lpMODELINFO->MotionCount;
	for(i = CHRMOTION_EXT; i < cnt; i++)
	{
		lpMotionInfo = &lpMODELINFO->MotionInfo[i];
		if(lpMotionInfo)
		{
			stFrm = lpMotionInfo->StartFrame;
			endFrm = lpMotionInfo->EndFrame;


			if(stFrm)
			{
				lpMotionInfo->MotionKeyWord_1 = (dwCode & 0xff000000) | ((stFrm & 0x0000ff00) << 8) |
					((dwCode & 0x00ff0000) >> 8) | (stFrm & 0x000000ff);

				lpMotionInfo->StartFrame = ((dwCode & 0x000000ff) << 24) | (stFrm & 0x00ff0000) |
					(dwCode & 0x0000ff00) | ((stFrm & 0xff000000) >> 24);
			}


			if(endFrm)
			{
				lpMotionInfo->MotionKeyWord_2 = ((dwCode & 0x000000ff) << 24) | ((endFrm & 0xff000000) >> 8) |
					(dwCode & 0x0000ff00) | ((endFrm & 0x00ff0000) >> 16);

				lpMotionInfo->EndFrame = ((dwCode & 0x00ff0000) << 8) | ((endFrm & 0x0000ff00) << 8) |
					((dwCode & 0xff000000) >> 16) | (endFrm & 0x000000ff);
			}
		}
	}
}


int MotionKeyWordDecode(DWORD dwCode, smMODELINFO *lpMODELINFO)
{
	int cnt, i, KeyWord, Frame;
	smMOTIONINFO *lpMotionInfo;


	cnt = lpMODELINFO->MotionCount;
	for(i = CHRMOTION_EXT; i < cnt; i++)
	{
		lpMotionInfo = &lpMODELINFO->MotionInfo[i];
		if(lpMotionInfo)
		{

			if(lpMotionInfo->MotionKeyWord_1 || lpMotionInfo->StartFrame)
			{
				KeyWord = (lpMotionInfo->MotionKeyWord_1 & 0xff000000) | ((lpMotionInfo->MotionKeyWord_1 & 0x0000ff00) << 8) |
					(lpMotionInfo->StartFrame & 0x0000ff00) | ((lpMotionInfo->StartFrame & 0xff000000) >> 24);

				Frame = ((lpMotionInfo->StartFrame & 0x000000ff) << 24) | (lpMotionInfo->StartFrame & 0x00ff0000) |
					((lpMotionInfo->MotionKeyWord_1 & 0x00ff0000) >> 8) | (lpMotionInfo->MotionKeyWord_1 & 0x000000ff);

				if(dwCode != KeyWord)
					return FALSE;

				lpMotionInfo->MotionKeyWord_1 = 0;
				lpMotionInfo->StartFrame = Frame;
			}


			if(lpMotionInfo->MotionKeyWord_2 || lpMotionInfo->EndFrame)
			{
				KeyWord = ((lpMotionInfo->EndFrame & 0x0000ff00) << 16) | ((lpMotionInfo->EndFrame & 0xff000000) >> 8) |
					(lpMotionInfo->MotionKeyWord_2 & 0x0000ff00) | ((lpMotionInfo->MotionKeyWord_2 & 0xff000000) >> 24);

				Frame = ((lpMotionInfo->MotionKeyWord_2 & 0x00ff0000) << 8) | ((lpMotionInfo->MotionKeyWord_2 & 0x000000ff) << 16) |
					((lpMotionInfo->EndFrame & 0x00ff0000) >> 8) | (lpMotionInfo->EndFrame & 0x000000ff);

				if(dwCode != KeyWord)
					return FALSE;

				lpMotionInfo->MotionKeyWord_2 = 0;
				lpMotionInfo->EndFrame = Frame;
			}
		}
	}

	return TRUE;
}



