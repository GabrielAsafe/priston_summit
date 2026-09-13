#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <vector>

#include "io/LegacyBinaryIO.h"

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace material_serialization
{

static const DWORD TextureSlotCapacity = 8;
static const DWORD AnimatedTextureSlotCapacity = 32;
static const std::size_t TextureNameCapacity = 64;
static const std::size_t MaxSerializedNameBytes =
	(TextureSlotCapacity + AnimatedTextureSlotCapacity) * 2 * TextureNameCapacity;

inline void ResetRuntimeState(smMATERIAL_GROUP &group)
{
	group.smMaterial = 0;
	group.MaterialCount = 0;
	group.ReformTexture = 0;
	group.MaxMaterial = 0;
	group.LastSearchMaterial = 0;
	std::memset(group.szLastSearchName, 0, sizeof(group.szLastSearchName));
}

inline bool HasValidTextureCounts(const smMATERIAL &material)
{
	return material.TextureCounter <= TextureSlotCapacity &&
		material.AnimTexCounter <= AnimatedTextureSlotCapacity;
}

inline void ClearRuntimeTexturePointers(smMATERIAL &material)
{
	std::fill(material.smTexture, material.smTexture + TextureSlotCapacity,
		static_cast<smTEXTUREHANDLE *>(0));
	std::fill(material.smAnimTexture, material.smAnimTexture + AnimatedTextureSlotCapacity,
		static_cast<smTEXTUREHANDLE *>(0));
}

inline void ReleaseMaterialTextures(smMATERIAL &material)
{
	const DWORD textureCount = std::min(material.TextureCounter, TextureSlotCapacity);
	for(DWORD index = 0; index < textureCount; ++index)
	{
		if(material.smTexture[index])
			smTexture.Delete(material.smTexture[index]);
		material.smTexture[index] = 0;
	}

	const DWORD animationCount = std::min(material.AnimTexCounter, AnimatedTextureSlotCapacity);
	for(DWORD index = 0; index < animationCount; ++index)
	{
		if(material.smAnimTexture[index])
			smTexture.Delete(material.smAnimTexture[index]);
		material.smAnimTexture[index] = 0;
	}
}

inline void ReleaseGroupStorage(smMATERIAL_GROUP &group)
{
	if(group.smMaterial)
	{
		const DWORD capacity = group.MaxMaterial >= 0 && group.MaxMaterial <= MATRIAL_MAX
			? static_cast<DWORD>(group.MaxMaterial) : 0;
		const DWORD materialCount = std::min(group.MaterialCount, capacity);
		for(DWORD index = 0; index < materialCount; ++index)
		{
			if(group.smMaterial[index].InUse && HasValidTextureCounts(group.smMaterial[index]))
				ReleaseMaterialTextures(group.smMaterial[index]);
		}
		delete[] group.smMaterial;
	}
	ResetRuntimeState(group);
}

inline bool InitializeGroupStorage(smMATERIAL_GROUP &group, int materialCapacity)
{
	ReleaseGroupStorage(group);
	if(materialCapacity < 0 || materialCapacity > MATRIAL_MAX)
		return false;
	if(materialCapacity == 0)
		return true;
	group.smMaterial = new smMATERIAL[static_cast<std::size_t>(materialCapacity)]();
	group.MaxMaterial = materialCapacity;
	return group.smMaterial != 0;
}

inline bool BoundedStringLength(const char *text, std::size_t capacity, std::size_t &length)
{
	if(!text || capacity == 0)
		return false;
	const void *terminator = std::memchr(text, 0, capacity);
	if(!terminator)
		return false;
	length = static_cast<const char *>(terminator) - text;
	return true;
}

inline bool AddSize(std::size_t increment, std::size_t &total)
{
	if(increment > static_cast<std::size_t>(std::numeric_limits<int>::max()) - total)
		return false;
	total += increment;
	return true;
}

inline bool TextureNameBytes(const smTEXTUREHANDLE *texture, std::size_t &bytes)
{
	if(!texture)
		return false;
	std::size_t nameLength = 0;
	std::size_t alphaLength = 0;
	if(!BoundedStringLength(texture->Name, TextureNameCapacity, nameLength) ||
		!BoundedStringLength(texture->NameA, TextureNameCapacity, alphaLength))
		return false;
	bytes = nameLength + 1 + alphaLength + 1;
	return true;
}

inline bool MaterialNameBytes(const smMATERIAL &material, std::size_t &bytes)
{
	bytes = 0;
	if(!material.InUse)
		return true;
	if(!HasValidTextureCounts(material))
		return false;

	for(DWORD index = 0; index < material.TextureCounter; ++index)
	{
		std::size_t pairBytes = 0;
		if(!TextureNameBytes(material.smTexture[index], pairBytes) || !AddSize(pairBytes, bytes))
			return false;
	}
	for(DWORD index = 0; index < material.AnimTexCounter; ++index)
	{
		std::size_t pairBytes = 0;
		if(!TextureNameBytes(material.smAnimTexture[index], pairBytes) || !AddSize(pairBytes, bytes))
			return false;
	}
	return bytes <= MaxSerializedNameBytes;
}

inline int CalculateSaveSize(const smMATERIAL_GROUP &group, std::size_t serializedGroupBytes)
{
	if(serializedGroupBytes == 0 || group.MaterialCount > static_cast<DWORD>(MATRIAL_MAX) ||
		(group.MaterialCount > 0 && !group.smMaterial))
		return 0;

	std::size_t total = serializedGroupBytes;
	for(DWORD index = 0; index < group.MaterialCount; ++index)
	{
		if(!AddSize(sizeof(smMATERIAL), total))
			return 0;
		if(group.smMaterial[index].InUse)
		{
			std::size_t nameBytes = 0;
			if(!MaterialNameBytes(group.smMaterial[index], nameBytes) ||
				!AddSize(sizeof(int), total) || !AddSize(nameBytes, total))
				return 0;
		}
	}
	return static_cast<int>(total);
}

inline bool WriteTextureNames(HANDLE file, const smTEXTUREHANDLE *texture, std::size_t &totalBytes)
{
	std::size_t nameLength = 0;
	std::size_t alphaLength = 0;
	if(!texture || !BoundedStringLength(texture->Name, TextureNameCapacity, nameLength) ||
		!BoundedStringLength(texture->NameA, TextureNameCapacity, alphaLength))
		return false;
	const DWORD nameBytes = static_cast<DWORD>(nameLength + 1);
	const DWORD alphaBytes = static_cast<DWORD>(alphaLength + 1);
	if(!summit::io::legacy_binary::WriteExact(file, texture->Name, nameBytes) ||
		!summit::io::legacy_binary::WriteExact(file, texture->NameA, alphaBytes))
		return false;
	totalBytes += nameBytes + alphaBytes;
	return true;
}

inline int SaveGroup(HANDLE file, const smMATERIAL_GROUP &group,
	const void *serializedGroupStart, std::size_t serializedGroupBytes)
{
	const int expectedSize = CalculateSaveSize(group, serializedGroupBytes);
	if(expectedSize <= 0 || !serializedGroupStart || serializedGroupBytes > std::numeric_limits<DWORD>::max())
		return 0;

	std::size_t totalBytes = 0;
	if(!summit::io::legacy_binary::WriteExact(file, serializedGroupStart,
		static_cast<DWORD>(serializedGroupBytes)))
		return 0;
	totalBytes += serializedGroupBytes;

	for(DWORD index = 0; index < group.MaterialCount; ++index)
	{
		const smMATERIAL &material = group.smMaterial[index];
		if(!summit::io::legacy_binary::WriteExact(file, &material, sizeof(material)))
			return 0;
		totalBytes += sizeof(material);
		if(!material.InUse)
			continue;

		std::size_t nameBytes = 0;
		if(!MaterialNameBytes(material, nameBytes) || nameBytes > static_cast<std::size_t>(std::numeric_limits<int>::max()))
			return 0;
		const int serializedNameBytes = static_cast<int>(nameBytes);
		if(!summit::io::legacy_binary::WriteExact(file, &serializedNameBytes, sizeof(serializedNameBytes)))
			return 0;
		totalBytes += sizeof(serializedNameBytes);

		for(DWORD textureIndex = 0; textureIndex < material.TextureCounter; ++textureIndex)
		{
			if(!WriteTextureNames(file, material.smTexture[textureIndex], totalBytes))
				return 0;
		}
		for(DWORD textureIndex = 0; textureIndex < material.AnimTexCounter; ++textureIndex)
		{
			if(!WriteTextureNames(file, material.smAnimTexture[textureIndex], totalBytes))
				return 0;
		}
	}
	return totalBytes == static_cast<std::size_t>(expectedSize) ? expectedSize : 0;
}

inline bool ReadField(const std::vector<unsigned char> &raw, const void *objectStart,
	const void *fieldAddress, void *value, std::size_t valueBytes)
{
	if(!objectStart || !fieldAddress || !value)
		return false;
	const std::uintptr_t base = reinterpret_cast<std::uintptr_t>(objectStart);
	const std::uintptr_t field = reinterpret_cast<std::uintptr_t>(fieldAddress);
	if(field < base)
		return false;
	const std::size_t offset = static_cast<std::size_t>(field - base);
	if(offset > raw.size() || valueBytes > raw.size() - offset)
		return false;
	std::memcpy(value, &raw[offset], valueBytes);
	return true;
}

template <typename T>
inline bool ReadField(const std::vector<unsigned char> &raw, const void *objectStart,
	const T *fieldAddress, T &value)
{
	return ReadField(raw, objectStart, fieldAddress, &value, sizeof(value));
}

struct SerializedGroupState
{
	DWORD materialCount;
	int reformTexture;
	int maxMaterial;
	int lastSearchMaterial;
	char lastSearchName[64];
};

inline bool DecodeGroupState(const std::vector<unsigned char> &raw, const smMATERIAL_GROUP &layout,
	const void *layoutStart, SerializedGroupState &state)
{
	std::memset(&state, 0, sizeof(state));
	return ReadField(raw, layoutStart, &layout.MaterialCount, state.materialCount) &&
		ReadField(raw, layoutStart, &layout.ReformTexture, state.reformTexture) &&
		ReadField(raw, layoutStart, &layout.MaxMaterial, state.maxMaterial) &&
		ReadField(raw, layoutStart, &layout.LastSearchMaterial, state.lastSearchMaterial) &&
		ReadField(raw, layoutStart, layout.szLastSearchName, state.lastSearchName, sizeof(state.lastSearchName));
}

inline bool ValidateGroupState(const SerializedGroupState &state)
{
	return state.materialCount <= static_cast<DWORD>(MATRIAL_MAX) &&
		state.maxMaterial >= 0 && state.maxMaterial <= MATRIAL_MAX &&
		state.materialCount <= static_cast<DWORD>(state.maxMaterial);
}

inline bool ReadCString(const std::vector<char> &buffer, std::size_t &offset, char *&value)
{
	if(offset >= buffer.size())
		return false;
	value = const_cast<char *>(&buffer[offset]);
	const void *terminator = std::memchr(value, 0, buffer.size() - offset);
	if(!terminator)
		return false;
	const std::size_t length = static_cast<const char *>(terminator) - value;
	if(length >= TextureNameCapacity)
		return false;
	offset += length + 1;
	return true;
}

inline bool ReadTexturePair(const std::vector<char> &buffer, std::size_t &offset, smTEXTUREHANDLE *&texture)
{
	char *name = 0;
	char *alphaName = 0;
	if(!ReadCString(buffer, offset, name) || !ReadCString(buffer, offset, alphaName))
		return false;
	texture = alphaName[0] ? smTexture.Add(name, alphaName) : smTexture.Add(name);
	return texture != 0;
}

inline bool LoadMaterialNames(HANDLE file, smMATERIAL &material, std::size_t &totalBytes)
{
	int serializedNameBytes = 0;
	if(!summit::io::legacy_binary::ReadExact(file, &serializedNameBytes, sizeof(serializedNameBytes)))
		return false;
	totalBytes += sizeof(serializedNameBytes);
	if(serializedNameBytes < 0 || static_cast<std::size_t>(serializedNameBytes) > MaxSerializedNameBytes)
		return false;
	if((material.TextureCounter + material.AnimTexCounter) > 0 && serializedNameBytes == 0)
		return false;

	std::vector<char> names(static_cast<std::size_t>(serializedNameBytes));
	if(!summit::io::legacy_binary::ReadExact(file, names.empty() ? 0 : &names[0],
		static_cast<DWORD>(names.size())))
		return false;
	totalBytes += names.size();

	std::size_t offset = 0;
	for(DWORD index = 0; index < material.TextureCounter; ++index)
	{
		if(!ReadTexturePair(names, offset, material.smTexture[index]))
			return false;
	}
	for(DWORD index = 0; index < material.AnimTexCounter; ++index)
	{
		if(!ReadTexturePair(names, offset, material.smAnimTexture[index]))
			return false;
	}
	return offset == names.size();
}

inline int LoadGroup(HANDLE file, smMATERIAL_GROUP &group, void *serializedGroupStart,
	std::size_t serializedGroupBytes)
{
	if(file == INVALID_HANDLE_VALUE || !serializedGroupStart || serializedGroupBytes == 0 ||
		serializedGroupBytes > std::numeric_limits<DWORD>::max())
		return 0;

	std::vector<unsigned char> rawHeader(serializedGroupBytes);
	if(!summit::io::legacy_binary::ReadExact(file, &rawHeader[0], static_cast<DWORD>(rawHeader.size())))
		return 0;

	SerializedGroupState state;
	if(!DecodeGroupState(rawHeader, group, serializedGroupStart, state) || !ValidateGroupState(state))
		return 0;

	smMATERIAL_GROUP loaded;
	ResetRuntimeState(loaded);
	const int allocationCapacity = state.maxMaterial;
	if(allocationCapacity > 0)
		loaded.smMaterial = new smMATERIAL[static_cast<std::size_t>(allocationCapacity)]();
	loaded.MaxMaterial = allocationCapacity;
	loaded.MaterialCount = state.materialCount;
	loaded.ReformTexture = state.reformTexture;
	loaded.LastSearchMaterial = state.lastSearchMaterial;
	std::memcpy(loaded.szLastSearchName, state.lastSearchName, sizeof(loaded.szLastSearchName));
	loaded.szLastSearchName[sizeof(loaded.szLastSearchName) - 1] = 0;

	std::size_t totalBytes = serializedGroupBytes;
	for(DWORD index = 0; index < loaded.MaterialCount; ++index)
	{
		smMATERIAL &material = loaded.smMaterial[index];
		if(!summit::io::legacy_binary::ReadExact(file, &material, sizeof(material)))
			return 0;
		totalBytes += sizeof(material);
		ClearRuntimeTexturePointers(material);
		if(!HasValidTextureCounts(material))
		{
			material.TextureCounter = 0;
			material.AnimTexCounter = 0;
			return 0;
		}
		if(material.InUse && !LoadMaterialNames(file, material, totalBytes))
			return 0;
	}

	ReleaseGroupStorage(group);
	if(rawHeader.size() < sizeof(DWORD))
		return 0;
	std::memcpy(serializedGroupStart, &rawHeader[0], sizeof(DWORD));
	group.smMaterial = loaded.smMaterial;
	loaded.smMaterial = 0;
	group.MaterialCount = loaded.MaterialCount;
	group.ReformTexture = loaded.ReformTexture;
	group.MaxMaterial = loaded.MaxMaterial;
	group.LastSearchMaterial = loaded.LastSearchMaterial;
	std::memcpy(group.szLastSearchName, loaded.szLastSearchName, sizeof(group.szLastSearchName));
	ResetRuntimeState(loaded);
	return totalBytes <= static_cast<std::size_t>(std::numeric_limits<int>::max()) ? static_cast<int>(totalBytes) : 0;
}

} // namespace material_serialization
} // namespace legacy3d
} // namespace rendering
} // namespace summit
