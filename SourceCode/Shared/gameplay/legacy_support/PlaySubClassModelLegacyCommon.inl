static const char *ResolveLegacyArmorFileName(int legacyJobCode,
                                              gameplay::classes::ArmorVisualVariant variant,
                                              int armorIndex)
{
	int armorModelIndex = 0;
	if(!gameplay::classes::DefaultClassService().TryArmorModelIndex(legacyJobCode, armorModelIndex))
		return NULL;
	if(armorModelIndex < 0 || armorModelIndex >= LEGACY_ARMOR_MODEL_ROW_COUNT ||
		armorIndex < 0 || armorIndex >= ARMOR_FILE_MAX)
		return NULL;

	switch(variant)
	{
	case gameplay::classes::ArmorVisualVariant::Comic:
		return szArmorNumFile_Comic[armorModelIndex][armorIndex];
	case gameplay::classes::ArmorVisualVariant::NewSet:
		return szArmorNumFile_New[armorModelIndex][armorIndex];
	case gameplay::classes::ArmorVisualVariant::NewSet2:
		return szArmorNumFile_New_2[armorModelIndex][armorIndex];
	case gameplay::classes::ArmorVisualVariant::Normal:
	default:
		return szArmorNumFile[armorModelIndex][armorIndex];
	}
}

static int TryResolveClassArmorPath(int legacyJobCode,
                                    gameplay::classes::ArmorVisualVariant variant,
                                    int armorIndex,
                                    std::string &outPath)
{
	if(armorIndex < 0)
		return FALSE;

	const gameplay::classes::ClassService &classService = gameplay::classes::DefaultClassService();
	const gameplay::classes::PlayableClassDefinition *definition = classService.FindEnabled(legacyJobCode);
	if(!definition || !definition->armor.IsDefined())
		return FALSE;

	if(definition->armor.HasDataDrivenModels())
	{
		const std::string *fileName = classService.ArmorModel(legacyJobCode, variant,
			static_cast<std::size_t>(armorIndex));
		if(!fileName || fileName->empty())
			return FALSE;
		outPath = definition->armor.modelRoot;
		outPath += *fileName;
		return TRUE;
	}

	if(armorIndex >= ARMOR_FILE_MAX)
		armorIndex = ARMOR_FILE_MAX - 1;
	const char *legacyFile = ResolveLegacyArmorFileName(legacyJobCode, variant, armorIndex);
	if(!legacyFile || !legacyFile[0])
		return FALSE;
	outPath = szArmorDirectory;
	outPath += legacyFile;
	return TRUE;
}

static int TryFindLegacyArmorModel(int legacyJobCode,
                                   const char *fileName,
                                   gameplay::classes::ArmorVisualVariant &variant,
                                   int &armorIndex)
{
	if(!fileName || !fileName[0])
		return FALSE;

	const gameplay::classes::ArmorVisualVariant variants[] = {
		gameplay::classes::ArmorVisualVariant::Normal,
		gameplay::classes::ArmorVisualVariant::Comic,
		gameplay::classes::ArmorVisualVariant::NewSet,
		gameplay::classes::ArmorVisualVariant::NewSet2
	};
	for(std::size_t v = 0; v < sizeof(variants) / sizeof(variants[0]); ++v)
	{
		for(int index = 0; index < ARMOR_FILE_MAX; ++index)
		{
			const char *candidate = ResolveLegacyArmorFileName(legacyJobCode, variants[v], index);
			if(candidate && candidate[0] && lstrcmpi(candidate, fileName) == 0)
			{
				variant = variants[v];
				armorIndex = index;
				return TRUE;
			}
		}
	}
	return FALSE;
}

static int TryFindClassArmorModel(int legacyJobCode,
                                  const char *fileName,
                                  gameplay::classes::ArmorVisualVariant &variant,
                                  int &armorIndex)
{
	if(!fileName || !fileName[0])
		return FALSE;

	const gameplay::classes::PlayableClassDefinition *definition =
		gameplay::classes::DefaultClassService().FindEnabled(legacyJobCode);
	if(!definition)
		return FALSE;

	if(definition->armor.HasDataDrivenModels())
	{
		std::size_t index = 0u;
		if(!gameplay::classes::DefaultClassService().FindArmorModel(legacyJobCode, fileName, variant, index) ||
			index > static_cast<std::size_t>(0x7FFFFFFF))
			return FALSE;
		armorIndex = static_cast<int>(index);
		return TRUE;
	}

	return TryFindLegacyArmorModel(legacyJobCode, fileName, variant, armorIndex);
}

static std::vector<int> EnabledLegacyArmorModelRows()
{
	std::vector<int> rows;
	const std::vector<const gameplay::classes::PlayableClassDefinition*> enabled =
		gameplay::classes::DefaultClassService().EnabledClasses();
	for(std::vector<const gameplay::classes::PlayableClassDefinition*>::const_iterator definition = enabled.begin();
		definition != enabled.end(); ++definition)
	{
		const int row = (*definition)->armor.legacyModelIndex;
		if(row < 0 || row >= LEGACY_ARMOR_MODEL_ROW_COUNT)
			continue;
		bool exists = false;
		for(std::vector<int>::const_iterator it = rows.begin(); it != rows.end(); ++it)
		{
			if(*it == row)
			{
				exists = true;
				break;
			}
		}
		if(!exists)
			rows.push_back(row);
	}
	return rows;
}

static int LoadLegacyClassArmorModelSet(gameplay::classes::ArmorVisualVariant variant,
									int firstIndex, int lastIndex,
									int skipOneToNine)
{
	int errorCount = 0;
	char path[128];
	const std::vector<int> rows = EnabledLegacyArmorModelRows();
	for(std::vector<int>::const_iterator row = rows.begin(); row != rows.end(); ++row)
	{
		for(int armorIndex = firstIndex; armorIndex < lastIndex; ++armorIndex)
		{
			if(skipOneToNine && armorIndex > 0 && armorIndex < 10)
				continue;
			const char *fileName = 0;
			switch(variant)
			{
			case gameplay::classes::ArmorVisualVariant::Comic:
				fileName = szArmorNumFile_Comic[*row][armorIndex];
				break;
			case gameplay::classes::ArmorVisualVariant::NewSet:
				fileName = szArmorNumFile_New[*row][armorIndex];
				break;
			case gameplay::classes::ArmorVisualVariant::NewSet2:
				fileName = szArmorNumFile_New_2[*row][armorIndex];
				break;
			case gameplay::classes::ArmorVisualVariant::Normal:
			default:
				fileName = szArmorNumFile[*row][armorIndex];
				break;
			}
			if(!fileName || !fileName[0])
				continue;
			strcpy_s(path, szArmorDirectory);
			lstrcat(path, fileName);
			if(LoadOftenMeshPattern(path) == FALSE)
				errorCount++;
		}
	}
	return errorCount;
}

