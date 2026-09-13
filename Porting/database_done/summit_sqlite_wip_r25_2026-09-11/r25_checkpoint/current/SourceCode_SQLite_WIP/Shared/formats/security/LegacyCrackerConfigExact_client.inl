int ReadCustomCracker(char *szCrackFile, TRANS_FIND_CRACKER *lpTransFindCrack)
{
	FILE	*fp;
	char strBuff[64];
	char *p, *pb;
	int cnt;


	fp = fopen(szCrackFile, "rb");
	if(fp == NULL)
	{
		return FALSE;
	}

	cnt = 0;

	while(!feof(fp))
	{

		if(fgets(line, 255, fp) == NULL)	break;

		p = fileread_GetWord(decode, line);


		if(lstrcmpi(decode, "*CRACK") == 0)
		{


			pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
			strcpy_s(lpTransFindCrack->CrackName[cnt].szName1, strBuff);

			pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
			strcpy_s(lpTransFindCrack->CrackName[cnt].szName2, strBuff);

			pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
			strcpy_s(lpTransFindCrack->CrackName[cnt].szClass, strBuff);

			cnt++;
		}
		if(cnt >= FIND_CRACK_LIST_MAX) break;

	}

	lpTransFindCrack->code = OpCode::OPCODE_FINDCRACK;
	lpTransFindCrack->size = sizeof(_CRACK_NAME)*cnt + 16;
	lpTransFindCrack->FindCounter = cnt;

	fclose(fp);

	return cnt;
}










