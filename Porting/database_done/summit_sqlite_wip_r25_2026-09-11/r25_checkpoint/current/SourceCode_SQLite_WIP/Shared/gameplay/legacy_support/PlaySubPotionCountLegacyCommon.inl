int GetInvenPotionCount(sCHK_POTION_COUNT *ChkPotion)
{
	int cnt;

	ZeroMemory(ChkPotion, sizeof(sCHK_POTION_COUNT));

	for (cnt = 0; cnt < INVENTORY_MAXITEM; cnt++)
	{
		if (cInvenTory.InvenItem[cnt].Flag)
		{
			IncPotionCount(ChkPotion, &cInvenTory.InvenItem[cnt].sItemInfo);
		}
	}

	for (cnt = 0; cnt < INVENTORY_MAXITEM; cnt++)
	{
		if (cInvenTory.InvenItemTemp[cnt].Flag)
		{
			IncPotionCount(ChkPotion, &cInvenTory.InvenItemTemp[cnt].sItemInfo);
		}
	}

	if (cTrade.OpenFlag)
	{
		for (cnt = 0; cnt < MAX_TRADE_ITEM; cnt++)
		{
			if (sTrade.TradeItem[cnt].Flag)
			{
				IncPotionCount(ChkPotion, &sTrade.TradeItem[cnt].sItemInfo);
			}
		}
	}

	if (MouseItem.Flag)
		IncPotionCount(ChkPotion, &MouseItem.sItemInfo);

	return TRUE;
}

int	GetItemCheckCode(sITEMINFO *lpItem)
{
	if (lpItem->CODE < sinPM1 && lpItem->ItemName[0])
	{
		return lpItem->ItemHeader.dwChkSum;
	}
	return 0;
}
