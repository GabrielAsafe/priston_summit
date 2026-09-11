int	Sod_SodEffect(int Round)
{

	if (!BellatraEffectInitFlag) return FALSE;

	SetBellatraFontEffect(E_BL_FONT_ROUND);

	switch (Round)
	{
	case 1:
		SetBellatraFontEffect(E_BL_FONT_COUNT_1);
		break;
	case 2:
		SetBellatraFontEffect(E_BL_FONT_COUNT_2);
		break;
	case 3:
		SetBellatraFontEffect(E_BL_FONT_COUNT_3);
		break;
	case 4:
		SetBellatraFontEffect(E_BL_FONT_COUNT_4);
		break;
	case 5:
		SetBellatraFontEffect(E_BL_FONT_COUNT_5);
		break;
	case 6:
		SetBellatraFontEffect(E_BL_FONT_COUNT_6);
		break;
	case 7:
		SetBellatraFontEffect(E_BL_FONT_COUNT_7);
		break;
	case 8:
		SetBellatraFontEffect(E_BL_FONT_COUNT_8);
		break;
	}
	SetBellatraFontEffect(E_BL_FONT_START);

	esPlayContSound(Round);



	return TRUE;
}



SBL_SetFontEffect SoD_SetFontEffect_CloseScreen;
char	szSOD_String[64] = { 0, };
BYTE	bSOD_StringColor[4];
int		SOD_StringCount = 0;
int		TeamScoreSort[4];
int		TeamMarkSort[4] = { 3 , 0 , 2 , 1 };
int		SoDGateFlag;





int	SetSodScoreResult()
{
	int cnt;

	g_EventScore.TeamNum = 0;
	g_EventScore.EntryTeamNum = 0;

	for (cnt = 0; cnt < 4; cnt++)
	{
		g_EventScore.TeamMarkBuff[cnt] = TeamMarkSort[TeamScoreSort[cnt]];
		wsprintf(g_EventScore.szTeamPointBuff[cnt], "%d", sSodScore.TeamScore[TeamScoreSort[cnt]]);
	}
	for (cnt = 0; cnt < 2; cnt++)
	{
		g_EventScore.EntryTeamMarkBuff[cnt] = g_EventScore.TeamMarkBuff[cnt];
	}

	if (sSodScore.Round > 0 && sSodScore.Round <= 4)
	{
		g_EventScore.TeamNum = 4;
		g_EventScore.EntryTeamNum = 2;
	}
	if (sSodScore.Round > 4 && sSodScore.Round <= 6)
	{
		g_EventScore.TeamNum = 2;
		g_EventScore.EntryTeamNum = 1;
	}

	if (!g_EventScore.TeamNum) return FALSE;

	return TRUE;
}
