int SetJobFace(char *szModelName)
{

	strcpy_s(lpCurPlayer->smCharInfo.szModelName2, szModelName);
	AddLoaderPattern(lpCurPlayer, lpCurPlayer->smCharInfo.szModelName, lpCurPlayer->smCharInfo.szModelName2);
	lpCurPlayer->smCharInfo.bUpdateInfo[0]++;
	ReformCharForm();


	SendSetObjectSerial(lpCurPlayer->dwObjectSerial);

	return TRUE;
}
