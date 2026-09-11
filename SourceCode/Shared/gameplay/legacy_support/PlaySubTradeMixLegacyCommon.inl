int ReformTrade(void *lpTrade)
{
	DWORD dwCode;

	dwCode = GetRefromTradeCode((sTRADE *)lpTrade, 84);

	((sTRADE *)lpTrade)->CheckMoney = dwCode;

	ItemFormKey++;

	return TRUE;
}


int CheckTrade(void *lpTrade)
{
	DWORD dwCode;

	dwCode = GetRefromTradeCode((sTRADE *)lpTrade, 84);

	if (((sTRADE *)lpTrade)->CheckMoney == (int)dwCode) return TRUE;


	SendSetHackUser2(4000, 0);

	return FALSE;
}




short GetMixItemForm(sITEMINFO *lpItemInfo)
{
	short sCode = 0;

#ifdef _W_SERVER

	DWORD dwCode;
	DWORD dwItemKey;

	if (lpItemInfo->ItemKindCode)
	{
		dwItemKey = lpItemInfo->ItemHeader.dwChkSum;



		dwCode = lpItemInfo->ItemKindCode*(
			lpItemInfo->ItemAgingNum[0] * 3 + lpItemInfo->ItemAgingNum[1] * 7 + lpItemInfo->ItemAgingCount[1] * 17 + 17);

		dwCode = dwCode ^ dwItemKey;

		sCode = (short)((dwCode >> 16) ^ (dwCode & 0xFFFF));
	}
#endif

	return sCode;
}


int ReformMixItem(sITEMINFO *lpItemInfo)
{
#ifdef _W_SERVER
	lpItemInfo->ItemAgingProtect[0] = GetMixItemForm(lpItemInfo);
#endif
	return TRUE;
}


int CheckMixItem(sITEMINFO *lpItemInfo)
{

#ifdef _W_SERVER
	if (GetMixItemForm(lpItemInfo) == lpItemInfo->ItemAgingProtect[0])
	{
		return TRUE;
	}
#else
	return TRUE;
#endif

	return FALSE;
}
