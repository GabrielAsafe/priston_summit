int ChangeHairModel(int HairCode)
{

	smCHAR *lpChar = lpCurPlayer;
	char *lpHeadName = lpChar->smCharInfo.szModelName2;
	char *lpSelFaceName = 0;

	if (HairCode < 0 || HairCode > 0xFF)
		return FALSE;

	int FindFaceValue = FindFaceSkin(lpChar, lpHeadName, 0, &lpSelFaceName);

	if (FindFaceValue < 0)
		return FALSE;

	int FaceCode = FindFaceValue >> 8;
	char *nextHeadModel = ResolveClassHeadModel(lpChar->smCharInfo.JOB_CODE, FaceCode, HairCode);
	if(!nextHeadModel)
		return FALSE;

	if (lstrcmpi(lpChar->smCharInfo.szModelName2, nextHeadModel) == 0)
		return FALSE;

	strcpy_s(lpChar->smCharInfo.szModelName2, nextHeadModel);
	lpChar->smCharInfo.ModelNameCode2 = GetHairSkinCode(FaceCode, HairCode);

	lpChar->PatLoading = TRUE;
	AddLoaderPattern(lpChar, lpChar->smCharInfo.szModelName, lpChar->smCharInfo.szModelName2);

	lpCurPlayer->smCharInfo.bUpdateInfo[0]++;
	ReformCharForm();
	SendSetObjectSerial(lpCurPlayer->dwObjectSerial);
	SaveGameData();

	StartSkill(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, 0, 0, SKILL_UP1);
	SkillPlaySound(SKILL_SOUND_LEARN, lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ);


	return TRUE;
}
