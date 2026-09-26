int	Open_SODConfig(char *szFileName, void *lphCoreMonster, int *SodTax, char *szSodText, size_t szSodTextSize, int *SodScoreDiv)
{

	FILE	*fp;
	char strBuff[256];
	char *p;
	char *pb;
	int NextFlag;
	int cnt;
	int hcRoundCount = 0;

	int	m_SodTax = 0;
	int m_SodScoreDiv = 0;
	char m_szSodText[256];
	sHCORE_MONSTER	hcMonster[hCORE_ROUND_MAX];
	sHCORE_MONSTER	*lpSod_Monster = (sHCORE_MONSTER *)lphCoreMonster;

	m_szSodText[0] = 0;
	memcpy(hcMonster, lpSod_Monster, sizeof(sHCORE_MONSTER)*hCORE_ROUND_MAX);

	for(cnt = 0; cnt < rsHARDCORE_COUNT_END; cnt++)
	{

		hcMonster[cnt].hCoreMonster1.szName[0] = 0;
		hcMonster[cnt].hCoreMonster1.lpCharInfo = 0;
		hcMonster[cnt].hCoreMonster1.perCount = 0;

		hcMonster[cnt].hCoreMonster2.szName[0] = 0;
		hcMonster[cnt].hCoreMonster2.lpCharInfo = 0;
		hcMonster[cnt].hCoreMonster2.perCount = 0;

		hcMonster[cnt].hCoreMonster3.szName[0] = 0;
		hcMonster[cnt].hCoreMonster3.lpCharInfo = 0;
		hcMonster[cnt].hCoreMonster3.perCount = 0;

		hcMonster[cnt].hCoreMonster4.szName[0] = 0;
		hcMonster[cnt].hCoreMonster4.lpCharInfo = 0;
		hcMonster[cnt].hCoreMonster4.perCount = 0;

		hcMonster[cnt].hCoreMonsterBoss.szName[0] = 0;
		hcMonster[cnt].hCoreMonsterBoss.lpCharInfo = 0;
		hcMonster[cnt].hCoreMonsterBoss.perCount = 0;

		hcMonster[cnt].MaxMonster = 0;
	}


	fp = summit::content::text_resource::OpenTextResourceFile(szFileName, "rb");
	if(fp == NULL) return FALSE;


	while(!feof(fp))
	{
		if(fgets(line, 255, fp) == NULL)	break;

		p = local_GetWord(decode, line);

		NextFlag = 0;


		if(lstrcmpi(decode, "*BELATRA_TAX") == 0)
		{

			pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
			if(strBuff[0]) m_SodTax = atoi(strBuff);

			NextFlag = TRUE;
		}



		if(!NextFlag && lstrcmpi(decode, "*BELATRA_TEXT") == 0)
		{

			pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
			strcpy_s(m_szSodText, strBuff);

			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmpi(decode, "*BELATRA_DIVSCORE") == 0)
		{

			pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
			if(strBuff[0]) m_SodScoreDiv = atoi(strBuff);

			NextFlag = TRUE;
		}



		if(!NextFlag && lstrcmpi(decode, "*BELATRA_MONSTER") == 0)
		{
			if(hcRoundCount < rsHARDCORE_COUNT_END)
			{
				pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
				strcpy_s(hcMonster[hcRoundCount].hCoreMonster1.szName, strBuff);
				p = local_GetWord(strBuff, p);
				hcMonster[hcRoundCount].hCoreMonster1.perCount = atoi(strBuff);

				pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
				strcpy_s(hcMonster[hcRoundCount].hCoreMonster2.szName, strBuff);
				p = local_GetWord(strBuff, p);
				hcMonster[hcRoundCount].hCoreMonster2.perCount = atoi(strBuff);

				pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
				strcpy_s(hcMonster[hcRoundCount].hCoreMonster3.szName, strBuff);
				p = local_GetWord(strBuff, p);
				hcMonster[hcRoundCount].hCoreMonster3.perCount = atoi(strBuff);

				pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
				strcpy_s(hcMonster[hcRoundCount].hCoreMonster4.szName, strBuff);
				p = local_GetWord(strBuff, p);
				hcMonster[hcRoundCount].hCoreMonster4.perCount = atoi(strBuff);

				pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
				strcpy_s(hcMonster[hcRoundCount].hCoreMonsterBoss.szName, strBuff);
				hcMonster[hcRoundCount].hCoreMonsterBoss.perCount = 0;

				p = local_GetWord(strBuff, p);
				hcMonster[hcRoundCount].MaxMonster = atoi(strBuff);

				hcRoundCount++;
			}
			NextFlag = TRUE;
		}
	}
	fclose(fp);

	if(hcRoundCount < rsHARDCORE_COUNT_END)
	{
		return 0;
	}

	if(m_SodTax >= 0 && m_SodTax < 100)
	{
		*SodTax = m_SodTax;
	}

	if(m_szSodText[0])
	{
		strcpy_s(szSodText, szSodTextSize, m_szSodText);
	}

	if(m_SodScoreDiv > 0)
	{
		*SodScoreDiv = m_SodScoreDiv;
	}

	memcpy(lpSod_Monster, hcMonster, sizeof(sHCORE_MONSTER)*hCORE_ROUND_MAX);

	return 1;
}





