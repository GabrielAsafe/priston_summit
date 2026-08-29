int STG_AREA::OpenNpc(smTRNAS_PLAYERINFO *lpTransPlayerInfo)
{

	smCHAR *lpChar;
	char *lpFileName;
	int dist, cnt;


	for(cnt = 0; cnt < AUTOPLAYER_MAX; cnt++)
	{
		if(chrAutoPlayer[cnt].Flag &&
			chrAutoPlayer[cnt].smCharInfo.State == smCHAR_STATE_NPC &&
			chrAutoPlayer[cnt].pX == lpTransPlayerInfo->x &&
			chrAutoPlayer[cnt].pY == lpTransPlayerInfo->y &&
			chrAutoPlayer[cnt].pZ == lpTransPlayerInfo->z)
		{



			return FALSE;
		}
	}



	lpChar = srFindEmptyChar();
	if(lpChar)
	{
		lpChar->Init();

		lpTransPlayerInfo->dwObjectSerial = GetNewObjectSerial();
		lpChar->pX = lpTransPlayerInfo->x;
		lpChar->pY = lpTransPlayerInfo->y;
		lpChar->pZ = lpTransPlayerInfo->z;
		lpChar->Angle.x = lpTransPlayerInfo->ax;
		lpChar->Angle.y = lpTransPlayerInfo->ay;
		lpChar->Angle.z = lpTransPlayerInfo->az;

		lpChar->PosiAround.x = lpTransPlayerInfo->x;
		lpChar->PosiAround.y = lpTransPlayerInfo->y;
		lpChar->PosiAround.z = lpTransPlayerInfo->z;
		lpChar->DistAroundDbl = DIST_AROUND_NEAR;


		lpChar->lpExt1 = (void *)&Head;
		lpChar->dwObjectSerial = lpTransPlayerInfo->dwObjectSerial;
		lpChar->ActionPattern = 12;
		memcpy(&lpChar->smCharInfo, &lpTransPlayerInfo->smCharInfo, sizeof(smCHAR_INFO));


		lpFileName = lpChar->smCharInfo.szModelName2;
		if(lpFileName[0])
		{

			lpChar->szChatMessage[0] = 0;
			ZeroMemory(&lpChar->smMonsterInfo, sizeof(smCHAR_MONSTER_INFO));
			smCharDecode(lpFileName, &lpChar->smCharInfo, &lpChar->smMonsterInfo, lpChar->szChatMessage, sizeof(lpChar->szChatMessage));

			if(rsOpenNPC_RandomPos != -1 && lpChar->smMonsterInfo.OpenCount[1])
			{
				if((rsOpenNPC_RandomPos%lpChar->smMonsterInfo.OpenCount[1]) != lpChar->smMonsterInfo.OpenCount[0])
				{
					lpChar->Flag = 0;
					return FALSE;
				}
			}


			if(lpChar->smMonsterInfo.szName[0])
			{
				strcpy_s(lpChar->smCharInfo.szName, lpChar->smMonsterInfo.szName);
				lpChar->smMonsterInfo.szName[0] = 0;
			}
			srLinkNpcItem(lpChar);

			dist = lpChar->smMonsterInfo.MoveRange >> FLOATNS;
			lpChar->DistAroundDbl = dist*dist;
		}

		lpChar->SetLoadPattern(lpChar->smCharInfo.szModelName);
		lpChar->lpStage = lpStage;

		if(lpChar->FindMotionCountFromCode(CHRMOTION_STATE_WALK) == 0)
		{

			lpChar->MoveSpeed = 0;
		}

		return TRUE;
	}

	return FALSE;
}