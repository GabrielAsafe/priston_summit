int LoadFieldMap(int PosNum, sFIELD *lpField, smSTAGE3D *lpStage)
{
	int ReadTexFlag = 0;

	if (!lpField || !lpStage || sCompactMap[PosNum].lpField == lpField)
		return TRUE;

	sCompactMap[PosNum].lpField = lpField;

	memcpy(&sCompactMap[PosNum].StageRect, &lpStage->StageMapRect, sizeof(RECT));

	if (sCompactMap[PosNum].hTexHandle)
	{
		smTexture.Delete(sCompactMap[PosNum].hTexHandle);
		sCompactMap[PosNum].hTexHandle = 0;
	}
	if (sCompactMap[PosNum].hTexTitleHandle)
	{
		smTexture.Delete(sCompactMap[PosNum].hTexTitleHandle);
		sCompactMap[PosNum].hTexTitleHandle = 0;
	}


	if (lpField->szNameMap[0])
	{
		sCompactMap[PosNum].hTexHandle = smTexture.Add(lpField->szNameMap, 0);
		sCompactMap[PosNum].hTexHandle->TexSwapMode = FALSE;

		sCompactMap[PosNum].rx = (lpStage->StageMapRect.right - lpStage->StageMapRect.left) >> FLOATNS;
		sCompactMap[PosNum].ry = (lpStage->StageMapRect.bottom - lpStage->StageMapRect.top) >> FLOATNS;

		ReadTexFlag = TRUE;
	}
	else
	{
		sCompactMap[PosNum].hTexHandle = 0;
		return FALSE;
	}

	if (lpField->szNameTitle[0])
	{
		sCompactMap[PosNum].hTexTitleHandle = smTexture.Add(lpField->szNameTitle, 0);
		sCompactMap[PosNum].hTexTitleHandle->TexSwapMode = FALSE;

		ReadTexFlag = TRUE;
	}
	else
	{
		sCompactMap[PosNum].hTexTitleHandle = 0;
		return FALSE;
	}

	if (ReadTexFlag)	ReadTextures();

	return TRUE;
}

int psDrawTexImage(smTEXTUREHANDLE *hTexHandle, float x, float y, float w, float h,
	float fx, float fy, float fw, float fh, int Transp, int specular);

int psDrawTexImage_Point(smTEXTUREHANDLE *hTexHandle, float x, float y, float w, float h,
	float fx, float fy, float fw, float fh, int Transp, int specular);

int psDrawTexImage2(smTEXTUREHANDLE *hTexHandle, float *x, float *y,
	float fx, float fy, float fw, float fh, int Transp, int specular);

int DrawMapArrow(int x, int y, int Angle);
int psSetDefaultRenderState();


int DrawFieldMap(sCOMPACT_MAP *lpCompactMap, int px, int py, int size);

int FieldMapDrawMode = 2;

int FieldMapDrawSize[3] = { 8 , 16 , 24 };
int FieldMapViewSize1 = 18 * 64 * fONE;
int FieldMapViewSize2 = 8 * 64 * fONE;


int MainFieldMap()
{
	if (FieldMapViewSize1 > FieldMapViewSize2)
	{
		FieldMapViewSize2 += 2048;
		if (FieldMapViewSize1 < FieldMapViewSize2)
			FieldMapViewSize2 = FieldMapViewSize1;
	}

	if (FieldMapViewSize1 < FieldMapViewSize2)
	{
		FieldMapViewSize2 -= 2048;
		if (FieldMapViewSize1 > FieldMapViewSize2)
			FieldMapViewSize2 = FieldMapViewSize1;
	}


	return TRUE;
}

extern int MatEachBox;

int	DrawMapPartyPlayer(int px, int py);

int	DrawMapNPC(int px, int py);


int	GetFieldMapCursor()
{
	int px = 656 + (WinSizeX - 800);
	int py = 426 + (WinSizeY - 600);

	if (WinSizeX <= 800 && cSkill.OpenFlag) return FALSE;

	if (pCursorPos.x > px && pCursorPos.x<px + 128 && pCursorPos.y>py && pCursorPos.y < py + 128 && lpCurPlayer->dwTradeMsgCode == 0)
	{

		if (lpCurPlayer->OnStageField >= 0 &&
			(StageField[lpCurPlayer->OnStageField]->FieldCode == rsSOD_FIELD || StageField[lpCurPlayer->OnStageField]->FieldCode == QUEST_ARENA_FIELD))
		{

			return FALSE;
		}

		return TRUE;
	}

	return FALSE;
}
