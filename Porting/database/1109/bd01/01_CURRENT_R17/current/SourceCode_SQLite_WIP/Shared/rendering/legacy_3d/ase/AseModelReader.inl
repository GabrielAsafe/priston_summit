#pragma once

void  smASE_SetPhysique(smPAT3D *p)
{
	smPhysique = p;
}


smPAT3D *smASE_GetPhysique()
{
	return smPhysique;
}

char szFileModelBip[] = "smb";
char szFileModel[] = "smd";

static const int ASE_STAGE_MAX_OBJECTS = 2048;
static const int ASE_STAGE_MAX_LIGHTS = 256;

namespace
{

bool QueryLegacyFileInfo(const char *fileName, WIN32_FIND_DATA *fileInfo)
{
	if(!fileName || !fileName[0] || !fileInfo)
		return false;

	HANDLE handle = FindFirstFile(fileName, fileInfo);
	if(handle == INVALID_HANDLE_VALUE)
		return false;
	FindClose(handle);
	return true;
}

bool IsCachedFileCurrent(const WIN32_FIND_DATA &cacheInfo, const char *sourceFile)
{
	WIN32_FIND_DATA sourceInfo;
	if(!QueryLegacyFileInfo(sourceFile, &sourceInfo))
		return true;
	return CompareFileTime(&cacheInfo.ftLastWriteTime, &sourceInfo.ftLastWriteTime) >= 0;
}

} // anonymous namespace

char *smFindFile(char *szfile, char *FileExt, DWORD *lpFileLen)
{
	if(!szfile || !szfile[0])
		return FALSE;

	char *candidate = FileExt ? ChangeFileExt(szfile, FileExt) : szfile;
	if(!candidate || !candidate[0])
		return FALSE;

	WIN32_FIND_DATA candidateInfo;
	if(!QueryLegacyFileInfo(candidate, &candidateInfo))
		return FALSE;

	if(lpFileLen)
		*lpFileLen = candidateInfo.nFileSizeLow;
	if(!FileExt)
		return candidate;
	return IsCachedFileCurrent(candidateInfo, szfile) ? candidate : FALSE;
}


smPAT3D *smReadModel(char *file, char *szModelName)
{
	smPAT3D *pat;
	int result;

	pat = new smPAT3D;

	result = pat->LoadFile(file, szModelName);

	if(result == FALSE)
	{
		delete pat;
		return NULL;
	}

	return pat;
}

smPAT3D *smReadModel_Bip(char *file)
{
	smPAT3D *pat;
	int result;

	pat = new smPAT3D;

	result = pat->LoadFile(file);

	if(result == FALSE)
	{
		delete pat;
		return NULL;
	}

	return pat;
}





namespace
{

bool IsBipNode(const smOBJ3D *object)
{
	return object && object->NodeName[0] == 'B' && object->NodeName[1] == 'i' && object->NodeName[2] == 'p';
}

bool ParseSceneLastFrame(AseImportContext &context, const char *tag, const char *payload)
{
	if(!summit::formats::ase::TagEquals(tag, "*SCENE_LASTFRAME"))
		return true;

	summit::formats::ase::LineCursor cursor(payload);
	int lastFrame = 0;
	if(!cursor.NextInt(&lastFrame) || lastFrame < 0 ||
		lastFrame > std::numeric_limits<int>::max() / SCENE_TICKSPERFRAME)
		return false;

	context.sceneLastFrame = lastFrame * SCENE_TICKSPERFRAME;
	return true;
}

std::unique_ptr<smOBJ3D> ReadGeometryObject(FILE *&stream, AseImportContext &context)
{
	std::unique_ptr<smOBJ3D> object(new smOBJ3D);
	FILE *next = ReadASE_GEOMOBJECT(stream, context, object.get());
	if(!next)
		return std::unique_ptr<smOBJ3D>();

	stream = next;
	return object;
}

bool LoadPatternMaterials(FILE *&stream, smPAT3D *pattern, AseImportContext &context)
{
	smMATERIAL_GROUP *materialGroup = 0;
	FILE *next = ReadASE_LIGHTMAP_MATERIAL(stream, context, &materialGroup);
	if(!next || !materialGroup)
		return false;

	stream = next;
	pattern->smMaterialGroup = materialGroup;
	context.BindMaterialPattern(materialGroup->smMaterial, static_cast<int>(materialGroup->MaterialCount));
	return true;
}

void FinalizePatternTransforms(smPAT3D *pattern, const AseImportContext &context)
{
	pattern->TmParent = context.physique;
	if(context.physique)
		pattern->ReformPhysique();
	else
	{
		pattern->LinkObject();
		pattern->ReformTM();
	}
}

bool IsTalkHeadNode(const smOBJ3D *object)
{
	if(!object)
		return false;

	const char *head = strstr(object->NodeName, "Head");
	if(!head)
		return false;

	const bool validPrefix = head == object->NodeName || *(head - 1) == ' ';
	const bool validSuffix = head[4] == ' ' || head[4] == '\0';
	return validPrefix && validSuffix;
}

void BindCurrentMaterialPattern(AseImportContext &context)
{
	context.BindMaterialPattern(smMaterial, smMaterialGroup ? static_cast<int>(smMaterialGroup->MaterialCount) : 0);
}

smPAT3D *LoadCachedPattern(char *sourceFile, char *extension, char *modelName, bool bonePattern)
{
	char *cacheFile = smFindFile(sourceFile, extension);
	if(!cacheFile)
		return 0;
	return bonePattern ? smReadModel_Bip(cacheFile) : smReadModel(cacheFile, modelName);
}

bool ParseModelDocument(summit::io::LegacyFile &inputFile, FILE *&stream, smPAT3D *pattern, AseImportContext &context)
{
	char inputLine[256];
	while(inputFile.ReadLine(inputLine, sizeof(inputLine)))
	{
		summit::formats::ase::LineCursor cursor(inputLine);
		char tag[64];
		if(!cursor.NextToken(tag, sizeof(tag)))
			continue;

		const char *payload = cursor.Position();
		if(summit::formats::ase::TagEquals(tag, "*MATERIAL_LIST"))
		{
			if(!LoadPatternMaterials(stream, pattern, context))
				return false;
			continue;
		}

		if(!ParseSceneLastFrame(context, tag, payload))
			return false;
		if(!summit::formats::ase::TagEquals(tag, "*GEOMOBJECT"))
			continue;

		std::unique_ptr<smOBJ3D> object = ReadGeometryObject(stream, context);
		if(!object)
			return false;
		if(IsBipNode(object.get()))
			continue;

		object->SortVertex();
		pattern->AddObject(object.release());
	}
	return true;
}

enum class BoneReadMode
{
	Standard,
	Talk
};

bool KeepBoneObject(std::unique_ptr<smOBJ3D> &object, BoneReadMode mode, smOBJ3D *&talkHead)
{
	if(!object || !IsBipNode(object.get()))
		return false;
	if(mode == BoneReadMode::Standard)
		return true;
	if(object->TmRotCnt || object->TmPosCnt)
		return true;
	if(!IsTalkHeadNode(object.get()))
		return false;

	talkHead = object.get();
	object->NodeParent[0] = 0;
	return true;
}

bool ParseBoneDocument(summit::io::LegacyFile &inputFile, FILE *&stream,
	smPAT3D *pattern, BoneReadMode mode, smOBJ3D *&talkHead, AseImportContext &context)
{
	char inputLine[256];
	while(inputFile.ReadLine(inputLine, sizeof(inputLine)))
	{
		summit::formats::ase::LineCursor cursor(inputLine);
		char tag[64];
		if(!cursor.NextToken(tag, sizeof(tag)))
			continue;

		const char *payload = cursor.Position();
		if(!ParseSceneLastFrame(context, tag, payload))
			return false;
		if(!summit::formats::ase::TagEquals(tag, "*GEOMOBJECT"))
			continue;

		std::unique_ptr<smOBJ3D> object = ReadGeometryObject(stream, context);
		if(!object)
			return false;
		if(KeepBoneObject(object, mode, talkHead))
			pattern->AddObject(object.release());
	}
	return true;
}

bool FinalizeBonePattern(smPAT3D *pattern, smOBJ3D *talkHead)
{
	if(!pattern)
		return false;

	pattern->LinkObject();
	pattern->ReformTM();
	if(talkHead)
		strcpy(talkHead->NodeName, "Head");
	return true;
}

smPAT3D *LoadBonePattern(char *file, BoneReadMode mode)
{
	if(!file || !file[0])
		return 0;

	AseImportContext context;
	context.physique = smPhysique;
	BindCurrentMaterialPattern(context);
	if(smMeshReload)
	{
		if(smPAT3D *cached = LoadCachedPattern(file, szFileModelBip, 0, true))
			return cached;
	}

	summit::io::LegacyFile inputFile(file, "rt");
	if(!inputFile)
		return LoadCachedPattern(file, szFileModelBip, 0, true);

	FILE *stream = inputFile.Get();
	std::unique_ptr<smPAT3D> pattern(new smPAT3D);
	smOBJ3D *talkHead = 0;
	if(!ParseBoneDocument(inputFile, stream, pattern.get(), mode, talkHead, context) ||
		!FinalizeBonePattern(pattern.get(), talkHead))
		return 0;

	char *cacheFile = ChangeFileExt(file, szFileModelBip);
	if(!cacheFile || !cacheFile[0])
		return 0;
	pattern->SaveFile(cacheFile);

	smPhysique = pattern.get();
	return pattern.release();
}

} // anonymous namespace

smPAT3D *smASE_Read(char *file, char *szModelName)
{
	if(!file || !file[0])
		return FALSE;

	char *requestedName = (szModelName && szModelName[0]) ? szModelName : 0;
	SetDirectoryFromFile(file);
	if(smMeshReload)
	{
		if(smPAT3D *cached = LoadCachedPattern(file, szFileModel, requestedName, false))
			return cached;
	}

	summit::io::LegacyFile inputFile(file, "rt");
	if(!inputFile)
		return FALSE;

	FILE *stream = inputFile.Get();
	std::unique_ptr<smPAT3D> pattern(new smPAT3D);
	AseImportContext context;
	context.physique = smPhysique;
	context.ResetLightMapState();
	if(!ParseModelDocument(inputFile, stream, pattern.get(), context))
		return FALSE;

	FinalizePatternTransforms(pattern.get(), context);
	char *cacheFile = ChangeFileExt(file, szFileModel);
	if(!cacheFile || !cacheFile[0])
		return FALSE;
	pattern->SaveFile(cacheFile);

	pattern.reset();
	return smReadModel(cacheFile, requestedName);
}

smPAT3D *smASE_ReadBone(char *file)
{
	return LoadBonePattern(file, BoneReadMode::Standard);
}

smPAT3D* smASE_TalkReadBone(char *file)
{
	return LoadBonePattern(file, BoneReadMode::Talk);
}
