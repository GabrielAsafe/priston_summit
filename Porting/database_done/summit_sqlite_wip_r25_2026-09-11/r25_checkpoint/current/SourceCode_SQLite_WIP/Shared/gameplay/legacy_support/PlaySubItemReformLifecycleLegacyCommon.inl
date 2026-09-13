int ReformItem(sITEMINFO *lpItemInfo)
{


	DWORD dwCode;

#ifdef _W_SERVER

	extern time_t		tServerTime;

	lpItemInfo->ItemHeader.Head = ItemFormKey + GetCurrentTime();

	dwCode = GetRefromItemCode(lpItemInfo, lpItemInfo->ItemHeader.Head);

	lpItemInfo->ItemHeader.dwChkSum = dwCode;
	lpItemInfo->ItemHeader.dwVersion = ItemFormVersion;
	lpItemInfo->dwCreateTime = (DWORD)tServerTime;

	if (!lpItemInfo->BackUpKey || !lpItemInfo->BackUpChkSum)
	{
		lpItemInfo->BackUpKey = lpItemInfo->ItemHeader.Head;
		lpItemInfo->BackUpChkSum = lpItemInfo->ItemHeader.dwChkSum;
	}

	ReformMixItem(lpItemInfo);
	rsReformItem_Server(lpItemInfo);

	lpItemInfo->Temp0 = lpItemInfo->ItemHeader.dwChkSum + lpItemInfo->CODE;

#else

	if (!lpItemInfo->CODE || lpItemInfo->CODE == (sinGG1 | sin01) || (lpItemInfo->CODE&sinITEM_MASK1) == (sinPM1&sinITEM_MASK1))
	{
		lpItemInfo->ItemHeader.Head = ItemFormKey + GetCurrentTime();

		dwCode = GetRefromItemCode(lpItemInfo, lpItemInfo->ItemHeader.Head);

		lpItemInfo->ItemHeader.dwChkSum = dwCode;
		lpItemInfo->ItemHeader.dwVersion = ItemFormVersion;
	}
#endif

	ItemFormKey++;

	return TRUE;
}


int ReformItem_AdjustTime(sITEMINFO *lpItemInfo, int TimeAdjust)
{

#ifdef _W_SERVER


	DWORD dwCode;


	extern time_t		tServerTime;

	lpItemInfo->ItemHeader.Head = ItemFormKey + GetCurrentTime();

	dwCode = GetRefromItemCode(lpItemInfo, lpItemInfo->ItemHeader.Head);

	lpItemInfo->ItemHeader.dwChkSum = dwCode;
	lpItemInfo->ItemHeader.dwVersion = ItemFormVersion;
	lpItemInfo->dwCreateTime = (DWORD)tServerTime + TimeAdjust;

	if (!lpItemInfo->BackUpKey || !lpItemInfo->BackUpChkSum)
	{
		lpItemInfo->BackUpKey = lpItemInfo->ItemHeader.Head;
		lpItemInfo->BackUpChkSum = lpItemInfo->ItemHeader.dwChkSum;
	}

	ReformMixItem(lpItemInfo);
	rsReformItem_Server(lpItemInfo);

	lpItemInfo->Temp0 = lpItemInfo->ItemHeader.dwChkSum + lpItemInfo->CODE;

#endif

	ItemFormKey++;

	return TRUE;
}

#ifdef _W_SERVER


int CheckItemForm(sITEMINFO *lpItemInfo)
{
	DWORD dwCode;




	if (lpItemInfo->CODE == (sinGG1 | sin01) || (lpItemInfo->CODE&sinITEM_MASK1) == (sinPM1&sinITEM_MASK1))
	{
		dwCode = GetRefromItemCode(lpItemInfo, lpItemInfo->ItemHeader.Head);
		if (lpItemInfo->ItemHeader.dwChkSum == dwCode) return TRUE;
		return FALSE;
	}



	dwCode = rsGetItemForm_Server(lpItemInfo);

	if (lpItemInfo->ItemHeader.dwTime == dwCode) return TRUE;

	return FALSE;
}

#else


int CheckItemForm(sITEMINFO *lpItemInfo)
{
	DWORD dwCode;

	dwCode = GetRefromItemCode(lpItemInfo, lpItemInfo->ItemHeader.Head);

	if (lpItemInfo->ItemHeader.dwChkSum == dwCode) return TRUE;

	return FALSE;
}

#endif


int CompareItems(sITEMINFO *lpItemInfo1, sITEMINFO *lpItemInfo2)
{

	if (lpItemInfo1->ItemHeader.dwVersion != (DWORD)ItemFormVersion) CheckItemForm(lpItemInfo1);
	if (lpItemInfo2->ItemHeader.dwVersion != (DWORD)ItemFormVersion) CheckItemForm(lpItemInfo2);

	if (lpItemInfo1->CODE != lpItemInfo2->CODE) return FALSE;
	if (lpItemInfo1->ItemHeader.Head != lpItemInfo2->ItemHeader.Head) return FALSE;
	if (lpItemInfo1->ItemHeader.dwChkSum != lpItemInfo2->ItemHeader.dwChkSum) return FALSE;

	if (CompareReformItemCode(lpItemInfo1, lpItemInfo2) == 0)
		return TRUE;

	return FALSE;
}


int ReformCharForm(smCHAR_INFO *lpCharInfo)
{
	DWORD dwCode;

	dwCode = GetRefromCharState(lpCharInfo, 10);

	lpCharInfo->RefomCode = dwCode;

	ItemFormKey++;

	return TRUE;
}


int CheckCharForm(smCHAR_INFO *lpCharInfo)
{
	DWORD dwCode;

	dwCode = GetRefromCharState(lpCharInfo, 10);

	if (lpCharInfo->RefomCode == dwCode) return TRUE;

	return FALSE;
}


int ReformCharForm()
{



	return ReformCharForm(&lpCurPlayer->smCharInfo);

}
