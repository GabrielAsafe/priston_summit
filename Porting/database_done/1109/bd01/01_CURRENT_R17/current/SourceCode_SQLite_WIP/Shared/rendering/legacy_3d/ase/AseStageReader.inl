#pragma once

#include <memory>
#include <new>

FILE *smSTAGE3D_GetASE_Info(FILE *fp, int *pVertex, int *pFace)
{
	if(!fp || !pVertex || !pFace)
		return FALSE;

	int vertexCount = -1;
	int faceCount = -1;
	int blockDepth = 0;
	char inputLine[256];
	bool objectFinished = false;

	while(fgets(inputLine, sizeof(inputLine), fp) != NULL)
	{
		summit::formats::ase::LineCursor lineCursor(inputLine);
		char tag[64];
		if(!lineCursor.NextToken(tag, sizeof(tag)))
			continue;

		const char *payload = lineCursor.Position();
		summit::formats::ase::LineCursor valueCursor(payload);
		if(summit::formats::ase::TagEquals(tag, "*MESH_NUMVERTEX"))
		{
			if(!valueCursor.NextInt(&vertexCount) || vertexCount < 0)
				return FALSE;
		}
		else if(summit::formats::ase::TagEquals(tag, "*MESH_NUMFACES"))
		{
			if(!valueCursor.NextInt(&faceCount) || faceCount < 0)
				return FALSE;
		}

		if(UpdateBlockDepth(tag, payload, blockDepth) == AseLineStatus::FinishObject)
		{
			objectFinished = true;
			break;
		}
	}

	if(!objectFinished || vertexCount < 0 || faceCount < 0)
		return FALSE;

	*pVertex = vertexCount;
	*pFace = faceCount;
	return fp;
}

smSTAGE3D *smReadStage(char *file, smSTAGE3D *smStagep)
{
	smSTAGE3D *smStage;
	int result;

	if(smStagep)
		smStage = smStagep;
	else
		smStage = new smSTAGE3D;

	result = smStage->LoadFile(file);

	if(result == FALSE)
	{
		if(!smStagep)
		{
			smStage->Init(0, 0);
			delete smStage;
		}
		return NULL;
	}

	return smStage;
}









namespace
{

struct AseStageLayout
{
	AseStageLayout()
		: vertexCount(0), faceCount(0), objectCount(0)
	{
		ZeroMemory(objectHasGeometry, sizeof(objectHasGeometry));
	}

	int vertexCount;
	int faceCount;
	int objectCount;
	char objectHasGeometry[ASE_STAGE_MAX_OBJECTS];
	std::unique_ptr<smMATERIAL_GROUP> materialGroup;
};

bool ScanStageLayout(summit::io::LegacyFile &inputFile, FILE *&stream, AseStageLayout &layout, AseImportContext &context)
{
	char inputLine[256];
	while(inputFile.ReadLine(inputLine, sizeof(inputLine)))
	{
		summit::formats::ase::LineCursor cursor(inputLine);
		char tag[64];
		if(!cursor.NextToken(tag, sizeof(tag)))
			continue;

		if(summit::formats::ase::TagEquals(tag, "*MATERIAL_LIST"))
		{
			smMATERIAL_GROUP *parsedMaterialGroup = 0;
			FILE *next = ReadASE_LIGHTMAP_MATERIAL(stream, context, &parsedMaterialGroup);
			layout.materialGroup.reset(parsedMaterialGroup);
			if(!next || !layout.materialGroup)
				return false;
			stream = next;
			continue;
		}

		if(!summit::formats::ase::TagEquals(tag, "*GEOMOBJECT"))
			continue;
		if(layout.objectCount >= ASE_STAGE_MAX_OBJECTS)
			return false;

		int objectVertices = 0;
		int objectFaces = 0;
		FILE *next = smSTAGE3D_GetASE_Info(stream, &objectVertices, &objectFaces);
		if(!next || objectVertices < 0 || objectFaces < 0)
			return false;
		stream = next;

		layout.vertexCount += objectVertices;
		layout.faceCount += objectFaces;
		layout.objectHasGeometry[layout.objectCount] = (objectVertices > 0 && objectFaces > 0) ? 1 : 0;
		++layout.objectCount;
	}

	return layout.materialGroup.get() != 0;
}

smSTAGE3D *CreateStageFromLayout(AseStageLayout &layout, smSTAGE3D *existingStage)
{
	if(!layout.materialGroup)
		return 0;

	std::unique_ptr<smSTAGE3D> ownedStage;
	smSTAGE3D *stage = existingStage;
	if(!stage)
	{
		ownedStage.reset(new(std::nothrow) smSTAGE3D);
		stage = ownedStage.get();
	}
	if(!stage || !stage->Init(layout.vertexCount, layout.faceCount))
		return 0;

	stage->smMaterialGroup = layout.materialGroup.release();
	stage->smMaterial = stage->smMaterialGroup->smMaterial;
	return ownedStage ? ownedStage.release() : stage;
}

bool LoadStageContent(summit::io::LegacyFile &inputFile, FILE *&stream,
	const AseStageLayout &layout, smSTAGE3D *stage, smASE_LIGHT *lights, int &lightCount,
	AseImportContext &context)
{
	int objectIndex = 0;
	char inputLine[256];
	while(inputFile.ReadLine(inputLine, sizeof(inputLine)))
	{
		summit::formats::ase::LineCursor cursor(inputLine);
		char tag[64];
		if(!cursor.NextToken(tag, sizeof(tag)))
			continue;

		if(summit::formats::ase::TagEquals(tag, "*LIGHTOBJECT"))
		{
			if(lightCount >= ASE_STAGE_MAX_LIGHTS)
				return false;
			FILE *next = smReadASE_LIGHTOBJECT(stream, &lights[lightCount]);
			if(!next)
				return false;
			stream = next;
			++lightCount;
			continue;
		}

		if(!summit::formats::ase::TagEquals(tag, "*GEOMOBJECT"))
			continue;
		if(objectIndex >= layout.objectCount)
			return false;

		if(layout.objectHasGeometry[objectIndex])
		{
			FILE *next = smSTAGE3D_ReadASE_GEOMOBJECT(stream, context, stage);
			if(!next)
				return false;
			stream = next;
		}
		else
		{
			int ignoredVertices = 0;
			int ignoredFaces = 0;
			FILE *next = smSTAGE3D_GetASE_Info(stream, &ignoredVertices, &ignoredFaces);
			if(!next)
				return false;
			stream = next;
		}

		++objectIndex;
	}

	return objectIndex == layout.objectCount;
}

bool ApplyStageLights(smSTAGE3D *stage, const smASE_LIGHT *lights, int lightCount)
{
	if(!stage || lightCount < 0 || (lightCount > 0 && !lights))
		return false;
	int dynamicLightCount = 0;
	POINT3D lightPosition;
	for(int index = 0; index < lightCount; ++index)
	{
		if((lights[index].Type & smLIGHT_TYPE_DYNAMIC) != 0)
		{
			++dynamicLightCount;
			continue;
		}

		lightPosition.x = lights[index].x;
		lightPosition.y = lights[index].y;
		lightPosition.z = lights[index].z;
		stage->AddVertexLightRound(&lightPosition,
			lights[index].r, lights[index].g, lights[index].b, lights[index].Range);
	}

	if(dynamicLightCount <= 0)
		return true;

	if(!stage->InitDynLight(dynamicLightCount))
		return false;
	for(int index = 0; index < lightCount; ++index)
	{
		if((lights[index].Type & smLIGHT_TYPE_DYNAMIC) == 0)
			continue;

		lightPosition.x = lights[index].x;
		lightPosition.y = lights[index].y;
		lightPosition.z = lights[index].z;
		if(!stage->AddDynLight(lights[index].Type, &lightPosition,
			lights[index].r, lights[index].g, lights[index].b, lights[index].Range))
			return false;
	}
	return true;
}

} // anonymous namespace

smSTAGE3D *smSTAGE3D_ReadASE(char *file, smSTAGE3D *smStagep)
{
	if(!file || !file[0])
		return FALSE;

	SetDirectoryFromFile(file);
	char *cacheFile = 0;
	if(smMeshReload && (cacheFile = smFindFile(file, szFileModel)))
	{
		if(smSTAGE3D *cached = smReadStage(cacheFile, smStagep))
			return cached;
	}

	summit::io::LegacyFile inputFile(file, "rt");
	if(!inputFile)
		return FALSE;

	AseImportContext context;
	context.ResetLightMapState();
	FILE *stream = inputFile.Get();
	AseStageLayout layout;
	if(!ScanStageLayout(inputFile, stream, layout, context))
		return FALSE;

	smSTAGE3D *stage = CreateStageFromLayout(layout, smStagep);
	if(!stage)
		return FALSE;

	if(!inputFile.Open(file, "rt"))
	{
		if(!smStagep) delete stage;
		return FALSE;
	}

	stream = inputFile.Get();
	smASE_LIGHT lights[ASE_STAGE_MAX_LIGHTS];
	int lightCount = 0;
	if(!LoadStageContent(inputFile, stream, layout, stage, lights, lightCount, context))
	{
		if(!smStagep) delete stage;
		return FALSE;
	}

	if(!stage->SetupPolyAreas() || !stage->SetVertexShade(!context.lightMapStage) ||
		!ApplyStageLights(stage, lights, lightCount))
	{
		if(!smStagep) delete stage;
		return FALSE;
	}

	if(smMeshReload || smMeshSave)
	{
		cacheFile = ChangeFileExt(file, szFileModel);
		stage->SaveFile(cacheFile);
	}

	return stage;
}

void smSetMeshReload(int flag, int MeshSave)
{
	smMeshReload = flag;
	smMeshSave = MeshSave;
}





