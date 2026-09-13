int	SodScoreMain()
{
	int dx, dy, dz;
	int x, z, cnt;




	if (!SodScoreFlag)
	{
		SodScoreFlag = TRUE;
		ZeroMemory(&sSodScore, sizeof(sSOD_SCORE));
	}

	if (SOD_StringCount) SOD_StringCount--;


	if (sSodScore.dwSoD_NextStageTime)
	{

		if (sSodScore.dwSoD_CloseStageTime && sSodScore.dwSoD_CloseStageTime < dwPlayTime)
		{

			if (SoD_SetFontEffect_CloseScreen.eBL_Type != E_BL_CHANGE_TRUE)
			{

				if (SoDGateFlag == TRUE && SetSodScoreResult() == TRUE)
					SetBellatraFontEffect(E_BL_CHANGE_TRUE, &SoD_SetFontEffect_CloseScreen, E_BL_EVENT_SCORE);
				else
					SetBellatraFontEffect(E_BL_CHANGE_TRUE, &SoD_SetFontEffect_CloseScreen);


				esPlaySound(18, 400);
			}

			sSodScore.dwSoD_CloseStageTime = 0;
		}

		if (sSodScore.dwSoD_NextStageTime < dwPlayTime)
		{

			if (SoD_SetFontEffect_CloseScreen.eBL_Type == E_BL_CHANGE_TRUE)
			{
				CheckBellatraFontEffect(&SoD_SetFontEffect_CloseScreen, TRUE);
				SetBellatraFontEffect(E_BL_CHANGE_FALSE, &SoD_SetFontEffect_CloseScreen);
				esPlaySound(18, 400);
			}

			if (sSodScore.SodNextStageNum >= 0 && lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_DEAD)
			{
				if (rsCheckHardCoreEventArea(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ) == TRUE && sSodScore.SodNextStageNum < 100)
				{


					dx = rsHardCoreEventOpenPos[sSodScore.SodNextStageNum][2].x*fONE;
					dy = rsHardCoreEventOpenPos[sSodScore.SodNextStageNum][2].y*fONE;
					dz = rsHardCoreEventOpenPos[sSodScore.SodNextStageNum][2].z*fONE;

					CloseEachPlayer();

					WarpFieldNearPos(rsSOD_FIELD, dx, dz, &x, &z);

					lpCurPlayer->SetPosi(dx, 0, dz, 0, 0, 0);
					TraceCameraPosi.x = lpCurPlayer->pX;
					TraceCameraPosi.y = lpCurPlayer->pY;
					TraceCameraPosi.z = lpCurPlayer->pZ;
					TraceTargetPosi.x = lpCurPlayer->pX;
					TraceTargetPosi.y = lpCurPlayer->pY;
					TraceTargetPosi.z = lpCurPlayer->pZ;


					lpCurPlayer->OnStageField = -1;
					if (smGameStage[0])
					{
						cnt = smGameStage[0]->GetFloorHeight(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, lpCurPlayer->Pattern->SizeHeight);
						lpCurPlayer->OnStageField = 0;
					}
					if (cnt == CLIP_OUT && smGameStage[1])
					{
						lpCurPlayer->OnStageField = 1;
					}

					sSodScore.EffectStartCount = 70 * 1;
					sSodScore.EffectCode = sSodScore.NextRound;
				}
			}
			else if (sSodScore.SodNextStageNum == -1)
			{

				dx = sField[rsSOD_VILLAGE].cX*fONE;
				dz = sField[rsSOD_VILLAGE].cZ*fONE;

				CloseEachPlayer();

				WarpFieldNearPos(rsSOD_VILLAGE, dx, dz, &x, &z);

				lpCurPlayer->SetPosi(dx, 0, dz, 0, 0, 0);
				TraceCameraPosi.x = lpCurPlayer->pX;
				TraceCameraPosi.y = lpCurPlayer->pY;
				TraceCameraPosi.z = lpCurPlayer->pZ;
				TraceTargetPosi.x = lpCurPlayer->pX;
				TraceTargetPosi.y = lpCurPlayer->pY;
				TraceTargetPosi.z = lpCurPlayer->pZ;


				lpCurPlayer->OnStageField = -1;
				if (smGameStage[0])
				{
					cnt = smGameStage[0]->GetFloorHeight(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, lpCurPlayer->Pattern->SizeHeight);
					lpCurPlayer->OnStageField = 0;
				}
				if (cnt == CLIP_OUT && smGameStage[1])
				{
					lpCurPlayer->OnStageField = 1;
				}
			}
			else if (sSodScore.SodNextStageNum == -2)
			{

				if (SoD_SetFontEffect.eBL_Type == E_BL_LODING)
				{
					CheckBellatraFontEffect(&SoD_SetFontEffect, TRUE);
				}
			}

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_STAND);
			lpCurPlayer->dwNextMotionCode = 0;
			sSodScore.dwSoD_NextStageTime = 0;
		}
		else
		{
			if (sSodScore.SodNextStageNum >= 0)
				lpCurPlayer->dwNextMotionCode = CHRMOTION_STATE_YAHOO;
		}

	}

	if (sSodScore.EffectStartCount)
	{
		sSodScore.EffectStartCount--;
		if (sSodScore.EffectStartCount == 0)
		{
			switch (sSodScore.EffectCode)
			{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:

				Sod_SodEffect(sSodScore.EffectCode);
				break;
			}
		}
	}


	if (sSodScore.ScoreEffectCount)
	{
		sSodScore.ScoreEffectCount -= 8;
		if (sSodScore.ScoreEffectCount < 0)
			sSodScore.ScoreEffectCount = 0;
	}


	if (rsBlessCastle.CastleMode == 3)
	{
		if (rsBlessCastle.dwMasterClan && rsBlessCastle.dwMasterClan == GetClanCode(lpCurPlayer->smCharInfo.ClassClan))
		{
			if (!lpCurPlayer->dwNextMotionCode)
			{
				lpCurPlayer->dwNextMotionCode = CHRMOTION_STATE_YAHOO;
			}
		}
	}


	return TRUE;

}




int SodTeamSort()
{
	int cnt, cnt1;
	int r1, r2;

	for (cnt = 0; cnt < 4; cnt++) TeamScoreSort[cnt] = cnt;



	for (cnt = 0; cnt < 4; cnt++)
	{
		for (cnt1 = cnt; cnt1 < 4; cnt1++)
		{
			r1 = TeamScoreSort[cnt];
			r2 = TeamScoreSort[cnt1];

			if (sSodScore.TeamScore[r1] < sSodScore.TeamScore[r2])
			{
				TeamScoreSort[cnt] = r2;
				TeamScoreSort[cnt1] = r1;
			}
		}
	}

	return TRUE;
}
