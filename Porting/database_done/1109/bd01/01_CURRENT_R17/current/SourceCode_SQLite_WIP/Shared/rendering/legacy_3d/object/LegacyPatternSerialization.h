#pragma once

#include <cstring>
#include <limits>
#include <memory>
#include <vector>

#include "io/LegacyWin32Handle.h"

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace pattern_serialization
{

static const int PatternObjectCapacity = 128;

inline bool CheckedAddFileOffset(int &offset, int increment)
{
	if(increment < 0 || offset < 0 || increment > std::numeric_limits<int>::max() - offset)
		return false;
	offset += increment;
	return true;
}

inline bool CopyFixedString(char *destination, std::size_t capacity, const char *source)
{
	if(!destination || capacity == 0 || !source)
		return false;
	std::memset(destination, 0, capacity);
	const std::size_t length = std::strlen(source);
	if(length >= capacity)
		return false;
	std::memcpy(destination, source, length);
	return true;
}

inline bool ValidatePatternHeader(const smDFILE_HEADER &header, const char *expectedHeader)
{
	if(!expectedHeader || std::strncmp(header.szHeader, expectedHeader, sizeof(header.szHeader)) != 0)
		return false;
	if(header.ObjCounter < 0 || header.ObjCounter > PatternObjectCapacity ||
		header.MatCounter < 0 || header.MatCounter > MATRIAL_MAX ||
		header.TmFrameCounter < 0 || header.TmFrameCounter > OBJ_FRAME_SEARCH_MAX)
		return false;

	DWORD objectInfoBytes = 0;
	if(!summit::rendering::legacy3d::object_serialization::CheckedByteCount(
		sizeof(smDFILE_OBJINFO), header.ObjCounter, objectInfoBytes))
		return false;
	if(objectInfoBytes > static_cast<DWORD>(std::numeric_limits<int>::max() - sizeof(smDFILE_HEADER)))
		return false;
	const int expectedMaterialOffset = static_cast<int>(sizeof(smDFILE_HEADER)) + static_cast<int>(objectInfoBytes);
	return header.MatFilePoint == expectedMaterialOffset && header.First_ObjInfoPoint >= header.MatFilePoint;
}

inline bool ValidateObjectInfo(const smDFILE_HEADER &header, const smDFILE_OBJINFO &info)
{
	return info.Length > 0 && info.ObjFilePoint >= header.First_ObjInfoPoint &&
		info.ObjFilePoint <= std::numeric_limits<int>::max() - info.Length;
}

inline bool BuildPatternLayout(smPAT3D &pattern, const char *headerText,
	smDFILE_HEADER &header, std::vector<smDFILE_OBJINFO> &objectInfo)
{
	if(!headerText || pattern.nObj3d < 0 || pattern.nObj3d > PatternObjectCapacity ||
		pattern.TmFrameCnt < 0 || pattern.TmFrameCnt > OBJ_FRAME_SEARCH_MAX)
		return false;

	header = smDFILE_HEADER();
	if(!CopyFixedString(header.szHeader, sizeof(header.szHeader), headerText))
		return false;
	header.ObjCounter = pattern.nObj3d;
	if(pattern.smMaterialGroup && pattern.smMaterialGroup->MaterialCount > static_cast<DWORD>(MATRIAL_MAX))
		return false;
	header.MatCounter = pattern.smMaterialGroup ? static_cast<int>(pattern.smMaterialGroup->MaterialCount) : 0;
	header.TmFrameCounter = pattern.TmFrameCnt;
	for(int index = 0; index < OBJ_FRAME_SEARCH_MAX; ++index)
		header.TmFrame[index] = pattern.TmFrame[index];

	objectInfo.assign(static_cast<std::size_t>(pattern.nObj3d), smDFILE_OBJINFO());
	int fileOffset = static_cast<int>(sizeof(smDFILE_HEADER));
	const std::size_t objectInfoBytes = sizeof(smDFILE_OBJINFO) * objectInfo.size();
	if(objectInfoBytes > static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
		!CheckedAddFileOffset(fileOffset, static_cast<int>(objectInfoBytes)))
		return false;
	header.MatFilePoint = fileOffset;

	if(pattern.smMaterialGroup)
	{
		const int materialBytes = pattern.smMaterialGroup->GetSaveSize();
		if(materialBytes <= 0 || !CheckedAddFileOffset(fileOffset, materialBytes))
			return false;
	}
	header.First_ObjInfoPoint = fileOffset;

	for(int index = 0; index < pattern.nObj3d; ++index)
	{
		smOBJ3D *object = pattern.obj3d[index];
		if(!object || !CopyFixedString(objectInfo[index].szNodeName,
			sizeof(objectInfo[index].szNodeName), object->NodeName))
			return false;
		const int objectBytes = object->GetSaveSize();
		if(objectBytes <= 0)
			return false;
		objectInfo[index].ObjFilePoint = fileOffset;
		objectInfo[index].Length = objectBytes;
		if(!CheckedAddFileOffset(fileOffset, objectBytes))
			return false;
	}
	return true;
}

inline bool SeekTo(HANDLE file, int offset)
{
	if(file == INVALID_HANDLE_VALUE || offset < 0)
		return false;
	return ::SetFilePointer(file, offset, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER;
}

inline bool SavePattern(smPAT3D &pattern, const char *fileName, const char *headerText)
{
	if(!fileName || !fileName[0])
		return false;

	smDFILE_HEADER header;
	std::vector<smDFILE_OBJINFO> objectInfo;
	if(!BuildPatternLayout(pattern, headerText, header, objectInfo))
		return false;

	summit::io::legacy_win32::ScopedHandle file(::CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));
	if(!file.IsValid())
		return false;

	using summit::rendering::legacy3d::object_serialization::WriteExact;
	DWORD objectInfoBytes = 0;
	if(!summit::rendering::legacy3d::object_serialization::CheckedByteCount(
		sizeof(smDFILE_OBJINFO), header.ObjCounter, objectInfoBytes) ||
		!WriteExact(file.Get(), &header, sizeof(header)) ||
		!WriteExact(file.Get(), objectInfo.empty() ? 0 : &objectInfo[0], objectInfoBytes))
		return false;

	if(pattern.smMaterialGroup && pattern.smMaterialGroup->SaveFile(file.Get()) <= 0)
		return false;
	for(int index = 0; index < pattern.nObj3d; ++index)
	{
		if(pattern.obj3d[index]->SaveFile(file.Get()) <= 0)
			return false;
	}
	return true;
}

inline bool LoadPatternMaterialGroup(HANDLE file, const smDFILE_HEADER &header, smPAT3D &pattern)
{
	if(header.MatCounter <= 0)
		return true;

	std::unique_ptr<smMATERIAL_GROUP> materialGroup(new smMATERIAL_GROUP);
	if(materialGroup->LoadFile(file) <= 0 || static_cast<int>(materialGroup->MaterialCount) != header.MatCounter)
		return false;
	pattern.smMaterialGroup = materialGroup.release();
	return true;
}

inline int FindObjectInfo(const std::vector<smDFILE_OBJINFO> &objectInfo, const char *nodeName)
{
	if(!nodeName || !nodeName[0])
		return -1;
	for(std::size_t index = 0; index < objectInfo.size(); ++index)
	{
		if(lstrcmpi(nodeName, objectInfo[index].szNodeName) == 0)
			return static_cast<int>(index);
	}
	return -1;
}

inline bool LoadObjectAt(HANDLE file, const smDFILE_HEADER &header, const smDFILE_OBJINFO &info,
	smPAT3D &pattern, smPAT3D *physiquePattern)
{
	if(!ValidateObjectInfo(header, info) || !SeekTo(file, info.ObjFilePoint))
		return false;

	std::unique_ptr<smOBJ3D> object(new smOBJ3D);
	const int bytes = object->LoadFile(file, physiquePattern);
	if(bytes <= 0 || bytes != info.Length || !pattern.AddObject(object.get()))
		return false;
	object.release();
	return true;
}

inline bool LoadPattern(smPAT3D &pattern, const char *fileName, const char *nodeName,
	const char *headerText, smPAT3D *physiquePattern)
{
	if(!fileName || !fileName[0])
		return false;

	summit::io::legacy_win32::ScopedHandle file(::CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
	if(!file.IsValid())
		return false;

	using summit::rendering::legacy3d::object_serialization::ReadExact;
	smDFILE_HEADER header;
	if(!ReadExact(file.Get(), &header, sizeof(header)) || !ValidatePatternHeader(header, headerText))
		return false;

	std::vector<smDFILE_OBJINFO> objectInfo(static_cast<std::size_t>(header.ObjCounter));
	DWORD objectInfoBytes = 0;
	if(!summit::rendering::legacy3d::object_serialization::CheckedByteCount(
		sizeof(smDFILE_OBJINFO), header.ObjCounter, objectInfoBytes) ||
		!ReadExact(file.Get(), objectInfo.empty() ? 0 : &objectInfo[0], objectInfoBytes))
		return false;
	for(std::size_t index = 0; index < objectInfo.size(); ++index)
	{
		objectInfo[index].szNodeName[sizeof(objectInfo[index].szNodeName) - 1] = 0;
		if(!ValidateObjectInfo(header, objectInfo[index]))
			return false;
	}

	// Build the entire replacement off to the side. The destination pattern is
	// untouched until every material/object/link has been validated.
	smPAT3D staged;
	staged.TmFrameCnt = header.TmFrameCounter;
	for(int index = 0; index < OBJ_FRAME_SEARCH_MAX; ++index)
		staged.TmFrame[index] = header.TmFrame[index];
	if(!SeekTo(file.Get(), header.MatFilePoint) || !LoadPatternMaterialGroup(file.Get(), header, staged))
		return false;

	if(nodeName && nodeName[0])
	{
		const int selected = FindObjectInfo(objectInfo, nodeName);
		if(selected < 0 || !LoadObjectAt(file.Get(), header, objectInfo[selected], staged, physiquePattern))
			return false;
	}
	else
	{
		for(std::size_t index = 0; index < objectInfo.size(); ++index)
		{
			if(!LoadObjectAt(file.Get(), header, objectInfo[index], staged, physiquePattern))
				return false;
		}
		if(!staged.LinkObject())
			return false;
	}

	staged.TmParent = physiquePattern;
	summit::rendering::legacy3d::pattern_lifecycle::SwapRuntimeState(pattern, staged);
	return true;
}

inline bool HasValidPatternHeader(const char *fileName, const char *headerText)
{
	if(!fileName || !fileName[0])
		return false;
	summit::io::legacy_win32::ScopedHandle file(::CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
	if(!file.IsValid())
		return false;

	smDFILE_HEADER header;
	return summit::rendering::legacy3d::object_serialization::ReadExact(file.Get(), &header, sizeof(header)) &&
		ValidatePatternHeader(header, headerText);
}

} // namespace pattern_serialization
} // namespace legacy3d
} // namespace rendering
} // namespace summit
