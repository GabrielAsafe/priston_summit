int GetItemKindFromBliankColor(smCHARTOOL	*lpCharTool)
{
	int cnt;

	lpCharTool->TexMixCode = -1;
	lpCharTool->TexScroll = 0;

	for (cnt = 0; cnt < CONST_AGINGBCOLOR_MAX; cnt++)
	{
		if (AgingBlinkColor[cnt][0] == lpCharTool->sColors[SMC_R] &&
			AgingBlinkColor[cnt][1] == lpCharTool->sColors[SMC_G] &&
			AgingBlinkColor[cnt][2] == lpCharTool->sColors[SMC_B] &&
			AgingBlinkColor[cnt][3] == lpCharTool->sColors[SMC_A])
		{

			lpCharTool->TexMixCode = AgingBlinkColor[cnt][4];
			lpCharTool->TexScroll = AgingBlinkColor[cnt][5];
			return TRUE;
		}
	}

	for (cnt = 0; cnt < 6; cnt++)
	{
		if (CraftBlinkColor[cnt][0] == lpCharTool->sColors[SMC_R] &&
			CraftBlinkColor[cnt][1] == lpCharTool->sColors[SMC_G] &&
			CraftBlinkColor[cnt][2] == lpCharTool->sColors[SMC_B] &&
			CraftBlinkColor[cnt][3] == lpCharTool->sColors[SMC_A])
		{

			lpCharTool->TexMixCode = CraftBlinkColor[cnt][4];
			lpCharTool->TexScroll = CraftBlinkColor[cnt][5];
			return TRUE;
		}
	}


	return NULL;

}
