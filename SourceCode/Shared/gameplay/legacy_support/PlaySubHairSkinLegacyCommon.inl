static char *ResolveLegacyHeadModelForClass(int legacyJobCode, int faceCode, int hairCode)
{
	if(faceCode < 0 || faceCode >= FACE_MODEL_MAX || hairCode < 0 || hairCode >= HEAREPOTION_MAX)
		return NULL;

	const gameplay::classes::PlayableClassDefinition *definition =
		gameplay::classes::DefaultClassService().FindEnabled(legacyJobCode);
	if(!definition)
		return NULL;

	const std::string &catalog = definition->appearance.legacyCatalogKey;
	if(catalog == "fighter") return szModel_FighterFaceName[faceCode][hairCode];
	if(catalog == "mechanician") return szModel_MechFaceName[faceCode][hairCode];
	if(catalog == "archer") return szModel_ArcherFaceName[faceCode][hairCode];
	if(catalog == "pikeman") return szModel_PikeFaceName[faceCode][hairCode];
	if(catalog == "atalanta") return szModel_AtalantaFaceName[faceCode][hairCode];
	if(catalog == "knight") return szModel_KnightFaceName[faceCode][hairCode];
	if(catalog == "magician") return szModel_MagicanFaceName[faceCode][hairCode];
	if(catalog == "priestess") return szModel_PristessFaceName[faceCode][hairCode];
	return NULL;
}

static char *ResolveClassHeadModel(int legacyJobCode, int faceCode, int hairCode)
{
	if(faceCode < 0 || hairCode < 0)
		return NULL;

	const std::string *dataDriven = gameplay::classes::DefaultClassService().HeadModel(
		legacyJobCode, static_cast<std::size_t>(faceCode), static_cast<std::size_t>(hairCode));
	if(dataDriven && !dataDriven->empty())
		return const_cast<char *>(dataDriven->c_str());

	return ResolveLegacyHeadModelForClass(legacyJobCode, faceCode, hairCode);
}

static char *ResolveDefaultClassHeadModel(int legacyJobCode, int rankIndex)
{
	if(rankIndex < 0)
		return NULL;

	const std::string *dataDriven = gameplay::classes::DefaultClassService().DefaultHeadModel(
		legacyJobCode, static_cast<std::size_t>(rankIndex));
	if(dataDriven && !dataDriven->empty())
		return const_cast<char *>(dataDriven->c_str());

	// Historical classes encode the default head for each change-job rank in
	// the first column of the legacy matrix. Keep that convention isolated here.
	return ResolveLegacyHeadModelForClass(legacyJobCode, rankIndex, 0);
}

static int FindClassHeadModel(int legacyJobCode, const char *headName, int &faceCode, int &hairCode)
{
	if(!headName || !headName[0])
		return FALSE;

	std::size_t dataFace = 0u;
	std::size_t dataHair = 0u;
	if(gameplay::classes::DefaultClassService().FindHeadModel(
		legacyJobCode, headName, dataFace, dataHair))
	{
		if(dataFace > 0xFFu || dataHair > 0xFFu)
			return FALSE;
		faceCode = static_cast<int>(dataFace);
		hairCode = static_cast<int>(dataHair);
		return TRUE;
	}

	for(int face = 0; face < FACE_MODEL_MAX; ++face)
	{
		for(int hair = 0; hair < HEAREPOTION_MAX; ++hair)
		{
			char *candidate = ResolveLegacyHeadModelForClass(legacyJobCode, face, hair);
			if(candidate && candidate[0] && lstrcmpi(candidate, headName) == 0)
			{
				faceCode = face;
				hairCode = hair;
				return TRUE;
			}
		}
	}

	return FALSE;
}


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
	if(!lpChar)
		return NULL;

	DWORD dwModelCode = lpChar->smCharInfo.ModelNameCode2;
	int FaceCode = (dwModelCode >> 8) & 0xFF;
	int HairCode = dwModelCode & 0xFF;

	if (CheckHairSkinCode(dwModelCode) == FALSE) return NULL;
	if (FaceCode < 0 || FaceCode >= FACE_MODEL_MAX || HairCode < 0 || HairCode >= HEAREPOTION_MAX) return NULL;
	return ResolveClassHeadModel(lpChar->smCharInfo.JOB_CODE, FaceCode, HairCode);
}


char *GetDefaultFaceHairSkin(smCHAR *lpChar)
{
	if (!lpChar) return NULL;
	switch (lpChar->smCharInfo.JOB_CODE)
	{
	case 1: return szModel_FighterFaceName[0][0];
	case 2: return szModel_MechFaceName[0][0];
	case 3: return szModel_ArcherFaceName[0][0];
	case 4: return szModel_PikeFaceName[0][0];
	case 5: return szModel_AtalantaFaceName[0][0];
	case 6: return szModel_KnightFaceName[0][0];
	case 7: return szModel_MagicanFaceName[0][0];
	case 8: return szModel_PristessFaceName[0][0];
	default: return NULL;
	}
}
int FindFaceSkin(smCHAR *lpChar, char *lpHeadName, char ***lpSelFace, char **lpSelFaceName)
{
	if(lpSelFace) *lpSelFace = 0;
	if(lpSelFaceName) *lpSelFaceName = 0;
	if(!lpChar || !lpHeadName || !lpHeadName[0])
		return -1;

	int faceCode = 0;
	int hairCode = 0;
	if(!FindClassHeadModel(lpChar->smCharInfo.JOB_CODE, lpHeadName, faceCode, hairCode))
		return -1;

	char *resolved = ResolveClassHeadModel(lpChar->smCharInfo.JOB_CODE, faceCode, hairCode);
	if(lpSelFaceName) *lpSelFaceName = resolved;
	return (faceCode << 8) | hairCode;
}
