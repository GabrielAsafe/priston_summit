DWORD GetHairSkinCode(int FaceCode, int HairCode)
{
	return ((FaceCode & 0xFF) << 8) | (HairCode & 0xFF) | ((((FaceCode + 1) + (HairCode + 1) * 3) & 0xFF) << 16);
}


int CheckHairSkinCode(DWORD dwModelCode)
{
	if (dwModelCode)
	{
		int FaceCode = (dwModelCode >> 8) & 0xFF;
		int HairCode = dwModelCode & 0xFF;
		int Sum = (dwModelCode >> 16) & 0xFF;
		if ((((FaceCode + 1) + (HairCode + 1) * 3) & 0xFF) == Sum) return TRUE;
	}
	return FALSE;
}


char *GetFaceHairSkin(smCHAR *lpChar)
{

	DWORD dwModelCode = lpChar->smCharInfo.ModelNameCode2;
	int FaceCode = (dwModelCode >> 8) & 0xFF;
	int HairCode = dwModelCode & 0xFF;

	if (CheckHairSkinCode(dwModelCode) == FALSE) return NULL;

	char *szHeadName = 0;

	switch (lpChar->smCharInfo.JOB_CODE)
	{
	case 1:
		szHeadName = szModel_FighterFaceName[FaceCode][HairCode];
		break;
	case 2:
		szHeadName = szModel_MechFaceName[FaceCode][HairCode];
		break;
	case 3:
		szHeadName = szModel_ArcherFaceName[FaceCode][HairCode];
		break;
	case 4:
		szHeadName = szModel_PikeFaceName[FaceCode][HairCode];
		break;
	case 5:
		szHeadName = szModel_AtalantaFaceName[FaceCode][HairCode];
		break;
	case 6:
		szHeadName = szModel_KnightFaceName[FaceCode][HairCode];
		break;
	case 7:
		szHeadName = szModel_MagicanFaceName[FaceCode][HairCode];
		break;
	case 8:
		szHeadName = szModel_PristessFaceName[FaceCode][HairCode];
		break;

	}

	return szHeadName;
}


int FindFaceSkin(smCHAR *lpChar, char *lpHeadName, char ***lpSelFace, char **lpSelFaceName)
{
	int cnt, cnt2;
	int FindCount = 0;
	int FaceCode;
	int HairCode;

	switch (lpChar->smCharInfo.JOB_CODE)
	{
	case JOBCODE_FIGHTER:
		for (cnt = 0; cnt < FACE_MODEL_MAX; cnt++)
		{
			for (cnt2 = 0; cnt2 < HEAREPOTION_MAX; cnt2++)
			{
				if (lstrcmpi(szModel_FighterFaceName[cnt][cnt2], lpHeadName) == 0)
				{
					*lpSelFace = szModel_FighterFaceName[cnt];
					*lpSelFaceName = szModel_FighterFaceName[cnt][cnt2];
					FaceCode = cnt; HairCode = cnt2; FindCount++;
					break;
				}
			}
			if (cnt2 < HEAREPOTION_MAX) break;
		}
		break;
	case JOBCODE_MECHANICIAN:
		for (cnt = 0; cnt < FACE_MODEL_MAX; cnt++)
		{
			for (cnt2 = 0; cnt2 < HEAREPOTION_MAX; cnt2++)
			{
				if (lstrcmpi(szModel_MechFaceName[cnt][cnt2], lpHeadName) == 0)
				{
					*lpSelFace = szModel_MechFaceName[cnt];
					*lpSelFaceName = szModel_MechFaceName[cnt][cnt2];
					FaceCode = cnt; HairCode = cnt2; FindCount++;
					break;
				}
			}
			if (cnt2 < HEAREPOTION_MAX) break;
		}
		break;
	case JOBCODE_ARCHER:
		for (cnt = 0; cnt < FACE_MODEL_MAX; cnt++)
		{
			for (cnt2 = 0; cnt2 < HEAREPOTION_MAX; cnt2++)
			{
				if (lstrcmpi(szModel_ArcherFaceName[cnt][cnt2], lpHeadName) == 0)
				{
					*lpSelFace = szModel_ArcherFaceName[cnt];
					*lpSelFaceName = szModel_ArcherFaceName[cnt][cnt2];
					FaceCode = cnt; HairCode = cnt2; FindCount++;
					break;
				}
			}
			if (cnt2 < HEAREPOTION_MAX) break;
		}
		break;
	case JOBCODE_PIKEMAN:
		for (cnt = 0; cnt < FACE_MODEL_MAX; cnt++)
		{
			for (cnt2 = 0; cnt2 < HEAREPOTION_MAX; cnt2++)
			{
				if (lstrcmpi(szModel_PikeFaceName[cnt][cnt2], lpHeadName) == 0)
				{
					*lpSelFace = szModel_PikeFaceName[cnt];
					*lpSelFaceName = szModel_PikeFaceName[cnt][cnt2];
					FaceCode = cnt; HairCode = cnt2; FindCount++;
					break;
				}
			}
			if (cnt2 < HEAREPOTION_MAX) break;

		}
		break;
	case JOBCODE_ATALANTA:
		for (cnt = 0; cnt < FACE_MODEL_MAX; cnt++)
		{
			for (cnt2 = 0; cnt2 < HEAREPOTION_MAX; cnt2++)
			{
				if (lstrcmpi(szModel_AtalantaFaceName[cnt][cnt2], lpHeadName) == 0)
				{
					*lpSelFace = szModel_AtalantaFaceName[cnt];
					*lpSelFaceName = szModel_AtalantaFaceName[cnt][cnt2];
					FaceCode = cnt; HairCode = cnt2; FindCount++;
					break;
				}
			}
			if (cnt2 < HEAREPOTION_MAX) break;
		}
		break;
	case JOBCODE_KNIGHT:
		for (cnt = 0; cnt < FACE_MODEL_MAX; cnt++)
		{
			for (cnt2 = 0; cnt2 < HEAREPOTION_MAX; cnt2++)
			{
				if (lstrcmpi(szModel_KnightFaceName[cnt][cnt2], lpHeadName) == 0)
				{
					*lpSelFace = szModel_KnightFaceName[cnt];
					*lpSelFaceName = szModel_KnightFaceName[cnt][cnt2];
					FaceCode = cnt; HairCode = cnt2; FindCount++;
					break;
				}
			}
			if (cnt2 < HEAREPOTION_MAX) break;
		}
		break;
	case JOBCODE_MAGICIAN:
		for (cnt = 0; cnt < FACE_MODEL_MAX; cnt++)
		{
			for (cnt2 = 0; cnt2 < HEAREPOTION_MAX; cnt2++)
			{
				if (lstrcmpi(szModel_MagicanFaceName[cnt][cnt2], lpHeadName) == 0)
				{
					*lpSelFace = szModel_MagicanFaceName[cnt];
					*lpSelFaceName = szModel_MagicanFaceName[cnt][cnt2];
					FaceCode = cnt; HairCode = cnt2; FindCount++;
					break;
				}
			}
			if (cnt2 < HEAREPOTION_MAX) break;
		}
		break;
	case JOBCODE_PRIESTESS:
		for (cnt = 0; cnt < FACE_MODEL_MAX; cnt++)
		{
			for (cnt2 = 0; cnt2 < HEAREPOTION_MAX; cnt2++)
			{
				if (lstrcmpi(szModel_PristessFaceName[cnt][cnt2], lpHeadName) == 0)
				{
					*lpSelFace = szModel_PristessFaceName[cnt];
					*lpSelFaceName = szModel_PristessFaceName[cnt][cnt2];
					FaceCode = cnt; HairCode = cnt2; FindCount++;
					break;
				}
			}
			if (cnt2 < HEAREPOTION_MAX) break;
		}
		break;

	}

	if (!FindCount)
		return -1;

	return (FaceCode << 8) | HairCode;
}
