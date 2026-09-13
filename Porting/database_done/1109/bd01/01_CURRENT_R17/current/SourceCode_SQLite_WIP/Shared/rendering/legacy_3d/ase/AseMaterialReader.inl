#pragma once

#define BS_SCRIPT_MAX		7

const char *szMapStageScript[BS_SCRIPT_MAX] = {
	"BS_MODULATE:",
	"BS_MODULATE2X:",
	"BS_MODULATE4X:",
	"BS_ADD:",
	"BS_ADDSIGNED:",
	"BS_ADDSIGNED2X:",
	"BS_SUBTRACT:"
};




DWORD BsStageScript[BS_SCRIPT_MAX] = {
	D3DTOP_MODULATE,
	D3DTOP_MODULATE2X,
	D3DTOP_MODULATE4X,
	D3DTOP_ADD,
	D3DTOP_ADDSIGNED,
	D3DTOP_ADDSIGNED2X,
	D3DTOP_SUBTRACT
};






#define FS_SCRIPT_MAX		19


const char *szMapFormScript[FS_SCRIPT_MAX] = {
	"FS_NONE:",
	"FS_FORMX:",
	"FS_FORMY:",
	"FS_FORMZ:",
	"FS_SCROLL:",
	"FS_REFLEX:",

	"FS_SCROLL2:",
	"FS_SCROLL3:",
	"FS_SCROLL4:",
	"FS_SCROLL5:",
	"FS_SCROLL6:",
	"FS_SCROLL7:",
	"FS_SCROLL8:",
	"FS_SCROLL9:",
	"FS_SCROLL10:",



	"FS_SCROLLSLOW1:",
	"FS_SCROLLSLOW2:",
	"FS_SCROLLSLOW3:",
	"FS_SCROLLSLOW4:"

};




#define MATS_SCRIPT_MAX		17


struct MATS_SCRIPT_CODE
{
	const char *szScript;
	DWORD dwScriptCode;
};

MATS_SCRIPT_CODE MaterialFormScript[MATS_SCRIPT_MAX] = {
	{ "wind:" , sMATS_SCRIPT_WIND },
	{ "anim2:" , sMATS_SCRIPT_ANIM2 },
	{ "anim4:" , sMATS_SCRIPT_ANIM4 },
	{ "anim8:" , sMATS_SCRIPT_ANIM8 },
	{ "anim16:" , sMATS_SCRIPT_ANIM16 },

	{ "wind_z1:" , sMATS_SCRIPT_WINDZ1 },
	{ "wind_z2:" , sMATS_SCRIPT_WINDZ2 },
	{ "wind_x1:" , sMATS_SCRIPT_WINDX1 },
	{ "wind_x2:" , sMATS_SCRIPT_WINDX2 },

	{ "water:" , sMATS_SCRIPT_WATER },
	{ "wall:" , sMATS_SCRIPT_NOTVIEW },

	{ "pass:" , sMATS_SCRIPT_PASS },
	{ "notpass:" , sMATS_SCRIPT_NOTPASS },

	{ "render_latter:" , sMATS_SCRIPT_RENDLATTER },



	{ "BLINK_COLOR:", sMATS_SCRIPT_BLINK_COLOR },




	{ "ice:", sMATS_SCRIPT_CHECK_ICE },




	{ "orgwater:", sMATS_SCRIPT_ORG_WATER },

};


#define		MATS_BLEND_MAX		5

MATS_SCRIPT_CODE MaterialFormBlend[MATS_BLEND_MAX] = {
	{	"BLEND_ALPHA:"		,	SMMAT_BLEND_ALPHA		},
	{	"BLEND_COLOR:"		,	SMMAT_BLEND_COLOR		},
	{	"BLEND_ADDCOLOR:"	,	SMMAT_BLEND_ADDCOLOR	},
	{	"BLEND_SHADOW:"		,	SMMAT_BLEND_SHADOW		},
	{	"BLEND_LAMP:"		,	SMMAT_BLEND_LAMP		}
};



#define	ase_SCRIPT_MAX		256
struct ase_SCRIPT
{
	int		MatNum;
	char	szScript[64];
};



namespace
{

enum class MaterialLineResult
{
	NotHandled,
	Handled,
	Error
};

struct AseMaterialParseState
{
	AseMaterialParseState()
		: currentMaterial(0), bitmapMode(-1), bitmapIndex(-1),
		  bitmapFormState(0), bitmapStageState(0), scriptCount(0)
	{
		ZeroMemory(scripts, sizeof(scripts));
	}

	int currentMaterial;
	int bitmapMode;
	int bitmapIndex;
	int bitmapFormState;
	int bitmapStageState;
	ase_SCRIPT scripts[ase_SCRIPT_MAX];
	int scriptCount;
};

bool CurrentMaterialIsValid(const AseMaterialParseState &state)
{
	return summit::io::IndexInRange(state.currentMaterial, ASE_MATERIAL_CAPACITY);
}


bool ResolveAseFaceMaterial(const AseImportContext &context, int materialRef, int faceMaterial, int *materialIndex)
{
	if(!materialIndex || !context.HasMaterial(materialRef))
		return false;

	const int subPoint = context.Material(materialRef).SubPoint;
	*materialIndex = materialRef;
	if(!subPoint)
		return true;
	if(faceMaterial < 0 || !context.HasMaterial(subPoint) ||
		faceMaterial >= context.materialCount - subPoint)
		return false;

	*materialIndex = subPoint + faceMaterial;
	return context.HasMaterial(*materialIndex);
}

bool ParseMaterialIndex(const char *payload, int *value)
{
	summit::formats::ase::LineCursor cursor(payload);
	char token[64];
	return cursor.NextToken(token, sizeof(token)) && summit::io::ParseNonNegativeInt(token, value);
}

bool ApplyMaterialName(AseImportContext &context, const char *materialName, AseMaterialParseState &state)
{
	if(!materialName || !CurrentMaterialIsValid(state))
		return false;

	ASE_MATERIAL &material = context.Material(state.currentMaterial);
	for(int index = 0; index < MATS_SCRIPT_MAX; ++index)
	{
		if(strstr(materialName, MaterialFormScript[index].szScript))
			material.ScriptState |= MaterialFormScript[index].dwScriptCode;
	}

	if(material.ScriptState && state.scriptCount < ase_SCRIPT_MAX)
	{
		state.scripts[state.scriptCount].MatNum = state.currentMaterial;
		strcpy_s(state.scripts[state.scriptCount].szScript, materialName);
		++state.scriptCount;
	}

	for(int index = 0; index < MATS_BLEND_MAX; ++index)
	{
		if(strstr(materialName, MaterialFormBlend[index].szScript))
		{
			material.BlendType = MaterialFormBlend[index].dwScriptCode;
			break;
		}
	}

	return true;
}

MaterialLineResult ParseMaterialNameLine(AseImportContext &context, const char *tag, const char *payload,
	AseMaterialParseState &state, char *materialName, std::size_t materialNameCapacity)
{
	if(!summit::formats::ase::TagEquals(tag, "*MATERIAL_NAME"))
		return MaterialLineResult::NotHandled;

	summit::formats::ase::LineCursor cursor(payload);
	if(!cursor.NextQuoted(materialName, materialNameCapacity) || !ApplyMaterialName(context, materialName, state))
		return MaterialLineResult::Error;
	return MaterialLineResult::Handled;
}

MaterialLineResult ParseMaterialMapState(const char *tag, const char *payload, AseMaterialParseState &state)
{
	if(summit::formats::ase::TagEquals(tag, "*MAP_DIFFUSE"))
	{
		state.bitmapMode = 1;
		return MaterialLineResult::Handled;
	}
	if(summit::formats::ase::TagEquals(tag, "*MAP_OPACITY"))
	{
		state.bitmapMode = 0;
		return MaterialLineResult::Handled;
	}
	if(summit::formats::ase::TagEquals(tag, "*MAP_AMBIENT"))
	{
		state.bitmapMode = -1;
		return MaterialLineResult::Handled;
	}
	if(!summit::formats::ase::TagEquals(tag, "*MAP_NAME"))
		return MaterialLineResult::NotHandled;

	char mapName[256];
	summit::formats::ase::LineCursor cursor(payload);
	if(!cursor.NextQuoted(mapName, sizeof(mapName)))
		return MaterialLineResult::Error;

	state.bitmapStageState = 0;
	state.bitmapFormState = 0;
	for(int index = 0; index < BS_SCRIPT_MAX; ++index)
	{
		if(strstr(mapName, szMapStageScript[index]))
		{
			state.bitmapStageState = BsStageScript[index];
			break;
		}
	}
	for(int index = 0; index < FS_SCRIPT_MAX; ++index)
	{
		if(strstr(mapName, szMapFormScript[index]))
		{
			state.bitmapFormState = index;
			break;
		}
	}
	return MaterialLineResult::Handled;
}

MaterialLineResult ParseMaterialBitmap(AseImportContext &context, const char *tag, const char *payload, AseMaterialParseState &state)
{
	if(!summit::formats::ase::TagEquals(tag, "*BITMAP"))
		return MaterialLineResult::NotHandled;
	if(!CurrentMaterialIsValid(state))
		return MaterialLineResult::Error;

	char sourcePath[256];
	char resolvedPath[256];
	summit::formats::ase::LineCursor cursor(payload);
	if(!cursor.NextQuoted(sourcePath, sizeof(sourcePath)))
		return MaterialLineResult::Error;

	const char *bitmapName = summit::io::PathLeaf(sourcePath);
	if(!bitmapName || !bitmapName[0] ||
		!summit::io::ConcatenatePathParts(DataDirectory, bitmapName, resolvedPath, sizeof(resolvedPath)))
		return MaterialLineResult::Error;

	ASE_MATERIAL &material = context.Material(state.currentMaterial);
	if(std::strlen(resolvedPath) >= sizeof(material.BITMAP[0]))
		return MaterialLineResult::Error;

	if(state.bitmapMode == 1)
	{
		if(state.bitmapIndex + 1 >= ASE_TEXTURE_SLOT_MAX)
			return MaterialLineResult::Error;
		++state.bitmapIndex;
		strcpy_s(material.BITMAP[state.bitmapIndex], resolvedPath);
		++material.TextureCounter;
		material.BitmapStateState[state.bitmapIndex] = state.bitmapStageState;
		material.BitmapFormState[state.bitmapIndex] = state.bitmapFormState;
	}
	else if(state.bitmapMode == 0)
		strcpy_s(material.MAP_OPACITY, resolvedPath);

	return MaterialLineResult::Handled;
}

MaterialLineResult ParseMaterialUvw(AseImportContext &context, const char *tag, const char *payload, AseMaterialParseState &state)
{
	enum class UvwProperty
	{
		UOffset,
		VOffset,
		UTiling,
		VTiling,
		Angle
	};

	UvwProperty property;
	if(summit::formats::ase::TagEquals(tag, "*UVW_U_OFFSET"))
		property = UvwProperty::UOffset;
	else if(summit::formats::ase::TagEquals(tag, "*UVW_V_OFFSET"))
		property = UvwProperty::VOffset;
	else if(summit::formats::ase::TagEquals(tag, "*UVW_U_TILING"))
		property = UvwProperty::UTiling;
	else if(summit::formats::ase::TagEquals(tag, "*UVW_V_TILING"))
		property = UvwProperty::VTiling;
	else if(summit::formats::ase::TagEquals(tag, "*UVW_ANGLE"))
		property = UvwProperty::Angle;
	else
		return MaterialLineResult::NotHandled;

	if(!CurrentMaterialIsValid(state) ||
		!summit::io::IndexInRange(state.bitmapIndex, ASE_TEXTURE_SLOT_MAX))
		return MaterialLineResult::Error;

	ASE_MATERIAL &material = context.Material(state.currentMaterial);
	float *target = 0;
	switch(property)
	{
	case UvwProperty::UOffset: target = material.UVW_U_OFFSET; break;
	case UvwProperty::VOffset: target = material.UVW_V_OFFSET; break;
	case UvwProperty::UTiling: target = material.UVW_U_TILING; break;
	case UvwProperty::VTiling: target = material.UVW_V_TILING; break;
	case UvwProperty::Angle: target = material.UVW_ANGLE; break;
	}

	summit::formats::ase::LineCursor cursor(payload);
	float value = 0.0f;
	if(!cursor.NextFloat(&value))
		return MaterialLineResult::Error;
	target[state.bitmapIndex] = value;
	return MaterialLineResult::Handled;
}

MaterialLineResult ParseMaterialSurface(AseImportContext &context, const char *tag, const char *payload, AseMaterialParseState &state)
{
	const bool isTwoSided = summit::formats::ase::TagEquals(tag, "*MATERIAL_TWOSIDED");
	const bool isDiffuse = summit::formats::ase::TagEquals(tag, "*MATERIAL_DIFFUSE");
	const bool isTransparency = summit::formats::ase::TagEquals(tag, "*MATERIAL_TRANSPARENCY");
	const bool isSelfIllum = summit::formats::ase::TagEquals(tag, "*MATERIAL_SELFILLUM");
	if(!isTwoSided && !isDiffuse && !isTransparency && !isSelfIllum)
		return MaterialLineResult::NotHandled;
	if(!CurrentMaterialIsValid(state))
		return MaterialLineResult::Error;

	ASE_MATERIAL &material = context.Material(state.currentMaterial);
	if(isTwoSided)
	{
		material.TwoSide = TRUE;
		return MaterialLineResult::Handled;
	}

	summit::formats::ase::LineCursor cursor(payload);
	if(isDiffuse)
	{
		if(!cursor.NextFloat(&material.Diffuse.r) ||
			!cursor.NextFloat(&material.Diffuse.g) || !cursor.NextFloat(&material.Diffuse.b))
			return MaterialLineResult::Error;
		return MaterialLineResult::Handled;
	}

	if(isTransparency)
	{
		float value = 0.0f;
		if(!cursor.NextFloat(&value))
			return MaterialLineResult::Error;
		if(material.Transparency == 0)
			material.Transparency = value;
		return MaterialLineResult::Handled;
	}

	if(isSelfIllum)
	{
		float value = 0.0f;
		if(!cursor.NextFloat(&value))
			return MaterialLineResult::Error;
		if(material.SelfIllum == 0)
			material.SelfIllum = value;
		return MaterialLineResult::Handled;
	}

	return MaterialLineResult::NotHandled;
}

MaterialLineResult ParseCommonMaterialProperty(AseImportContext &context, const char *tag, const char *payload, AseMaterialParseState &state)
{
	MaterialLineResult result = ParseMaterialMapState(tag, payload, state);
	if(result != MaterialLineResult::NotHandled) return result;
	result = ParseMaterialBitmap(context, tag, payload, state);
	if(result != MaterialLineResult::NotHandled) return result;
	result = ParseMaterialUvw(context, tag, payload, state);
	if(result != MaterialLineResult::NotHandled) return result;
	return ParseMaterialSurface(context, tag, payload, state);
}

bool UpdateMaterialBlockDepth(const char *inputLine, int &blockDepth)
{
	if(strstr(inputLine, "{"))
		++blockDepth;
	if(strstr(inputLine, "}"))
	{
		--blockDepth;
		if(blockDepth < 0)
			return true;
	}
	return false;
}

DWORD MaterialChecksum(const ASE_MATERIAL &material)
{
	const BYTE *bytes = reinterpret_cast<const BYTE *>(&material);
	DWORD checksum = 0;
	for(std::size_t index = 0; index < sizeof(ASE_MATERIAL); ++index)
		checksum += static_cast<DWORD>(bytes[index]) * static_cast<DWORD>(index + 5) * 10;
	return checksum;
}

char *FindMaterialScript(AseMaterialParseState &state, int materialIndex)
{
	for(int index = 0; index < state.scriptCount; ++index)
	{
		if(state.scripts[index].MatNum == materialIndex)
			return state.scripts[index].szScript;
	}
	return 0;
}

bool RegisterAseMaterials(AseImportContext &context, AseMaterialParseState &state, smMATERIAL_GROUP **outputGroup)
{
	if(context.materialCount < 0 || context.materialCount > ASE_MATERIAL_CAPACITY)
		return false;

	DWORD checksums[ASE_MATERIAL_CAPACITY];
	int materialLinks[ASE_MATERIAL_CAPACITY];
	int uniqueMaterialCount = 0;

	for(int index = 0; index < context.materialCount; ++index)
		checksums[index] = MaterialChecksum(context.Material(index));

	for(int index = 0; index < context.materialCount; ++index)
	{
		int duplicate = 0;
		for(; duplicate < index; ++duplicate)
		{
			if(checksums[duplicate] == checksums[index])
				break;
		}

		// Preserve the legacy rule that material zero is never used as a deduplication source.
		if(duplicate > 0 && duplicate < index &&
			stricmp(context.Material(index).BITMAP[0], context.Material(duplicate).BITMAP[0]) == 0)
			materialLinks[index] = duplicate;
		else
		{
			materialLinks[index] = -1;
			++uniqueMaterialCount;
		}
	}

	smMATERIAL_GROUP *group = outputGroup ? new smMATERIAL_GROUP(uniqueMaterialCount) : smMaterialGroup;
	if(!group)
		return false;
	if(outputGroup)
		*outputGroup = group;

	for(int index = 0; index < context.materialCount; ++index)
	{
		if(materialLinks[index] < 0)
		{
			context.Material(index).RegistNum = group->AddMaterial(&context.Material(index),
				FindMaterialScript(state, index));
			if(context.Material(index).RegistNum < 0)
				return false;
		}
		else
		{
			context.Material(index).RegistNum = context.Material(materialLinks[index]).RegistNum;
			if(context.Material(index).RegistNum < 0)
				return false;
		}
		context.Material(index).Regist = TRUE;
	}
	return true;
}


struct StandardMaterialListState
{
	StandardMaterialListState()
		: blockDepth(0), materialNumber(0), subMaterialCount(0), nextMaterial(0), finished(false)
	{
	}

	int blockDepth;
	int materialNumber;
	int subMaterialCount;
	int nextMaterial;
	bool finished;
};

MaterialLineResult ParseStandardMaterialStructure(AseImportContext &context, const char *tag, const char *payload,
	AseMaterialParseState &state, StandardMaterialListState &list)
{
	if(summit::formats::ase::TagEquals(tag, "*MATERIAL_COUNT"))
	{
		int materialCount = 0;
		if(!ParseMaterialIndex(payload, &materialCount) || materialCount > ASE_MATERIAL_CAPACITY)
			return MaterialLineResult::Error;
		list.nextMaterial = materialCount;
		return MaterialLineResult::Handled;
	}

	if(summit::formats::ase::TagEquals(tag, "*MATERIAL"))
	{
		if(!ParseMaterialIndex(payload, &list.materialNumber) || list.materialNumber >= ASE_MATERIAL_CAPACITY)
			return MaterialLineResult::Error;
		state.currentMaterial = list.materialNumber;
		state.bitmapIndex = -1;
		return MaterialLineResult::Handled;
	}

	if(list.blockDepth == 1 && summit::formats::ase::TagEquals(tag, "*NUMSUBMTLS"))
	{
		if(!ParseMaterialIndex(payload, &list.subMaterialCount) ||
			!summit::io::IndexInRange(list.materialNumber, ASE_MATERIAL_CAPACITY) ||
			list.subMaterialCount > ASE_MATERIAL_CAPACITY - list.nextMaterial)
			return MaterialLineResult::Error;

		context.Material(list.materialNumber).SubPoint = list.nextMaterial;
		list.nextMaterial += list.subMaterialCount;
		return MaterialLineResult::Handled;
	}

	if(list.blockDepth == 1 && summit::formats::ase::TagEquals(tag, "*SUBMATERIAL"))
	{
		int subMaterialIndex = 0;
		if(!ParseMaterialIndex(payload, &subMaterialIndex) || subMaterialIndex >= list.subMaterialCount)
			return MaterialLineResult::Error;

		state.currentMaterial = context.Material(list.materialNumber).SubPoint + subMaterialIndex;
		if(!CurrentMaterialIsValid(state))
			return MaterialLineResult::Error;
		state.bitmapIndex = -1;
		return MaterialLineResult::Handled;
	}

	return MaterialLineResult::NotHandled;
}

bool ParseStandardMaterialLine(AseImportContext &context, const char *inputLine, AseMaterialParseState &state,
	StandardMaterialListState &list)
{
	summit::formats::ase::LineCursor cursor(inputLine);
	char tag[64];
	if(!cursor.NextToken(tag, sizeof(tag)))
		return true;
	const char *payload = cursor.Position();

	if(ParseStandardMaterialStructure(context, tag, payload, state, list) == MaterialLineResult::Error)
		return false;

	char materialName[256];
	MaterialLineResult result = ParseMaterialNameLine(context, tag, payload, state, materialName, sizeof(materialName));
	if(result == MaterialLineResult::NotHandled)
		result = ParseCommonMaterialProperty(context, tag, payload, state);
	if(result == MaterialLineResult::Error)
		return false;

	if(UpdateMaterialBlockDepth(inputLine, list.blockDepth))
		list.finished = true;
	return true;
}

} // anonymous namespace

FILE *ReadASE_MATERIAL(FILE *fp, AseImportContext &context, smMATERIAL_GROUP **MatGroup = 0)
{
	if(!fp)
		return FALSE;

	AseMaterialParseState state;
	StandardMaterialListState list;
	context.ResetMaterials();

	char inputLine[256];
	while(!list.finished && fgets(inputLine, sizeof(inputLine), fp) != NULL)
	{
		if(!ParseStandardMaterialLine(context, inputLine, state, list))
			return FALSE;
	}

	if(!list.finished || list.nextMaterial < 0 || list.nextMaterial > ASE_MATERIAL_CAPACITY)
		return FALSE;

	context.materialCount = list.nextMaterial;
	return RegisterAseMaterials(context, state, MatGroup) ? fp : FALSE;
}
