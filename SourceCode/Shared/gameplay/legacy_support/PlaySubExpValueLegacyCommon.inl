int	GetLevelFromExp(INT64 Exp)
{
	for (int cnt = 0; cnt < CHAR_LEVEL_MAX; cnt++)
	{
		if (Exp >= ExpLevelTable[cnt] && Exp < ExpLevelTable[cnt + 1])
		{
			return cnt + 1;
		}
	}

	return NULL;
}


INT64 GetExp64(smCHAR_INFO *smCharInfo)
{
	INT64 exp1, exp2;
	exp1 = (INT64)smCharInfo->Exp;

#ifdef _MODE_EXP64
	exp2 = (INT64)smCharInfo->Exp_High;
#else
	exp2 = 0;
#endif

	return (exp2 << 32) | (exp1 & 0xFFFFFFFF);
}


INT64 GetXorExp64(smCHAR_INFO *smCharInfo)
{
	INT64 exp1, exp2;

	exp1 = (INT64)(smCharInfo->Exp^CodeXorExp);
	exp2 = (INT64)(smCharInfo->Exp_High^CodeXorExp_High);

	return (exp2 << 32) | (exp1 & 0xFFFFFFFF);
}



int	SetExp64(smCHAR_INFO *smCharInfo, INT64 Exp64)
{
#ifdef _MODE_EXP64
	smCharInfo->Exp_High = (int)(Exp64 >> 32);
#endif
	smCharInfo->Exp = (int)(Exp64 & 0xFFFFFFFF);

	return smCharInfo->Exp;
}
