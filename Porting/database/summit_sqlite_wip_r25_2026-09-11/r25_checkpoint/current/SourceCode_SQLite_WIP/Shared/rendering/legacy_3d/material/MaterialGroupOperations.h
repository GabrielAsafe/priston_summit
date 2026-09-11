#pragma once

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>

#include "io/LegacyPath.h"

#include "rendering/legacy_3d/material/LegacyMaterialSerialization.h"

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace material_operations
{



struct BlinkScriptConfig
{
	BlinkScriptConfig(const char *const *scriptsValue, const DWORD *codesValue, std::size_t countValue)
		: scripts(scriptsValue), codes(codesValue), count(countValue)
	{
	}

	const char *const *scripts;
	const DWORD *codes;
	std::size_t count;
};

inline bool HasMaterialCapacity(const smMATERIAL_GROUP &group);
inline bool ChangeMaterialToAnimation(smMATERIAL *material, char **bitmaps, int textureCount,
	DWORD blendType, int autoAnimationMode);

inline int AnimationTextureCount(DWORD scriptState)
{
	if(scriptState & sMATS_SCRIPT_ANIM16) return 16;
	if(scriptState & sMATS_SCRIPT_ANIM8) return 8;
	if(scriptState & sMATS_SCRIPT_ANIM4) return 4;
	if(scriptState & sMATS_SCRIPT_ANIM2) return 2;
	return 0;
}

inline bool ParseAnimationScript(const char *script, std::string &pattern, int &frameSpeed)
{
	pattern.clear();
	frameSpeed = 0;
	if(!script)
		return false;

	const char *animation = std::strstr(script, "anim");
	if(!animation)
		return false;
	const char *patternStart = std::strchr(animation, ':');
	if(!patternStart)
		return false;
	++patternStart;
	const char *speedSeparator = std::strchr(patternStart, ':');
	const char *patternEnd = speedSeparator ? speedSeparator : script + std::strlen(script);
	if(patternEnd <= patternStart)
		return false;
	pattern.assign(patternStart, static_cast<std::size_t>(patternEnd - patternStart));

	if(speedSeparator && speedSeparator[1])
	{
		char *end = 0;
		const long parsed = std::strtol(speedSeparator + 1, &end, 10);
		if(end != speedSeparator + 1 && parsed > 0 && parsed <= std::numeric_limits<int>::max())
			frameSpeed = static_cast<int>(parsed);
	}
	return !pattern.empty();
}

inline bool AppendText(const char *text, std::string &output, std::size_t capacity)
{
	if(!text)
		return false;
	const std::size_t length = std::strlen(text);
	if(output.size() + length + 1 > capacity)
		return false;
	output.append(text, length);
	return true;
}

inline bool AppendIndexedValue(int value, int width, bool zeroPad, std::string &output, std::size_t capacity)
{
	std::string number = std::to_string(value);
	if(width > static_cast<int>(number.size()))
	{
		const std::size_t padding = static_cast<std::size_t>(width) - number.size();
		if(output.size() + padding + number.size() + 1 > capacity)
			return false;
		output.append(padding, zeroPad ? '0' : ' ');
	}
	return AppendText(number.c_str(), output, capacity);
}

// Formats the subset of wsprintf patterns used by legacy material animation
// names. Literal %% and one or more integer conversions (%d, %02d, %3d) are
// supported; every other conversion is rejected instead of invoking undefined
// behavior with a mismatched format argument.
inline bool FormatIndexedAnimationPath(const char *directory, const std::string &pattern,
	int frameIndex, char *output, std::size_t outputCapacity)
{
	if(!directory || !output || outputCapacity == 0)
		return false;

	std::string formatted;
	formatted.reserve(outputCapacity - 1);
	if(!AppendText(directory, formatted, outputCapacity))
		return false;

	for(std::size_t index = 0; index < pattern.size(); ++index)
	{
		if(pattern[index] != '%')
		{
			char literal[2] = { pattern[index], 0 };
			if(!AppendText(literal, formatted, outputCapacity))
				return false;
			continue;
		}
		if(index + 1 >= pattern.size())
			return false;
		if(pattern[index + 1] == '%')
		{
			if(!AppendText("%", formatted, outputCapacity))
				return false;
			++index;
			continue;
		}

		std::size_t cursor = index + 1;
		bool zeroPad = false;
		if(cursor < pattern.size() && pattern[cursor] == '0')
		{
			zeroPad = true;
			++cursor;
		}
		int width = 0;
		while(cursor < pattern.size() && std::isdigit(static_cast<unsigned char>(pattern[cursor])))
		{
			if(width > 100)
				return false;
			width = width * 10 + (pattern[cursor] - '0');
			++cursor;
		}
		if(cursor >= pattern.size() || (pattern[cursor] != 'd' && pattern[cursor] != 'i'))
			return false;
		if(!AppendIndexedValue(frameIndex, width, zeroPad, formatted, outputCapacity))
			return false;
		index = cursor;
	}

	if(formatted.size() + 1 > outputCapacity)
		return false;
	std::memcpy(output, formatted.c_str(), formatted.size() + 1);
	return true;
}

inline void ReleaseEveryTextureSlot(smMATERIAL &material)
{
	for(DWORD index = 0; index < material_serialization::TextureSlotCapacity; ++index)
	{
		if(material.smTexture[index])
			smTexture.Delete(material.smTexture[index]);
		material.smTexture[index] = 0;
	}
	for(DWORD index = 0; index < material_serialization::AnimatedTextureSlotCapacity; ++index)
	{
		if(material.smAnimTexture[index])
			smTexture.Delete(material.smAnimTexture[index]);
		material.smAnimTexture[index] = 0;
	}
	material.TextureCounter = 0;
	material.AnimTexCounter = 0;
}

inline bool LoadBaseMaterialTextures(const ASE_MATERIAL &source, smMATERIAL &target)
{
	if(source.TextureCounter < 0 ||
		source.TextureCounter > static_cast<int>(material_serialization::TextureSlotCapacity))
		return false;

	const bool hasOpacity = source.MAP_OPACITY[0] != 0;
	for(int index = 0; index < source.TextureCounter; ++index)
	{
		if(!source.BITMAP[index][0])
			return false;
		const bool applyOpacity = hasOpacity && (source.TextureCounter == 1 || index < source.TextureCounter - 1);
		target.smTexture[index] = applyOpacity
			? smTexture.Add(const_cast<char *>(source.BITMAP[index]), const_cast<char *>(source.MAP_OPACITY))
			: smTexture.Add(const_cast<char *>(source.BITMAP[index]));
		if(!target.smTexture[index])
			return false;
		target.TextureStageState[index] = source.BitmapStateState[index];
		target.TextureFormState[index] = source.BitmapFormState[index];
		target.ReformTexture += static_cast<int>(source.BitmapFormState[index]);
		++target.TextureCounter;
	}
	target.MapOpacity = hasOpacity ? TRUE : FALSE;
	return true;
}

inline void ApplyMaterialScriptState(const ASE_MATERIAL &source, smMATERIAL &target)
{
	target.MeshState = source.Transparency == 0 ? SMMAT_STAT_CHECK_FACE : 0;
	if(source.ScriptState & sMATS_SCRIPT_WIND) { target.WindMeshBottom = sMATS_SCRIPT_WINDZ1; target.MeshState = 0; }
	if(source.ScriptState & sMATS_SCRIPT_WINDX1) { target.WindMeshBottom = sMATS_SCRIPT_WINDX1; target.MeshState = 0; }
	if(source.ScriptState & sMATS_SCRIPT_WINDX2) { target.WindMeshBottom = sMATS_SCRIPT_WINDX2; target.MeshState = 0; }
	if(source.ScriptState & sMATS_SCRIPT_WINDZ1) { target.WindMeshBottom = sMATS_SCRIPT_WINDZ1; target.MeshState = 0; }
	if(source.ScriptState & sMATS_SCRIPT_WINDZ2) { target.WindMeshBottom = sMATS_SCRIPT_WINDZ2; target.MeshState = 0; }
	if(source.ScriptState & sMATS_SCRIPT_WATER) { target.WindMeshBottom = sMATS_SCRIPT_WATER; target.MeshState = 0; }

	if(source.ScriptState & sMATS_SCRIPT_NOTPASS)
		target.MeshState = SMMAT_STAT_CHECK_FACE;
	else if(source.ScriptState & sMATS_SCRIPT_PASS)
		target.MeshState = 0;
	if(source.ScriptState & sMATS_SCRIPT_RENDLATTER)
		target.MeshState |= sMATS_SCRIPT_RENDLATTER;
	if(source.ScriptState & sMATS_SCRIPT_CHECK_ICE)
		target.MeshState |= sMATS_SCRIPT_CHECK_ICE;
	if(source.ScriptState & sMATS_SCRIPT_ORG_WATER)
		target.MeshState = sMATS_SCRIPT_ORG_WATER;
}

inline void ApplyBlinkScript(const ASE_MATERIAL &source, const char *script,
	const BlinkScriptConfig &blinkConfig, smMATERIAL &target)
{
	if(!(source.ScriptState & sMATS_SCRIPT_BLINK_COLOR) || target.WindMeshBottom != 0 ||
		!blinkConfig.codes || blinkConfig.count == 0)
		return;

	std::size_t selected = 0;
	if(script && blinkConfig.scripts)
	{
		for(std::size_t index = 0; index < blinkConfig.count; ++index)
		{
			if(blinkConfig.scripts[index] && std::strstr(script, blinkConfig.scripts[index]))
			{
				selected = index;
				break;
			}
		}
	}
	target.WindMeshBottom = static_cast<int>(blinkConfig.codes[selected]);
}

inline bool ApplyAnimationScript(const ASE_MATERIAL &source, const char *script, smMATERIAL &target)
{
	const int animationCount = AnimationTextureCount(source.ScriptState);
	if(animationCount == 0 || !script)
		return true;
	if(source.TextureCounter <= 0 || !source.BITMAP[0][0])
		return false;

	std::string pattern;
	int frameSpeed = 0;
	if(!ParseAnimationScript(script, pattern, frameSpeed))
		return true; // Legacy behavior: a flag without a usable directive leaves the base material intact.

	char directory[64];
	if(!summit::io::CopyFileDirectory(source.BITMAP[0], directory, sizeof(directory)))
		return false;
	char animationPaths[16][64] = {};
	char *pathPointers[16] = {};
	for(int index = 0; index < animationCount; ++index)
	{
		if(!FormatIndexedAnimationPath(directory, pattern, index, animationPaths[index], sizeof(animationPaths[index])))
			return false;
		pathPointers[index] = animationPaths[index];
	}

	if(!::summit::rendering::legacy3d::material_operations::ChangeMaterialToAnimation(&target, pathPointers, animationCount, target.BlendType, TRUE))
		return false;
	if(frameSpeed > 0)
		target.Shift_FrameSpeed = static_cast<DWORD>(frameSpeed);
	target.Transparency = source.Transparency;
	return true;
}

inline int AddAseMaterial(smMATERIAL_GROUP &group, ASE_MATERIAL *source, const char *script,
	const BlinkScriptConfig &blinkConfig)
{
	if(!source || !HasMaterialCapacity(group) || source->TextureCounter < 0 ||
		source->TextureCounter > static_cast<int>(material_serialization::TextureSlotCapacity))
		return -1;

	smMATERIAL material = smMATERIAL();
	material.InUse = TRUE;
	material.Shade = TRUE;
	material.BlendType = SMMAT_BLEND_ALPHA;
	material.TwoSide = source->TwoSide;
	material.ReformTexture = 0;
	if(!LoadBaseMaterialTextures(*source, material))
	{
		ReleaseEveryTextureSlot(material);
		return -1;
	}

	material.Transparency = source->Transparency;
	material.SelfIllum = source->SelfIllum;
	material.Diffuse = source->Diffuse;
	material.UseState = static_cast<int>(source->ScriptState);
	material.TextureType = SMTEX_TYPE_MULTIMIX;
	if(source->BlendType)
		material.BlendType = static_cast<DWORD>(source->BlendType);
	ApplyMaterialScriptState(*source, material);
	if(!ApplyAnimationScript(*source, script, material))
	{
		ReleaseEveryTextureSlot(material);
		return -1;
	}
	ApplyBlinkScript(*source, script, blinkConfig, material);

	const int materialIndex = static_cast<int>(group.MaterialCount);
	group.smMaterial[materialIndex] = material;
	++group.MaterialCount;
	if(material.ReformTexture)
		++group.ReformTexture;
	++source->Regist;
	source->RegistNum = materialIndex;
	return materialIndex;
}

inline bool EqualsIgnoreCaseAscii(const char *left, const char *right)
{
	if(!left || !right)
		return false;
	while(*left && *right)
	{
		const unsigned char l = static_cast<unsigned char>(*left++);
		const unsigned char r = static_cast<unsigned char>(*right++);
		if(std::tolower(l) != std::tolower(r))
			return false;
	}
	return *left == *right;
}

inline bool HasMaterialCapacity(const smMATERIAL_GROUP &group)
{
	return group.smMaterial && group.MaxMaterial > 0 && group.MaxMaterial <= MATRIAL_MAX &&
		group.MaterialCount < static_cast<DWORD>(group.MaxMaterial);
}

inline bool IsValidMaterialIndex(const smMATERIAL_GROUP &group, int materialIndex)
{
	return group.smMaterial && materialIndex >= 0 &&
		static_cast<DWORD>(materialIndex) < group.MaterialCount &&
		materialIndex < group.MaxMaterial;
}

inline int DeleteUnusedMaterials(smMATERIAL_GROUP &group)
{
	if(!group.smMaterial || group.MaxMaterial < 0 || group.MaxMaterial > MATRIAL_MAX)
		return 0;

	const DWORD materialCount = std::min(group.MaterialCount, static_cast<DWORD>(group.MaxMaterial));
	int deleted = 0;
	for(DWORD index = 0; index < materialCount; ++index)
	{
		smMATERIAL &material = group.smMaterial[index];
		if(material.InUse != TRUE)
			continue;
		if(material.ReformTexture && group.ReformTexture > 0)
			--group.ReformTexture;
		material_serialization::ReleaseMaterialTextures(material);
		material = smMATERIAL();
		++deleted;
	}
	return deleted;
}

inline int CreateTextureMaterial(smMATERIAL_GROUP &group, char *bitmap, char *opacity,
	int twoSided, int shade, DWORD textureSwap, DWORD blendType)
{
	if(!bitmap || !bitmap[0] || !HasMaterialCapacity(group))
		return -1;

	smTEXTUREHANDLE *texture = opacity && opacity[0]
		? smTexture.Add(bitmap, opacity)
		: smTexture.Add(bitmap);
	if(!texture)
		return -1;

	smMATERIAL material = smMATERIAL();
	material.InUse = TRUE;
	material.Shade = shade;
	material.TwoSide = twoSided;
	material.smTexture[0] = texture;
	material.TextureStageState[0] = 0;
	material.TextureFormState[0] = 0;
	material.ReformTexture = 0;
	material.MapOpacity = FALSE;
	material.TextureCounter = 1;
	material.Transparency = 0;
	material.UseState = 0;
	material.TextureType = SMTEX_TYPE_MULTIMIX;
	material.BlendType = blendType;
	material.smTexture[0]->TexSwapMode = textureSwap;
	material.InUse++;

	const int materialIndex = static_cast<int>(group.MaterialCount);
	group.smMaterial[materialIndex] = material;
	group.MaterialCount++;
	return materialIndex;
}

inline bool AddTexture(smMATERIAL_GROUP &group, int materialIndex, char *bitmap, char *opacity)
{
	if(!bitmap || !bitmap[0] || !IsValidMaterialIndex(group, materialIndex))
		return false;
	smMATERIAL &material = group.smMaterial[materialIndex];
	if(material.TextureCounter >= material_serialization::TextureSlotCapacity)
		return false;

	smTEXTUREHANDLE *texture = opacity && opacity[0]
		? smTexture.Add(bitmap, opacity)
		: smTexture.Add(bitmap);
	if(!texture)
		return false;
	material.smTexture[material.TextureCounter++] = texture;
	return true;
}

inline smMATERIAL *FindNextMaterialFromTextureName(smMATERIAL_GROUP &group)
{
	if(!group.smMaterial || !group.szLastSearchName[0])
		return 0;
	const DWORD start = group.LastSearchMaterial < 0 ? 0 : static_cast<DWORD>(group.LastSearchMaterial);
	for(DWORD materialIndex = start; materialIndex < group.MaterialCount; ++materialIndex)
	{
		smMATERIAL &material = group.smMaterial[materialIndex];
		if(!material.InUse || !material_serialization::HasValidTextureCounts(material))
			continue;
		for(DWORD textureIndex = 0; textureIndex < material.TextureCounter; ++textureIndex)
		{
			if(material.smTexture[textureIndex] &&
				EqualsIgnoreCaseAscii(material.smTexture[textureIndex]->Name, group.szLastSearchName))
			{
				group.LastSearchMaterial = static_cast<int>(materialIndex + 1);
				return &material;
			}
		}
		for(DWORD textureIndex = 0; textureIndex < material.AnimTexCounter; ++textureIndex)
		{
			if(material.smAnimTexture[textureIndex] &&
				EqualsIgnoreCaseAscii(material.smAnimTexture[textureIndex]->Name, group.szLastSearchName))
			{
				group.LastSearchMaterial = static_cast<int>(materialIndex + 1);
				return &material;
			}
		}
	}
	group.LastSearchMaterial = static_cast<int>(group.MaterialCount);
	return 0;
}

inline smMATERIAL *FindMaterialFromTextureName(smMATERIAL_GROUP &group, const char *name)
{
	if(!name)
		return 0;
	std::size_t length = 0;
	if(!material_serialization::BoundedStringLength(name, sizeof(group.szLastSearchName), length))
		return 0;
	std::memset(group.szLastSearchName, 0, sizeof(group.szLastSearchName));
	std::memcpy(group.szLastSearchName, name, length);
	group.LastSearchMaterial = 0;
	return FindNextMaterialFromTextureName(group);
}

inline int SetMaterialAnimationFrame(smMATERIAL *material, int frame)
{
	if(!material)
		return 0;
	const int oldFrame = static_cast<int>(material->AnimationFrame);
	material->AnimationFrame = frame;
	return oldFrame;
}

inline void DeleteTextureList(smTEXTUREHANDLE **textures, DWORD count)
{
	for(DWORD index = 0; index < count; ++index)
	{
		if(textures[index])
			smTexture.Delete(textures[index]);
		textures[index] = 0;
	}
}

inline bool ChangeMaterialToAnimation(smMATERIAL *material, char **bitmaps, int textureCount,
	DWORD blendType, int autoAnimationMode)
{
	if(!material || !bitmaps || textureCount <= 0 ||
		textureCount > static_cast<int>(material_serialization::AnimatedTextureSlotCapacity))
		return false;

	smTEXTUREHANDLE *loaded[material_serialization::AnimatedTextureSlotCapacity] = {};
	for(int index = 0; index < textureCount; ++index)
	{
		if(!bitmaps[index] || !bitmaps[index][0])
		{
			DeleteTextureList(loaded, static_cast<DWORD>(index));
			return false;
		}
		loaded[index] = smTexture.Add(bitmaps[index]);
		if(!loaded[index])
		{
			DeleteTextureList(loaded, static_cast<DWORD>(index));
			return false;
		}
		loaded[index]->TexSwapMode = FALSE;
	}

	if(material->AnimTexCounter <= material_serialization::AnimatedTextureSlotCapacity)
		DeleteTextureList(material->smAnimTexture, material->AnimTexCounter);
	else
		std::memset(material->smAnimTexture, 0, sizeof(material->smAnimTexture));

	for(int index = 0; index < textureCount; ++index)
		material->smAnimTexture[index] = loaded[index];
	const DWORD regularTextureCount = std::min(material->TextureCounter, material_serialization::TextureSlotCapacity);
	for(DWORD index = 1; index < regularTextureCount; ++index)
	{
		if(material->smTexture[index])
			smTexture.Delete(material->smTexture[index]);
		material->smTexture[index] = 0;
	}
	material->MapOpacity = FALSE;
	material->TextureCounter = 1;
	material->Transparency = 0;
	material->AnimTexCounter = static_cast<DWORD>(textureCount);
	material->FrameMask = static_cast<DWORD>(textureCount - 1);
	material->Shift_FrameSpeed = 6;
	material->UseState = 0;
	material->TextureType = SMTEX_TYPE_ANIMATION;
	material->BlendType = blendType;
	material->AnimationFrame = autoAnimationMode ? SMTEX_AUTOANIMATION : 0;
	material->InUse++;
	return true;
}

inline int CreateAnimationMaterial(smMATERIAL_GROUP &group, char **bitmaps, int textureCount,
	DWORD blendType, int autoAnimationMode)
{
	if(!HasMaterialCapacity(group))
		return -1;

	smMATERIAL material = smMATERIAL();
	material.InUse = TRUE;
	material.Shade = FALSE;
	material.TwoSide = TRUE;
	if(!::summit::rendering::legacy3d::material_operations::ChangeMaterialToAnimation(&material, bitmaps, textureCount, blendType, autoAnimationMode))
		return -1;

	const int materialIndex = static_cast<int>(group.MaterialCount);
	group.smMaterial[materialIndex] = material;
	group.MaterialCount++;
	return materialIndex;
}

} // namespace material_operations
} // namespace legacy3d
} // namespace rendering
} // namespace summit
