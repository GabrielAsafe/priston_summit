int SendInvenItemError(DWORD dwKind, DWORD dwItemCode, DWORD dwHead, DWORD dwChkSum)
{
	smTRANS_COMMAND	smTransCommand;

	smTransCommand.size = sizeof(smTRANS_COMMAND);
	smTransCommand.code = OpCode::OPCODE_INVEN_ERR_ITEM;
	smTransCommand.WParam = dwKind;

	smTransCommand.LParam = dwItemCode;
	smTransCommand.SParam = dwHead;
	smTransCommand.EParam = dwChkSum;

	if (smWsockDataServer)
		smWsockDataServer->Send((char *)&smTransCommand, smTransCommand.size, TRUE);

	return TRUE;
}

int	GetTotalInvenMoney()
{
	int Money;

	Money = lpCurPlayer->smCharInfo.Money;

	if (cTrade.OpenFlag && sTrade.Money)
		Money += sTrade.Money - 193;

	if (cWareHouse.OpenFlag)
		Money += sWareHouse.Money - 2023;

	return Money;
}

DWORD	dwTotal_CheckMoney = 0;
DWORD	dwTotal_CheckMoneyTime = 0;
#define	TOTAL_CHECK_MONEY_MASK	0x0018cccc
int		NChkMode = 0;



int	ResetInvenMoney()
{
	dwTotal_CheckMoney = GetTotalInvenMoney();
	dwTotal_CheckMoney = dwPlayTime ^ dwTotal_CheckMoney;
	dwTotal_CheckMoneyTime = dwPlayTime ^ TOTAL_CHECK_MONEY_MASK;

	return TRUE;
}



int	AddInvenMoney(int Money)
{
	int	m;

	m = dwTotal_CheckMoney ^ (dwTotal_CheckMoneyTime^TOTAL_CHECK_MONEY_MASK);
	m += Money;
	if (m < 0) m = 0;

	dwTotal_CheckMoney = dwPlayTime ^ m;
	dwTotal_CheckMoneyTime = dwPlayTime ^ TOTAL_CHECK_MONEY_MASK;

	return TRUE;
}


int	CheckInvenMoney()
{
	DWORD dwCode;
	smTRANS_COMMAND	smTransCommand;

	if (cShop.OpenFlag || SkillMasterFlag || cCraftItem.OpenFlag || cAging.OpenFlag || SmeltingItem.OpenFlag || ManufactureItem.m_OpenFlag || cMixtureReset.OpenFlag)
	{
		NChkMode = TRUE;
		return TRUE;
	}

	if (NChkMode)
	{
		NChkMode = 0;
		ResetInvenMoney();
		if (NChkMode == 0) return TRUE;
		dwCode = 0;
	}
	else
	{
		dwCode = GetTotalInvenMoney();
	}

	if (dwCode != (dwTotal_CheckMoney ^ (dwTotal_CheckMoneyTime^TOTAL_CHECK_MONEY_MASK)))
	{
		smTransCommand.size = sizeof(smTRANS_COMMAND);
		smTransCommand.code = OpCode::OPCODE_INVEN_ERR_MONEY;
		smTransCommand.WParam = 0;
		smTransCommand.LParam = dwCode;
		smTransCommand.SParam = (dwTotal_CheckMoney ^ (dwTotal_CheckMoneyTime^TOTAL_CHECK_MONEY_MASK));
		smTransCommand.EParam = 0;
		if (smWsockDataServer)
		{
			smWsockDataServer->Send((char *)&smTransCommand, sizeof(smTRANS_COMMAND), TRUE);
		}
	}

	dwTotal_CheckMoney = dwPlayTime ^ dwCode;
	dwTotal_CheckMoneyTime = dwPlayTime ^ TOTAL_CHECK_MONEY_MASK;

	return TRUE;
}




int SendUsingItemListToServer()
{

	int cnt;
	sITEMINFO *lpItem;

	TRANS_USINGITEM	TransUsingItem;

	ZeroMemory(&TransUsingItem, sizeof(TRANS_USINGITEM));

	TransUsingItem.size = sizeof(TRANS_USINGITEM);
	TransUsingItem.code = OpCode::OPCODE_INVEN_POSITION;

	for (cnt = 0; cnt < INVENTORY_MAXITEM; cnt++)
	{
		if (cInvenTory.InvenItem[cnt].Flag && cInvenTory.InvenItem[cnt].ItemPosition)
		{
			lpItem = &cInvenTory.InvenItem[cnt].sItemInfo;
			if (lpItem->ItemName[0])
			{
				if (TransUsingItem.ItemListCount >= 16) break;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].dwCode = lpItem->CODE;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[0] = cInvenTory.InvenItem[cnt].ItemPosition;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[1] = lpItem->Damage[1];
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[2] = lpItem->Defence;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[3] = (short)lpItem->fBlock_Rating;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[4] = (short)lpItem->fAbsorb;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[5] = lpItem->Attack_Rating;
				TransUsingItem.ItemListCount++;
			}
		}
	}

	for (cnt = 0; cnt < INVENTORY_MAXITEM; cnt++)
	{
		if (cInvenTory.InvenItemTemp[cnt].Flag && cInvenTory.InvenItemTemp[cnt].ItemPosition)
		{
			lpItem = &cInvenTory.InvenItemTemp[cnt].sItemInfo;
			if (lpItem->ItemName[0])
			{
				if (TransUsingItem.ItemListCount >= 16) break;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].dwCode = lpItem->CODE;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[0] = cInvenTory.InvenItemTemp[cnt].ItemPosition | 0x100;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[1] = lpItem->Damage[1];
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[2] = lpItem->Defence;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[3] = (short)lpItem->fBlock_Rating;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[4] = (short)lpItem->fAbsorb;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[5] = lpItem->Attack_Rating;
				TransUsingItem.ItemListCount++;
			}
		}
	}

	smWINSOCK	*lpsmSendSock;

	lpsmSendSock = GetAreaServerSock();
	if (lpsmSendSock)
	{
		return lpsmSendSock->Send((char *)&TransUsingItem, TransUsingItem.size, TRUE);
	}

	return NULL;
}


int SendUsingItemListToServer2()
{

	int cnt;
	sITEMINFO *lpItem;

	TRANS_USINGITEM	TransUsingItem;

	ZeroMemory(&TransUsingItem, sizeof(TRANS_USINGITEM));

	TransUsingItem.size = sizeof(TRANS_USINGITEM);
	TransUsingItem.code = OpCode::OPCODE_INVEN_POSITION2;

	for (cnt = 0; cnt < INVENTORY_MAXITEM; cnt++)
	{
		if (cInvenTory.InvenItem[cnt].Flag && cInvenTory.InvenItem[cnt].ItemPosition)
		{
			lpItem = &cInvenTory.InvenItem[cnt].sItemInfo;
			if (lpItem->ItemName[0])
			{
				if (TransUsingItem.ItemListCount >= 16) break;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].dwCode = lpItem->CODE;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[0] = cInvenTory.InvenItem[cnt].ItemPosition;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[1] = lpItem->Damage[1];
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[2] = lpItem->Defence;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[3] = (short)lpItem->fBlock_Rating;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[4] = (short)lpItem->fAbsorb;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[5] = lpItem->Attack_Rating;
				TransUsingItem.ItemListCount++;
			}
		}
	}

	smWINSOCK	*lpsmSendSock;

	lpsmSendSock = GetAreaServerSock();
	if (lpsmSendSock)
	{
		return lpsmSendSock->Send((char *)&TransUsingItem, TransUsingItem.size, TRUE);
	}

	return NULL;
}



DWORD	GetPlayTime_T()
{
	DWORD dwTime;

	if (dwTime_ServerT)
	{
		dwTime = (dwPlayTime - dwTime_ConnectMS) / 1000;
		dwTime = dwTime_ServerT + dwTime;
		return dwTime;
	}

	return NULL;
}



int	ChangeBigHeadMode(int Mode, int Time)
{
	DWORD	dwTime;

	if (Mode)
	{
		if (Mode == 3)
		{
			lpCurPlayer->smCharInfo.SizeLevel = 1;
			ChangeModelSkin(lpCurPlayer, 0, 6, 0, 0);
		}
		else
		{
			lpCurPlayer->smCharInfo.SizeLevel = Mode + 0x1000;
			ChangeModelSkin(lpCurPlayer, 1, lpCurPlayer->smCharInfo.SizeLevel + 3 - 0x1000, 0, 0);
		}
		dwTime = GetPlayTime_T();

		if (dwTime)
			lpCurPlayer->smCharInfo.dwEventTime_T = dwTime + Time;
		else
			lpCurPlayer->smCharInfo.dwEventTime_T = 0;
	}
	else
	{
		lpCurPlayer->smCharInfo.SizeLevel = 1;
		lpCurPlayer->smCharInfo.dwEventTime_T = 0;
		ChangeModelSkin(lpCurPlayer, 0, gameplay::classes::DefaultClassService().PromotionVisualChangeJobIndex(
		lpCurPlayer->smCharInfo.JOB_CODE, lpCurPlayer->smCharInfo.ChangeJob), 0, GetFaceHairSkin(lpCurPlayer));
	}

	lpCurPlayer->smCharInfo.bUpdateInfo[0]++;
	ReformCharForm();
	SendSetObjectSerial(lpCurPlayer->dwObjectSerial);

	if (RestartPlayCount < 350) RestartPlayCount = 350;

	return TRUE;
}


int GetAttackSpeedMainFrame(int AttackSpeed)
{
	int cnt;

	cnt = AttackSpeed - 6;
	if (cnt < 0) cnt = 0;
	if (cnt > 6) cnt = 6;

	return fONE + 32 * cnt;
}


int	GetAttackSpeedFrame(int AttackSpeed, int AddSpeed)
{
	int cnt;

	cnt = GetAttackSpeedMainFrame(AttackSpeed);
	if (AddSpeed < 6 && AddSpeed>0)
		cnt += AddSpeed * 32;

	return (80 * cnt) >> FLOATNS;
}


int	GetAttackSpeedFrame(int AttackSpeed)
{
	int cnt;

	cnt = GetAttackSpeedMainFrame(AttackSpeed);

	return (80 * cnt) >> FLOATNS;
}
