int EecodeCompress(BYTE *lpSrcData, BYTE *lpDestData, int size, int DestSize)
{
	if(!lpSrcData || !lpDestData || size < 0 || DestSize < (int)sizeof(int)) return 0;

	int Counter = 0;
	int EncCounter = (int)sizeof(int);

	while(Counter < size)
	{
		const BYTE c1 = lpSrcData[Counter];
		const BYTE c2 = (Counter + 1 < size) ? lpSrcData[Counter + 1] : c1;

		if(c1 == 0 && c2 == 0)
		{
			int cnt = 0;
			while(cnt < 0x7F && Counter < size && lpSrcData[Counter] == 0)
			{
				++Counter;
				++cnt;
			}

			if(EncCounter >= DestSize) return 0;
			lpDestData[EncCounter++] = (BYTE)(cnt | 0x80);
		}
		else
		{
			if(EncCounter >= DestSize) return 0;
			const int pSet = EncCounter++;
			int cnt = 0;

			while(cnt < 0x7F && Counter < size)
			{
				const BYTE v1 = lpSrcData[Counter];
				const BYTE v2 = (Counter + 1 < size) ? lpSrcData[Counter + 1] : v1;
				if(v1 == 0 && v2 == 0) break;

				if(EncCounter >= DestSize) return 0;
				lpDestData[EncCounter++] = lpSrcData[Counter++];
				++cnt;
			}

			if(cnt == 0) return 0;
			lpDestData[pSet] = (BYTE)cnt;
		}
	}

	memcpy(lpDestData, &EncCounter, sizeof(EncCounter));
	return EncCounter;
}





int DecodeCompress_ItemPotion(rsPLAYINFO *lpPlayInfo, BYTE *lpSrcData, BYTE *lpDestData, int deCount, void *lpTransRecdata)
{

	int DecCounter;
	int cnt;
	int Counter;
	int size;
	int len;
	int dCount = deCount;
	int pCnt;
	TRANS_RECORD_DATA	*lpTransRecordData = (TRANS_RECORD_DATA *)lpTransRecdata;

	sRECORD_ITEM	*lpRecordItem = (sRECORD_ITEM *)lpDestData;

	size = ((int *)lpSrcData)[0];

	Counter = 4;
	DecCounter = 0;

	while (1)
	{
		if (Counter >= size) break;
		if (lpSrcData[Counter] & 0x80)
		{

			len = lpSrcData[Counter++] & 0x7F;
			for (cnt = 0; cnt < len; cnt++)
			{
				if (DecCounter >= dCount)
				{
					if (dCount >= sizeof(sRECORD_ITEM)) goto Skip_Decmp;
					if ((lpRecordItem->sItemInfo.CODE&sinITEM_MASK1) != (sinPM1&sinITEM_MASK1))
						return Counter;

					dCount = sizeof(sRECORD_ITEM);
				}
				lpDestData[DecCounter++] = 0;
			}

		}
		else
		{
			len = lpSrcData[Counter++] & 0x7F;
			for (cnt = 0; cnt < len; cnt++)
			{
				if (DecCounter >= dCount)
				{
					if (dCount >= sizeof(sRECORD_ITEM)) goto Skip_Decmp;
					if ((lpRecordItem->sItemInfo.CODE&sinITEM_MASK1) != (sinPM1&sinITEM_MASK1))
						return Counter;

					dCount = sizeof(sRECORD_ITEM);
				}
				lpDestData[DecCounter++] = lpSrcData[Counter++];
			}
		}
	}

Skip_Decmp:

	if (lpTransRecordData)
	{

		for (cnt = 0; cnt < lpTransRecordData->ThrowItemCount; cnt++)
		{

			if (lpTransRecordData->ThrowItemInfo[cnt].dwCode == lpRecordItem->sItemInfo.CODE &&
				lpTransRecordData->ThrowItemInfo[cnt].dwKey == lpRecordItem->sItemInfo.ItemHeader.Head &&
				lpTransRecordData->ThrowItemInfo[cnt].dwSum == lpRecordItem->sItemInfo.ItemHeader.dwChkSum)
			{


				return Counter;

			}
		}
	}

	pCnt = lpRecordItem->sItemInfo.PotionCount;
	if (pCnt == 0) pCnt = 1;

	rsAddServerPotion(lpPlayInfo, lpRecordItem->sItemInfo.CODE, pCnt);
	return Counter;
}



int DecodeCompress(BYTE *lpSrcData, BYTE *lpDestData, int dCount)
{

	int DecCounter;
	int cnt;
	int Counter;
	int size;
	int len;

	size = ((int *)lpSrcData)[0];

	Counter = 4;
	DecCounter = 0;
	 
	while (1)
	{
		if (Counter >= size) break;
		if (lpSrcData[Counter] & 0x80)
		{

			len = lpSrcData[Counter++] & 0x7F;
			for (cnt = 0; cnt < len; cnt++)
			{
				if (DecCounter >= dCount) return Counter;
				lpDestData[DecCounter++] = 0;
			}

		}
		else
		{
			len = lpSrcData[Counter++] & 0x7F;
			for (cnt = 0; cnt < len; cnt++)
			{
				if (DecCounter >= dCount) return Counter;
				lpDestData[DecCounter++] = lpSrcData[Counter++];
			}
		}
	}

	return Counter;
}



int DecodeCompress(BYTE *lpSrcData, BYTE *lpDestData)
{
	return DecodeCompress(lpSrcData, lpDestData, 0x7FFFF);
}

int EecodeCompress(BYTE *lpSrcData, BYTE *lpDestData, int size)
{
	return EecodeCompress(lpSrcData, lpDestData, size, 0x7FFFF);
}


smCHAR *CheckOtherPlayPosi(int x, int y, int z)
{
	int cnt;
	int dist;
	int dx, dz;


	if (ServerMode)
		return Serv_CheckOtherPlayPosi(x, y, z);

	for (cnt = 0; cnt < OTHER_PLAYER_MAX; cnt++)
	{
		if (chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].smCharInfo.Life[0] > 0 &&
			(chrOtherPlayer[cnt].dwLastTransTime + DISPLAY_TRANS_TIME_OVER) > dwPlayTime)
		{
			if ((y - 16 * fONE) < chrOtherPlayer[cnt].pY && (y + 16 * fONE) > chrOtherPlayer[cnt].pY)
			{

				dx = (chrOtherPlayer[cnt].pX - x) >> FLOATNS;
				dz = (chrOtherPlayer[cnt].pZ - z) >> FLOATNS;
				dist = dx * dx + dz * dz;
				if (dist < (16 * 16)) return &chrOtherPlayer[cnt];
			}
		}
	}

	return NULL;
}




int MessageLoading = 0;
