int DecodeOpenMonster(char *szCharFile, rsSTG_MONSTER_LIST *lpStgMonList, smCHAR_INFO *lpCharInfoList, int ListCnt)
{

	FILE	*fp;
	char strBuff[64];
	char *p;
	char *pb;
	int NextFlag;
	int cnt, MonCnt, PercentCnt;
	sBOSS_MONSTER *lpBossMonster;

	ZeroMemory(lpStgMonList, sizeof(rsSTG_MONSTER) * rsSTG_MONSTER_MAX);


	lpStgMonList->OpenInterval = 0x7F;
	lpStgMonList->OpenLimit = 3;

	fp = fopen(szCharFile, "rb");
	if(fp == NULL) return FALSE;

	MonCnt = 0;
	PercentCnt = 0;

	while(!feof(fp))
	{
		if(fgets(line, 255, fp) == NULL)	break;

		p = fileread_GetWord(decode, line);

		NextFlag = 0;


		if(lstrcmp(decode, "*출연자") == 0 || lstrcmpi(decode, "*ACTOR") == 0)
		{
			pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
			strcpy_s(lpStgMonList->rsMonster[MonCnt].szMonsterName, strBuff);
			for(cnt = 0; cnt < ListCnt; cnt++)
			{
				if(lstrcmp(strBuff, lpCharInfoList[cnt].szName) == 0)
				{
					lpStgMonList->rsMonster[MonCnt].lpCharInfo = &lpCharInfoList[cnt];

					p = fileread_GetWord(strBuff, p);
					cnt = atoi(strBuff);
					lpStgMonList->rsMonster[MonCnt].NumOpenStart = PercentCnt;
					lpStgMonList->rsMonster[MonCnt].OpenPercentage = cnt;
					PercentCnt += cnt;
					MonCnt++;
					break;
				}
			}

			NextFlag = TRUE;
		}

		if(lstrcmp(decode, "*출연자두목") == 0 || lstrcmpi(decode, "*BOSS_ACTOR") == 0)
		{

			lpBossMonster = &lpStgMonList->sBossMonsters[lpStgMonList->BossMonsterCount];

			pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
			for(cnt = 0; cnt < ListCnt; cnt++)
			{
				if(lstrcmp(strBuff, lpCharInfoList[cnt].szName) == 0)
				{
					lpBossMonster->MasterMonster = &lpCharInfoList[cnt];
					break;
				}
			}

			pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
			for(cnt = 0; cnt < ListCnt; cnt++)
			{
				if(lstrcmp(strBuff, lpCharInfoList[cnt].szName) == 0)
				{
					lpBossMonster->SlaveMonster = &lpCharInfoList[cnt];
					break;
				}
			}

			p = fileread_GetWord(strBuff, p);
			lpBossMonster->SlaveCount = atoi(strBuff);


			for(cnt = 0; cnt < 32; cnt++)
			{
				p = fileread_GetWord(strBuff, p);
				if(!strBuff[0]) break;
				lpBossMonster->bOpenTime[lpBossMonster->OpenTimeCount++] = atoi(strBuff);
			}

			lpStgMonList->BossMonsterCount++;
			NextFlag = TRUE;
		}


		if(!NextFlag && (lstrcmp(decode, "*최대동시출현수") == 0 || lstrcmp(decode, "*MAX_ACTOR_POS") == 0))
		{
			p = fileread_GetWord(strBuff, p);
			lpStgMonList->LimitMax = atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && (lstrcmp(decode, "*출현간격") == 0 || lstrcmp(decode, "*DELAY") == 0))
		{
			p = fileread_GetWord(strBuff, p);
			lpStgMonList->OpenInterval = (1 << atoi(strBuff));
			if(lpStgMonList->OpenInterval > 1) lpStgMonList->OpenInterval--;

			p = fileread_GetWord(strBuff, p);
			if(strBuff[0])
				lpStgMonList->dwIntervalTime = atoi(strBuff) * 1000;

			NextFlag = TRUE;
		}

		if(!NextFlag && (lstrcmp(decode, "*출현수") == 0 || lstrcmp(decode, "*MAX_ACTOR") == 0))
		{
			p = fileread_GetWord(strBuff, p);
			lpStgMonList->OpenLimit = atoi(strBuff);
			NextFlag = TRUE;
		}


	}
	lpStgMonList->Counter = MonCnt;
	lpStgMonList->PecetageCount = PercentCnt;


	fclose(fp);

	return TRUE;
}



int GetRandomPos(int Min, int Max)
{
	int sb;
	int rnd;

	sb = (Max + 1) - Min;

	if(sb == 0) return Max;

	rnd = rand() % sb;
	return Min + rnd;
}


float GetRandomFloatPos(float Min, float Max)
{
	int sb;
	int rnd;

	sb = (int)((Max - Min)*100.0f);

	if(sb == 0) return Max;

	rnd = rand() % (sb + 1);
	return Min + ((float)((float)rnd / 100.0f));
}


float GetRandomFloatPos2(float Min, float Max)
{
	int sb;
	int rnd;

	sb = (int)((Max - Min)*10.0f);

	if(sb == 0) return Max;

	rnd = rand() % (sb + 1);
	return Min + ((float)((float)rnd / 10.0f));
}



int GetMiddlePos(int Min, int Max)
{
	int sb;

	sb = (Max - Min) / 2;

	return Min + sb;
}



static int	Random_NormalMode = 0;


int GetRandomPos_Normal(int inMin, int inMax)
{
	int sb, sc;
	int rnd;
	int	Min, Max;

	if(!Random_NormalMode)
		return GetRandomPos(inMin, inMax);

	sc = inMax - inMin;
	sb = sc / 4;

	if(sb == 0 && sc >= 2) sb = 1;

	Min = inMin + sb;
	Max = inMax - sb;

	sb = (Max + 1) - Min;

	if(sb == 0) return Max;

	rnd = rand() % sb;
	return Min + rnd;
}

// Class-specific item requirement revisions are owned by PlayableClassDefinition.

