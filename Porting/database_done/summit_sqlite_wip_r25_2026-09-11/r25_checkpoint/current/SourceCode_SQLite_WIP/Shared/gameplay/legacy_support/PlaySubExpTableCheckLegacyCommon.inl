INT64 CheckLevelTable()
{
	INT64 cnt = 0;
	INT64 ChkSum = 0;

	while (1)
	{
		if (ExpLevelTable[cnt] == -1) break;
		ChkSum += ExpLevelTable[cnt] * (cnt + 1);
		cnt++;
	}

	return ChkSum;
}
