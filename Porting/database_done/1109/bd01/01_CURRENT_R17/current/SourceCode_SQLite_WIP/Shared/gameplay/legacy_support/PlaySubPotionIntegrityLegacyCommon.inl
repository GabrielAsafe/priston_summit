sCHK_POTION_COUNT	sPotionTotalCount;
int	tXorPotionCount = 0;


int	XorCheckPotion(sCHK_POTION_COUNT *ChkPotion)
{
	ChkPotion->Pl101 ^= tXorPotionCount;
	ChkPotion->Pl102 ^= tXorPotionCount;
	ChkPotion->Pl103 ^= tXorPotionCount;
	ChkPotion->Pl104 ^= tXorPotionCount;

	ChkPotion->Pm101 ^= tXorPotionCount;
	ChkPotion->Pm102 ^= tXorPotionCount;
	ChkPotion->Pm103 ^= tXorPotionCount;
	ChkPotion->Pm104 ^= tXorPotionCount;

	ChkPotion->Ps101 ^= tXorPotionCount;
	ChkPotion->Ps102 ^= tXorPotionCount;
	ChkPotion->Ps103 ^= tXorPotionCount;
	ChkPotion->Ps104 ^= tXorPotionCount;

	return TRUE;
}


int	ComparePotion()
{
	sCHK_POTION_COUNT	ChkPotionCount;
	int	CompareError = 0;

	XorCheckPotion(&sPotionTotalCount);

	GetInvenPotionCount(&ChkPotionCount);

	if (ChkPotionCount.Pl101 > sPotionTotalCount.Pl101)
		CompareError++;
	if (ChkPotionCount.Pl102 > sPotionTotalCount.Pl102)
		CompareError++;
	if (ChkPotionCount.Pl103 > sPotionTotalCount.Pl103)
		CompareError++;
	if (ChkPotionCount.Pl104 > sPotionTotalCount.Pl104)
		CompareError++;

	if (ChkPotionCount.Pm101 > sPotionTotalCount.Pm101)
		CompareError++;
	if (ChkPotionCount.Pm102 > sPotionTotalCount.Pm102)
		CompareError++;
	if (ChkPotionCount.Pm103 > sPotionTotalCount.Pm103)
		CompareError++;
	if (ChkPotionCount.Pm104 > sPotionTotalCount.Pm104)
		CompareError++;

	if (ChkPotionCount.Ps101 > sPotionTotalCount.Ps101)
		CompareError++;
	if (ChkPotionCount.Ps102 > sPotionTotalCount.Ps102)
		CompareError++;
	if (ChkPotionCount.Ps103 > sPotionTotalCount.Ps103)
		CompareError++;
	if (ChkPotionCount.Ps104 > sPotionTotalCount.Ps104)
		CompareError++;

	int cnt1, cnt2;
	cnt1 = ChkPotionCount.Pl101 + ChkPotionCount.Pl102 + ChkPotionCount.Pl103 + ChkPotionCount.Pl104 +
		ChkPotionCount.Pm101 + ChkPotionCount.Pm102 + ChkPotionCount.Pm103 + ChkPotionCount.Pm104 +
		ChkPotionCount.Ps101 + ChkPotionCount.Ps102 + ChkPotionCount.Ps103 + ChkPotionCount.Ps104;

	cnt2 = sPotionTotalCount.Pl101 + sPotionTotalCount.Pl102 + sPotionTotalCount.Pl103 + sPotionTotalCount.Pl104 +
		sPotionTotalCount.Pm101 + sPotionTotalCount.Pm102 + sPotionTotalCount.Pm103 + sPotionTotalCount.Pm104 +
		sPotionTotalCount.Ps101 + sPotionTotalCount.Ps102 + sPotionTotalCount.Ps103 + sPotionTotalCount.Ps104;

	if (CompareError)
	{


		SendSetHackUser3(1530, CompareError, cnt1 - cnt2);
	}

	tXorPotionCount = dwPlayTime & 0xFFFFFFF;
	memcpy(&sPotionTotalCount, &ChkPotionCount, sizeof(sCHK_POTION_COUNT));
	XorCheckPotion(&sPotionTotalCount);

	return CompareError;
}




int	ResetPotion()
{
	GetInvenPotionCount(&sPotionTotalCount);
	tXorPotionCount = dwPlayTime & 0xFFFFFFF;
	XorCheckPotion(&sPotionTotalCount);

	return TRUE;
}


int	ResetPotion2()
{
	int cnt1, cnt2;
	sCHK_POTION_COUNT	ChkPotionCount;

	XorCheckPotion(&sPotionTotalCount);
	GetInvenPotionCount(&ChkPotionCount);


	cnt1 = ChkPotionCount.Pl101 + ChkPotionCount.Pl102 + ChkPotionCount.Pl103 + ChkPotionCount.Pl104 +
		ChkPotionCount.Pm101 + ChkPotionCount.Pm102 + ChkPotionCount.Pm103 + ChkPotionCount.Pm104 +
		ChkPotionCount.Ps101 + ChkPotionCount.Ps102 + ChkPotionCount.Ps103 + ChkPotionCount.Ps104;

	cnt2 = sPotionTotalCount.Pl101 + sPotionTotalCount.Pl102 + sPotionTotalCount.Pl103 + sPotionTotalCount.Pl104 +
		sPotionTotalCount.Pm101 + sPotionTotalCount.Pm102 + sPotionTotalCount.Pm103 + sPotionTotalCount.Pm104 +
		sPotionTotalCount.Ps101 + sPotionTotalCount.Ps102 + sPotionTotalCount.Ps103 + sPotionTotalCount.Ps104;

	if (cnt1 > (cnt2 + TransRecvItem.Item.PotionCount))
		return TRUE;

	memcpy(&sPotionTotalCount, &ChkPotionCount, sizeof(sCHK_POTION_COUNT));
	tXorPotionCount = dwPlayTime & 0xFFFFFFF;
	XorCheckPotion(&sPotionTotalCount);

	return FALSE;
}
