#define WAIT_BATTLE_QUIT_TIME		10000


int QuitGame()
{
#ifdef _XIGNCODE_CLIENT

	ZCWAVE_Cleanup();
	ZCWAVE_SysExit();
#endif

	if (quit)
		return TRUE;

	if (dwBattleTime)
	{
		if ((dwBattleTime + WAIT_BATTLE_QUIT_TIME) > dwPlayTime)
		{
			dwBattleQuitTime = dwPlayTime;
			return FALSE;
		}
	}


	if (cWareHouse.OpenFlag || cCraftItem.OpenFlag || cTrade.OpenFlag || cAging.OpenFlag || SmeltingItem.OpenFlag || ManufactureItem.m_OpenFlag || cMixtureReset.OpenFlag)
	{
		dwCloseBoxTime = dwPlayTime + 2000;
		return FALSE;
	}

	quit = 1;

	return TRUE;
}

#define CHARINFO_CACHE_MAX		200

smTRNAS_PLAYERINFO	CharInfo_Cache[CHARINFO_CACHE_MAX];
DWORD	dwCharInfo_CacheCode[CHARINFO_CACHE_MAX];
int	CharInfoCacheCnt = 0;


smTRNAS_PLAYERINFO	*GetCharInfoCache(char *lpData)
{
	int cnt = 0;
	smPLAYDATA *lpPlayData = nullptr;
	smTRANS_PLAYDATA *lpTransPlayData = nullptr;
	smTRANS_PLAYBUFF *lpTransPlayBuff = nullptr;
	smTRNAS_PLAYERINFO *lpTransPlayInfo = nullptr;

	int RecvCode = ((int *)lpData)[1];

	DWORD dwChkCode = 0;

	switch (RecvCode)
	{
	case OpCode::OPCODE_PLAYDATA1:
		lpPlayData = (smPLAYDATA *)lpData;
		dwChkCode = lpPlayData->dwAutoCharCode;
		break;
	case OpCode::OPCODE_TRANSPLAYDATA:
		lpTransPlayData = (smTRANS_PLAYDATA *)lpData;
		lpTransPlayBuff = (smTRANS_PLAYBUFF *)(lpData + sizeof(smTRANS_PLAYDATA));
		if (lpTransPlayData->PlayBuffCnt > 0)
			dwChkCode = lpTransPlayData->dwAutoCharCode;
		break;
	}

	if (!dwChkCode) return NULL;

	for (cnt = 0; cnt < CharInfoCacheCnt; cnt++)
	{
		if (dwChkCode == dwCharInfo_CacheCode[cnt])
		{

			lpTransPlayInfo = &CharInfo_Cache[cnt];

			switch (RecvCode)
			{
			case OpCode::OPCODE_PLAYDATA1:

				lpTransPlayInfo->x = lpPlayData->x;
				lpTransPlayInfo->y = lpPlayData->y;
				lpTransPlayInfo->z = lpPlayData->z;
				lpTransPlayInfo->ax = lpPlayData->x;
				lpTransPlayInfo->ay = lpPlayData->y;
				lpTransPlayInfo->az = lpPlayData->z;
				lpTransPlayInfo->dwObjectSerial = lpPlayData->dwObjectSerial;
				lpTransPlayInfo->smCharInfo.dwObjectSerial = lpPlayData->dwObjectSerial;
				break;

			case OpCode::OPCODE_TRANSPLAYDATA:

				lpTransPlayBuff = (smTRANS_PLAYBUFF *)(((char *)lpTransPlayData) + sizeof(smTRANS_PLAYDATA));
				lpTransPlayBuff += lpTransPlayData->PlayBuffCnt - 1;

				lpTransPlayInfo->x = lpTransPlayBuff->PlayBuff.x;
				lpTransPlayInfo->y = lpTransPlayBuff->PlayBuff.y;
				lpTransPlayInfo->z = lpTransPlayBuff->PlayBuff.z;

				lpTransPlayInfo->ax = lpTransPlayBuff->PlayBuff.angle[0];
				lpTransPlayInfo->ay = lpTransPlayBuff->PlayBuff.angle[1];
				lpTransPlayInfo->az = lpTransPlayBuff->PlayBuff.angle[2];
				lpTransPlayInfo->dwObjectSerial = lpTransPlayData->dwObjectSerial;
				lpTransPlayInfo->smCharInfo.dwObjectSerial = lpTransPlayData->dwObjectSerial;
				break;
			}

			lpTransPlayInfo->smCharInfo.Life[0] = lpTransPlayInfo->smCharInfo.Life[1];
			return lpTransPlayInfo;
		}
	}

	return NULL;
}


int SaveCharInfoCache(smTRNAS_PLAYERINFO *lpTransPlayInfo)
{
	int cnt;
	DWORD	dwCode;

	dwCode = GetSpeedSum(lpTransPlayInfo->smCharInfo.szName);


	for (cnt = 0; cnt < CharInfoCacheCnt; cnt++)
	{
		if (dwCode == dwCharInfo_CacheCode[cnt]) return NULL;
	}


	dwCharInfo_CacheCode[CharInfoCacheCnt] = dwCode;
	memcpy(&CharInfo_Cache[CharInfoCacheCnt], lpTransPlayInfo, sizeof(smTRNAS_PLAYERINFO));
	CharInfoCacheCnt++;

	return TRUE;
}
