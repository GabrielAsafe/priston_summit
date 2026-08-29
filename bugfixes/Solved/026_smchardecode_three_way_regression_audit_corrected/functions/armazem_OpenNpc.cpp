int STG_AREA::OpenNpc(smTRNAS_PLAYERINFO *lpTransPlayerInfo)
{

	smCHAR *lpChar;
	char *lpFileName;
	int dist, cnt;


	for (cnt = 0; cnt < AUTOPLAYER_MAX; cnt++) {
		if (chrAutoPlayer[cnt].Flag &&
			chrAutoPlayer[cnt].smCharInfo.State == smCHAR_STATE_NPC &&
			chrAutoPlayer[cnt].pX == lpTransPlayerInfo->x &&
			chrAutoPlayer[cnt].pY == lpTransPlayerInfo->y &&
			chrAutoPlayer[cnt].pZ == lpTransPlayerInfo->z) {

			//같은자리에 이미 NPC가 서있는 경우 취소

			return FALSE;
		}
	}


	//빈캐릭터를 찾는다 
	lpChar = srFindEmptyChar();
	if (lpChar) {
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

		//확장자 변경 해서 찾음
		lpFileName = lpChar->smCharInfo.szModelName2;
		if (lpFileName[0]) {
			//채팅 메세지 불러옴
			lpChar->szChatMessage[0] = 0;
			ZeroMemory(&lpChar->smMonsterInfo, sizeof(smCHAR_MONSTER_INFO));
			smCharDecode(lpFileName, &lpChar->smCharInfo, &lpChar->smMonsterInfo, lpChar->szChatMessage);

			if (rsOpenNPC_RandomPos != -1 && lpChar->smMonsterInfo.OpenCount[1]) {
				if ((rsOpenNPC_RandomPos%lpChar->smMonsterInfo.OpenCount[1]) != lpChar->smMonsterInfo.OpenCount[0]) {
					lpChar->Flag = 0;
					return FALSE;
				}
			}

			//보조 NPC 이름 ( 외국버전 ) 있는경우 보조 이름으로 적용
			if (lpChar->smMonsterInfo.szName[0]) {
				lstrcpy(lpChar->smCharInfo.szName, lpChar->smMonsterInfo.szName);
				lpChar->smMonsterInfo.szName[0] = 0;
			}
			srLinkNpcItem(lpChar);
			//이동범위 설정
			dist = lpChar->smMonsterInfo.MoveRange >> FLOATNS;
			lpChar->DistAroundDbl = dist * dist;
		}

		lpChar->SetLoadPattern(lpChar->smCharInfo.szModelName);
		lpChar->lpStage = lpStage;

		if (lpChar->FindMotionCountFromCode(CHRMOTION_STATE_WALK) == 0) {
			//걷는 동작 없는 NPC
			lpChar->MoveSpeed = 0;
		}

		return TRUE;
	}

	return FALSE;
}
