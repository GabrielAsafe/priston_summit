INT64 CodeXorCharInfo_Exp()
{
	lpCurPlayer->smCharInfo.Exp ^= CodeXorExp;
	lpCurPlayer->smCharInfo.Exp_High ^= CodeXorExp_High;

	INT64 exp1 = lpCurPlayer->smCharInfo.Exp;
	INT64 exp2 = lpCurPlayer->smCharInfo.Exp_High;

	return (exp2 << 32) | (exp1 & 0xFFFFFFFF);
}
