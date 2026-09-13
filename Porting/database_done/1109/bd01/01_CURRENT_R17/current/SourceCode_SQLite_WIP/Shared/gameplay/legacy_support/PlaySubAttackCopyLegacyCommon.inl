int	PlayAttackCount = 0;
int	MotionAttackCount = 0;



int	AttackRecvCountUp()
{
	PlayAttackCount++;
	return PlayAttackCount;
}


int AttackMotionCountUp()
{
	MotionAttackCount++;
	if (MotionAttackCount > 30)
	{

		CheckPlayAttackCount();
	}

	return MotionAttackCount;
}


int	CheckPlayAttackCount()
{

	if (MotionAttackCount > 30 && PlayAttackCount <= 3)
	{


	}

	MotionAttackCount = 0;
	PlayAttackCount = 0;

	return TRUE;
}

struct	sCOPY_ITEM_INFO
{
	DWORD	dwItemCode;
	int		ChkSum;
	int		Header;
};

sCOPY_ITEM_INFO	sCopyItemList[] = {

	{ 0,0,0 }

};


int	CheckCopyItem(sITEMINFO *lpItem)
{
	int cnt;

	cnt = 0;

	if (!lpItem->ItemName[0])	return FALSE;
	if (lpItem->SpecialItemFlag[0])	return FALSE;

	if (!lpItem->BackUpKey || !lpItem->BackUpChkSum)
	{
		lpItem->BackUpKey = lpItem->ItemHeader.Head;
		lpItem->BackUpChkSum = lpItem->ItemHeader.dwChkSum;
	}

	while (1)
	{
		if (sCopyItemList[cnt].dwItemCode == 0) break;


		if (sCopyItemList[cnt].dwItemCode == lpItem->CODE)
		{
			if (sCopyItemList[cnt].ChkSum == (int)lpItem->ItemHeader.dwChkSum &&
				sCopyItemList[cnt].Header == (int)lpItem->ItemHeader.Head)
			{

				lpItem->SpecialItemFlag[0] = CHECK_COPY_ITEM;
				return TRUE;
			}
			if (sCopyItemList[cnt].ChkSum == (int)lpItem->BackUpChkSum &&
				sCopyItemList[cnt].Header == (int)lpItem->BackUpKey)
			{

				lpItem->SpecialItemFlag[0] = CHECK_COPY_ITEM;
				return TRUE;
			}
		}
		cnt++;
	}


	return FALSE;
}
