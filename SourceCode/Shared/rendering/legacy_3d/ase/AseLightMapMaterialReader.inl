#pragma once

using summit::rendering::legacy3d::AseLightMapReference;

namespace
{

bool TrackLightMapMaterial(AseImportContext &context, const char *materialName, int blockDepth)
{
	if(blockDepth != 1)
		return true;
	if(!materialName || context.lightMapReferenceCount < 0 || context.lightMapReferenceCount >= static_cast<int>(context.lightMapReferences.size()))
		return false;

	AseLightMapReference &reference = context.lightMapReferences[context.lightMapReferenceCount];
	reference.isLightMap = strstr(materialName, "*LM_") ? TRUE : FALSE;
	reference.subPoint = 0;
	reference.subMaterialCount = 0;
	if(reference.isLightMap)
		context.lightMapStage = true;
	++context.lightMapReferenceCount;
	return true;
}

bool UpdateLightMapScriptsForRemovedPair(AseMaterialParseState &state, int subPoint, int subMaterialCount)
{
	for(int index = 0; index < state.scriptCount; ++index)
	{
		if(state.scripts[index].MatNum == subPoint || state.scripts[index].MatNum == subPoint + 1)
			state.scripts[index].MatNum = 0;
		else if(state.scripts[index].MatNum >= subPoint + 2 &&
			state.scripts[index].MatNum <= subPoint + 2 + subMaterialCount - 1)
			state.scripts[index].MatNum -= 2;
	}
	return true;
}

void RebaseMaterialScripts(AseMaterialParseState &state, int firstMaterial, int materialCount, int offset)
{
	for(int index = 0; index < state.scriptCount; ++index)
	{
		if(state.scripts[index].MatNum >= firstMaterial &&
			state.scripts[index].MatNum <= firstMaterial + materialCount - 1)
			state.scripts[index].MatNum -= offset;
	}
}

bool AppendLightMapTexture(ASE_MATERIAL &material, const ASE_MATERIAL &lightMapMaterial)
{
	const int bitmapIndex = material.TextureCounter;
	if(!summit::io::IndexInRange(bitmapIndex, ASE_TEXTURE_SLOT_MAX))
		return false;

	++material.TextureCounter;
	strcpy_s(material.BITMAP[bitmapIndex], lightMapMaterial.BITMAP[0]);
	material.BitmapStateState[bitmapIndex] = lightMapMaterial.BitmapStateState[0];
	material.BitmapFormState[bitmapIndex] = lightMapMaterial.BitmapFormState[0];
	material.UVW_U_OFFSET[bitmapIndex] = lightMapMaterial.UVW_U_OFFSET[0];
	material.UVW_V_OFFSET[bitmapIndex] = lightMapMaterial.UVW_V_OFFSET[0];
	material.UVW_U_TILING[bitmapIndex] = lightMapMaterial.UVW_U_TILING[0];
	material.UVW_V_TILING[bitmapIndex] = lightMapMaterial.UVW_V_TILING[0];
	material.UVW_ANGLE[bitmapIndex] = lightMapMaterial.UVW_ANGLE[0];
	return true;
}

bool CompactLightMappedSubMaterials(AseImportContext &context, AseMaterialParseState &state,
	int sourceSubPoint, int subMaterialCount, int destinationSubPoint)
{
	if(sourceSubPoint + 1 >= context.materialCount || sourceSubPoint + 2 > context.materialCount ||
		subMaterialCount > context.materialCount - (sourceSubPoint + 2))
		return false;

	const ASE_MATERIAL lightMapMaterial = context.Material(sourceSubPoint + 1);
	if(lightMapMaterial.TextureCounter <= 0 || lightMapMaterial.TextureCounter > ASE_TEXTURE_SLOT_MAX)
		return false;

	::MoveMemory(&context.Material(destinationSubPoint), &context.Material(sourceSubPoint + 2),
		sizeof(ASE_MATERIAL) * subMaterialCount);
	for(int materialIndex = destinationSubPoint;
		materialIndex < destinationSubPoint + subMaterialCount; ++materialIndex)
	{
		if(!AppendLightMapTexture(context.Material(materialIndex), lightMapMaterial))
			return false;
	}

	if(context.materialCount < 2)
		return false;
	context.materialCount -= 2;
	return UpdateLightMapScriptsForRemovedPair(state, sourceSubPoint, subMaterialCount);
}

bool CompactRegularSubMaterials(AseImportContext &context, AseMaterialParseState &state,
	int sourceSubPoint, int subMaterialCount, int destinationSubPoint)
{
	if(subMaterialCount > context.materialCount - sourceSubPoint)
		return false;

	::MoveMemory(&context.Material(destinationSubPoint), &context.Material(sourceSubPoint),
		sizeof(ASE_MATERIAL) * subMaterialCount);
	RebaseMaterialScripts(state, sourceSubPoint, subMaterialCount, sourceSubPoint - destinationSubPoint);
	return true;
}

bool CompactLightMapReference(AseImportContext &context, AseMaterialParseState &state, int referenceIndex, int destinationSubPoint)
{
	if(!summit::io::IndexInRange(referenceIndex, context.lightMapReferenceCount))
		return false;

	const AseLightMapReference &reference = context.lightMapReferences[referenceIndex];
	const int sourceSubPoint = reference.subPoint;
	const int subMaterialCount = reference.subMaterialCount;
	if(subMaterialCount < 0)
		return false;

	if(!sourceSubPoint)
		return reference.isLightMap == FALSE;
	if(!summit::io::IndexInRange(referenceIndex, ASE_MATERIAL_CAPACITY) ||
		!context.HasMaterial(sourceSubPoint) ||
		subMaterialCount > ASE_MATERIAL_CAPACITY - destinationSubPoint)
		return false;

	const bool compacted = reference.isLightMap
		? CompactLightMappedSubMaterials(context, state, sourceSubPoint, subMaterialCount, destinationSubPoint)
		: CompactRegularSubMaterials(context, state, sourceSubPoint, subMaterialCount, destinationSubPoint);
	if(!compacted)
		return false;

	context.Material(referenceIndex).SubPoint = destinationSubPoint;
	return true;
}

bool CompactLightMapMaterials(AseImportContext &context, AseMaterialParseState &state)
{
	if(!context.lightMapStage)
		return true;

	int destinationSubPoint = context.lightMapReferenceCount;
	if(destinationSubPoint < 0 || destinationSubPoint > ASE_MATERIAL_CAPACITY)
		return false;

	for(int referenceIndex = 0; referenceIndex < context.lightMapReferenceCount; ++referenceIndex)
	{
		const int subMaterialCount = context.lightMapReferences[referenceIndex].subMaterialCount;
		if(!CompactLightMapReference(context, state, referenceIndex, destinationSubPoint))
			return false;
		if(context.lightMapReferences[referenceIndex].subPoint)
			destinationSubPoint += subMaterialCount;
	}
	return true;
}


struct LightMapMaterialListState
{
	LightMapMaterialListState()
		: blockDepth(0), oldBlockDepth(0), subMaterialCount(0), materialNumber(0),
		  oldMaterialNumber(0), subMaterialNumber(-1), nextMaterial(0), finished(false)
	{
	}

	int blockDepth;
	int oldBlockDepth;
	int subMaterialCount;
	int materialNumber;
	int oldMaterialNumber;
	int subMaterialNumber;
	int nextMaterial;
	bool finished;
};

MaterialLineResult ParseLightMapMaterialRoot(const char *tag, const char *payload,
	AseMaterialParseState &state, LightMapMaterialListState &list)
{
	if(summit::formats::ase::TagEquals(tag, "*MATERIAL_COUNT"))
	{
		int materialCount = 0;
		if(!ParseMaterialIndex(payload, &materialCount) || materialCount > ASE_MATERIAL_CAPACITY)
			return MaterialLineResult::Error;
		list.nextMaterial = materialCount;
		return MaterialLineResult::Handled;
	}

	if(!summit::formats::ase::TagEquals(tag, "*MATERIAL"))
		return MaterialLineResult::NotHandled;
	if(!ParseMaterialIndex(payload, &list.materialNumber) || list.materialNumber >= ASE_MATERIAL_CAPACITY)
		return MaterialLineResult::Error;

	state.currentMaterial = list.materialNumber;
	state.bitmapIndex = -1;
	return MaterialLineResult::Handled;
}

MaterialLineResult ParseLightMapSubMaterialCount(AseImportContext &context, const char *tag, const char *payload,
	AseMaterialParseState &state, LightMapMaterialListState &list)
{
	if(!summit::formats::ase::TagEquals(tag, "*NUMSUBMTLS"))
		return MaterialLineResult::NotHandled;

	if((list.blockDepth % 2) == 0 && (list.subMaterialCount - (list.subMaterialNumber + 1)) > 0)
	{
		list.oldBlockDepth = list.blockDepth - 1;
		list.oldMaterialNumber = list.materialNumber;
	}
	else
		list.oldMaterialNumber = 0;

	if(!ParseMaterialIndex(payload, &list.subMaterialCount) ||
		!CurrentMaterialIsValid(state) || list.subMaterialCount > ASE_MATERIAL_CAPACITY - list.nextMaterial)
		return MaterialLineResult::Error;

	context.Material(state.currentMaterial).SubPoint = list.nextMaterial;
	const int lightMapReference = context.lightMapReferenceCount - 1;
	if(lightMapReference < 0 || lightMapReference >= static_cast<int>(context.lightMapReferences.size()))
		return MaterialLineResult::Error;

	AseLightMapReference &reference = context.lightMapReferences[lightMapReference];
	if(reference.isLightMap == TRUE)
	{
		if(lightMapReference == state.currentMaterial)
			reference.subPoint = list.nextMaterial;
		else
			reference.subMaterialCount = list.subMaterialCount;
	}
	else
	{
		reference.subPoint = list.nextMaterial;
		reference.subMaterialCount = list.subMaterialCount;
	}

	list.materialNumber = list.nextMaterial;
	list.nextMaterial += list.subMaterialCount;
	return MaterialLineResult::Handled;
}

MaterialLineResult ParseLightMapSubMaterial(const char *tag, const char *payload,
	AseMaterialParseState &state, LightMapMaterialListState &list)
{
	if(!summit::formats::ase::TagEquals(tag, "*SUBMATERIAL"))
		return MaterialLineResult::NotHandled;

	if(list.blockDepth == list.oldBlockDepth && list.oldMaterialNumber)
	{
		list.materialNumber = list.oldMaterialNumber;
		list.oldBlockDepth = 0;
		list.oldMaterialNumber = 0;
	}

	if(!ParseMaterialIndex(payload, &list.subMaterialNumber) ||
		list.subMaterialNumber >= list.subMaterialCount)
		return MaterialLineResult::Error;

	state.currentMaterial = list.materialNumber + list.subMaterialNumber;
	if(!CurrentMaterialIsValid(state))
		return MaterialLineResult::Error;
	state.bitmapIndex = -1;
	return MaterialLineResult::Handled;
}

bool ParseLightMapMaterialLine(AseImportContext &context, const char *inputLine, AseMaterialParseState &state,
	LightMapMaterialListState &list)
{
	summit::formats::ase::LineCursor cursor(inputLine);
	char tag[64];
	if(!cursor.NextToken(tag, sizeof(tag)))
		return true;
	const char *payload = cursor.Position();

	if(ParseLightMapMaterialRoot(tag, payload, state, list) == MaterialLineResult::Error)
		return false;

	char materialName[256];
	MaterialLineResult result = ParseMaterialNameLine(context, tag, payload, state, materialName, sizeof(materialName));
	if(result == MaterialLineResult::Error)
		return false;
	if(result == MaterialLineResult::Handled && !TrackLightMapMaterial(context, materialName, list.blockDepth))
		return false;

	MaterialLineResult structure = ParseLightMapSubMaterialCount(context, tag, payload, state, list);
	if(structure == MaterialLineResult::Error)
		return false;
	if(structure == MaterialLineResult::NotHandled)
	{
		structure = ParseLightMapSubMaterial(tag, payload, state, list);
		if(structure == MaterialLineResult::Error)
			return false;
	}

	if(result == MaterialLineResult::NotHandled)
		result = ParseCommonMaterialProperty(context, tag, payload, state);
	if(result == MaterialLineResult::Error)
		return false;

	if(UpdateMaterialBlockDepth(inputLine, list.blockDepth))
		list.finished = true;
	return true;
}

} // anonymous namespace

FILE* ReadASE_LIGHTMAP_MATERIAL(FILE *fp, AseImportContext &context, smMATERIAL_GROUP **MatGroup = 0)
{
	if(!fp)
		return FALSE;

	AseMaterialParseState state;
	LightMapMaterialListState list;
	context.ResetMaterials();

	char inputLine[256];
	while(!list.finished && fgets(inputLine, sizeof(inputLine), fp) != NULL)
	{
		if(!ParseLightMapMaterialLine(context, inputLine, state, list))
			return FALSE;
	}

	if(!list.finished || list.nextMaterial < 0 || list.nextMaterial > ASE_MATERIAL_CAPACITY)
		return FALSE;

	context.materialCount = list.nextMaterial;
	if(!CompactLightMapMaterials(context, state))
		return FALSE;
	return RegisterAseMaterials(context, state, MatGroup) ? fp : FALSE;
}
