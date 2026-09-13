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

		p = local_GetWord(decode, line);

		NextFlag = 0;


		if((lstrcmp(decode, "\x2A\xC3\xE2\xBF\xAC\xC0\xDA") == 0 || lstrcmpi(decode, "*ACTOR") == 0)) // MINGW_SPM_CP949_TOKEN_FIX_V1
		{
			pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
			strcpy_s(lpStgMonList->rsMonster[MonCnt].szMonsterName, strBuff);
#ifdef __MINGW32__
            // MINGW_SPM_ACTOR_MATCH_TRACE_V1
            char dbgActorName[128] = { 0 };
            strncpy(dbgActorName, strBuff, sizeof(dbgActorName) - 1);
            dbgActorName[sizeof(dbgActorName) - 1] = 0;
#endif

			for(cnt = 0; cnt < ListCnt; cnt++)
			{
				if(lstrcmp(strBuff, lpCharInfoList[cnt].szName) == 0)
				{
					lpStgMonList->rsMonster[MonCnt].lpCharInfo = &lpCharInfoList[cnt];

					p = local_GetWord(strBuff, p);
					cnt = atoi(strBuff);
					lpStgMonList->rsMonster[MonCnt].NumOpenStart = PercentCnt;
					lpStgMonList->rsMonster[MonCnt].OpenPercentage = cnt;
					PercentCnt += cnt;
					MonCnt++;
					break;
				}
			}

			
#ifdef __MINGW32__
            {
                int dbgActorMatch = -1;

                for(int dbgI = 0; dbgI < ListCnt; dbgI++)
                {
                    if(lstrcmp(dbgActorName,
                               lpCharInfoList[dbgI].szName) == 0)
                    {
                        dbgActorMatch = dbgI;
                        break;
                    }
                }

                FILE *dbgActorFp =
                    fopen("spm_actor_trace.txt", "a");

                if(dbgActorFp)
                {
                    fprintf(
                        dbgActorFp,
                        "[ACTOR] spm=%s requested='%s' "
                        "match=%d listCnt=%d\n",
                        szCharFile,
                        dbgActorName,
                        dbgActorMatch,
                        ListCnt
                    );

                    /*
                     * No primeiro MISS apenas, mostra os primeiros
                     * nomes que smCharDecode colocou na lista global.
                     */
                    static int dbgListDumped = 0;

                    if(dbgActorMatch < 0 && !dbgListDumped)
                    {
                        dbgListDumped = 1;

                        fprintf(
                            dbgActorFp,
                            "[MONSTER_LIST_BEGIN]\n"
                        );

                        int dbgMax =
                            (ListCnt < 30) ? ListCnt : 30;

                        for(int dbgI = 0; dbgI < dbgMax; dbgI++)
                        {
                            fprintf(
                                dbgActorFp,
                                "[MONSTER_LIST] index=%d name='%s' "
                                "level=%d\n",
                                dbgI,
                                lpCharInfoList[dbgI].szName,
                                lpCharInfoList[dbgI].Level
                            );
                        }

                        fprintf(
                            dbgActorFp,
                            "[MONSTER_LIST_END]\n"
                        );
                    }

                    fclose(dbgActorFp);
                }
            }
#endif

            NextFlag = TRUE;
		}

		if((lstrcmp(decode, "\x2A\xC3\xE2\xBF\xAC\xC0\xDA\xB5\xCE\xB8\xF1") == 0 || lstrcmpi(decode, "*BOSS_ACTOR") == 0))
		{

			lpBossMonster = &lpStgMonList->sBossMonsters[lpStgMonList->BossMonsterCount];

			pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
			for(cnt = 0; cnt < ListCnt; cnt++)
			{
				if(lstrcmp(strBuff, lpCharInfoList[cnt].szName) == 0)
				{
					lpBossMonster->MasterMonster = &lpCharInfoList[cnt];
					break;
				}
			}

			pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
			for(cnt = 0; cnt < ListCnt; cnt++)
			{
				if(lstrcmp(strBuff, lpCharInfoList[cnt].szName) == 0)
				{
					lpBossMonster->SlaveMonster = &lpCharInfoList[cnt];
					break;
				}
			}

			p = local_GetWord(strBuff, p);
			lpBossMonster->SlaveCount = atoi(strBuff);


			for(cnt = 0; cnt < 32; cnt++)
			{
				p = local_GetWord(strBuff, p);
				if(!strBuff[0]) break;
				lpBossMonster->bOpenTime[lpBossMonster->OpenTimeCount++] = atoi(strBuff);
			}

			lpStgMonList->BossMonsterCount++;
			NextFlag = TRUE;
		}


		if(!NextFlag && (lstrcmp(decode, "\x2A\xC3\xD6\xB4\xEB\xB5\xBF\xBD\xC3\xC3\xE2\xC7\xF6\xBC\xF6") == 0 || lstrcmp(decode, "*MAX_ACTOR_POS") == 0))
		{
			p = local_GetWord(strBuff, p);
			lpStgMonList->LimitMax = atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && (lstrcmp(decode, "\x2A\xC3\xE2\xC7\xF6\xB0\xA3\xB0\xDD") == 0 || lstrcmp(decode, "*DELAY") == 0))
		{
			p = local_GetWord(strBuff, p);
			lpStgMonList->OpenInterval = (1 << atoi(strBuff));
			if(lpStgMonList->OpenInterval > 1) lpStgMonList->OpenInterval--;

			p = local_GetWord(strBuff, p);
			if(strBuff[0])
				lpStgMonList->dwIntervalTime = atoi(strBuff) * 1000;

			NextFlag = TRUE;
		}

		if(!NextFlag && (lstrcmp(decode, "\x2A\xC3\xE2\xC7\xF6\xBC\xF6") == 0 || lstrcmp(decode, "*MAX_ACTOR") == 0))
		{
			p = local_GetWord(strBuff, p);
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
	int smaller = 0;
	int bigger = 0;

	if(Min <= Max)
	{
		smaller = Min;
		bigger = Max;
	}
	else
	{
		smaller = Max;
		bigger = Min;
	}

	int sb = (Max + 1) - Min;

	if(sb == 0)
		return Max;
	else
	{
		int rnd = rand() % sb;
		return Min + rnd;
	}
}


float GetRandomFloatPos(float Min, float Max)
{
	float smaller = 0.0f;
	float bigger = 0.0f;

	if(Min <= Max)
	{
		smaller = Min;
		bigger = Max;
	}
	else
	{
		smaller = Max;
		bigger = Min;
	}


	int sb = (int)((bigger - smaller) * 100.0f);

	if(sb == 0)
		return Max;
	else
	{
		int rnd = rand() % (sb + 1);
		return Min + ((float)((float)rnd / 100.0f));
	}
}


float GetRandomFloatPos2(float Min, float Max)
{
	float smaller = 0.0f;
	float bigger = 0.0f;

	if(Min <= Max)
	{
		smaller = Min;
		bigger = Max;
	}
	else
	{
		smaller = Max;
		bigger = Min;
	}

	int sb = (int)((bigger - smaller) * 10.0f);

	if(sb == 0)
		return Max;
	else
	{
		int rnd = rand() % (sb + 1);

		return Min + ((float)((float)rnd / 10.0f));
	}
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

struct sSPECIAL_REVISION
{
	DWORD	dwJobCode;
	short	sLevel[2];
	short	sStrength[2];
	short	sSpirit[2];
	short	sTalent[2];
	short	sDexterity[2];
	short	sHealth[2];
};

#define	sSPECIAL_REVISION_MAX		8


sSPECIAL_REVISION	sSpecialRevision[sSPECIAL_REVISION_MAX] = {

	{ JOB_CODE_MECHANICIAN,		0,0,	5,15,		-20,-10,	0,0,		-25,-15,	0,0		},
	{ JOB_CODE_FIGHTER,			0,0,	10,15,		-20,-15,	0,0,		-20,-15,	0,0		},
	{ JOB_CODE_PIKEMAN,			0,0,	10,15,		-20,-15,	0,0,		-25,-15,	0,0		},
	{ JOB_CODE_ARCHER,			0,0,	-25,-15,	-20,-10,	0,0,		15,25,		0,0		},
	{ JOB_CODE_KNIGHT,			0,0,	5,15,		-15,-10,	5,10,		-25,-15,	0,0		},
	{ JOB_CODE_ATALANTA,		0,0,	-20,-15,	-20,-10,	0,0,		15,25,		0,0		},
	{ JOB_CODE_PRIEST,			0,0,	-25,-20,	15,20,		-15,-10,	-20,-15,	0,0		},
	{ JOB_CODE_MAGICIAN,		0,0,	-25,-20,	15,25,		-15,-10,	-20,-15,	0,0		},
};


