#pragma once

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <vector>

#include "io/LegacyBinaryIO.h"
#include "io/LegacyWin32File.h"
#include "rendering/legacy_3d/stage/StageStorage.h"
#include "rendering/legacy_3d/stage/StageAreaStorage.h"
#include "rendering/legacy_3d/stage/LegacyStageDiskLayout.h"

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_serialization
{

struct SerializedStageState
{
	SerializedStageState()
		: head(0), memMode(0), sumCount(0), calcSumCount(0), nVertex(0), nFace(0),
		  nTexLink(0), nLight(0), nVertColor(0), contrast(0), bright(0), wAreaSize(0),
		  serializedTexLinkBase(0)
	{
		vectLight = POINT3D();
		stageMapRect = RECT();
		std::memset(areaPresent, 0, sizeof(areaPresent));
	}

	DWORD head;
	int memMode;
	DWORD sumCount;
	int calcSumCount;
	int nVertex;
	int nFace;
	int nTexLink;
	int nLight;
	int nVertColor;
	int contrast;
	int bright;
	POINT3D vectLight;
	int wAreaSize;
	RECT stageMapRect;
	std::uintptr_t serializedTexLinkBase;
	bool areaPresent[MAP_SIZE][MAP_SIZE];
};

inline bool HeaderMatches(const smDFILE_HEADER &header, const char *expectedHeader)
{
	return expectedHeader &&
		std::strncmp(header.szHeader, expectedHeader, sizeof(header.szHeader)) == 0 &&
		header.MatCounter >= 0 && header.MatCounter <= MATRIAL_MAX;
}

inline bool CopyHeaderText(smDFILE_HEADER &header, const char *text)
{
	if(!text)
		return false;
	const std::size_t length = std::strlen(text);
	if(length >= sizeof(header.szHeader))
		return false;
	header = smDFILE_HEADER();
	std::memcpy(header.szHeader, text, length);
	return true;
}

inline bool DecodeStageState(const LegacyStageDiskLayout &layout, SerializedStageState &state)
{
	state.head = layout.Head;
	state.memMode = layout.MemMode;
	state.sumCount = layout.SumCount;
	state.calcSumCount = layout.CalcSumCount;
	state.nVertex = layout.nVertex;
	state.nFace = layout.nFace;
	state.nTexLink = layout.nTexLink;
	state.nLight = layout.nLight;
	state.nVertColor = layout.nVertColor;
	state.contrast = layout.Contrast;
	state.bright = layout.Bright;
	state.vectLight = layout.VectLight;
	state.wAreaSize = layout.wAreaSize;
	state.stageMapRect = layout.StageMapRect;
	state.serializedTexLinkBase = reinterpret_cast<std::uintptr_t>(layout.TexLink);

	for(int x = 0; x < MAP_SIZE; ++x)
	{
		for(int y = 0; y < MAP_SIZE; ++y)
			state.areaPresent[x][y] = layout.StageArea[x][y] != 0;
	}
	return true;
}

inline bool ValidateStageState(const SerializedStageState &state, DWORD fileSize)
{
	if(state.nVertex < 0 || state.nFace < 0 || state.nTexLink < 0 || state.nLight < 0 ||
		state.wAreaSize < 0)
		return false;
	DWORD vertexBytes = 0, faceBytes = 0, texLinkBytes = 0, lightBytes = 0;
	if(!summit::io::legacy_binary::CheckedByteCount(sizeof(smSTAGE_VERTEX), state.nVertex, vertexBytes) ||
		!summit::io::legacy_binary::CheckedByteCount(sizeof(smSTAGE_FACE), state.nFace, faceBytes) ||
		!summit::io::legacy_binary::CheckedByteCount(sizeof(smTEXLINK), state.nTexLink, texLinkBytes) ||
		!summit::io::legacy_binary::CheckedByteCount(sizeof(smLIGHT3D), state.nLight, lightBytes))
		return false;
	const std::uint64_t coreBytes = static_cast<std::uint64_t>(vertexBytes) + faceBytes + texLinkBytes + lightBytes;
	if(coreBytes > fileSize)
		return false;
	if(state.wAreaSize > 0 && static_cast<std::uint64_t>(state.wAreaSize) * sizeof(WORD) > fileSize)
		return false;
	if(state.nTexLink > 0 && state.serializedTexLinkBase == 0)
		return false;
	return true;
}

inline bool ValidateFaceTopology(const smSTAGE_FACE *faces, int faceCount, int vertexCount)
{
	if(faceCount < 0 || vertexCount < 0 || (faceCount > 0 && !faces))
		return false;
	for(int faceIndex = 0; faceIndex < faceCount; ++faceIndex)
	{
		for(int corner = 0; corner < 3; ++corner)
		{
			if(static_cast<int>(faces[faceIndex].Vertex[corner]) >= vertexCount)
				return false;
		}
	}
	return true;
}

inline void ResetRuntimeFaceMarks(smSTAGE_FACE *faces, int faceCount)
{
	if(!faces || faceCount <= 0)
		return;
	for(int faceIndex = 0; faceIndex < faceCount; ++faceIndex)
		faces[faceIndex].CalcSum = 0;
}

inline bool RebaseSerializedTexLinkPointer(std::uintptr_t serializedBase, int texLinkCount,
	smTEXLINK *newBase, smTEXLINK *serializedPointer, smTEXLINK *&rebased)
{
	if(!serializedPointer)
	{
		rebased = 0;
		return true;
	}
	if(!newBase || texLinkCount <= 0 || serializedBase == 0)
		return false;
	const std::uintptr_t pointerValue = reinterpret_cast<std::uintptr_t>(serializedPointer);
	if(pointerValue < serializedBase)
		return false;
	const std::uintptr_t delta = pointerValue - serializedBase;
	if(delta % sizeof(smTEXLINK) != 0)
		return false;
	const std::uintptr_t index = delta / sizeof(smTEXLINK);
	if(index >= static_cast<std::uintptr_t>(texLinkCount))
		return false;
	rebased = newBase + index;
	return true;
}

inline bool RebaseTextureLinks(const SerializedStageState &state, smSTAGE_FACE *faces,
	smTEXLINK *links)
{
	for(int index = 0; index < state.nTexLink; ++index)
	{
		smTEXLINK *rebased = 0;
		if(!RebaseSerializedTexLinkPointer(state.serializedTexLinkBase, state.nTexLink,
			links, links[index].NextTex, rebased))
			return false;
		links[index].NextTex = rebased;
	}
	for(int index = 0; index < state.nFace; ++index)
	{
		smTEXLINK *rebased = 0;
		if(!RebaseSerializedTexLinkPointer(state.serializedTexLinkBase, state.nTexLink,
			links, faces[index].lpTexLink, rebased))
			return false;
		faces[index].lpTexLink = rebased;
	}
	return true;
}

struct AreaData
{
	AreaData() : offsets(static_cast<std::size_t>(MAP_SIZE) * MAP_SIZE, -1) {}
	std::vector<int> offsets;
	std::vector<DWORD> words;
};

inline bool ReadBlobValue(const std::vector<unsigned char> &blob, std::size_t &offset,
	void *value, std::size_t bytes)
{
	if(!value || offset > blob.size() || bytes > blob.size() - offset)
		return false;
	std::memcpy(value, &blob[offset], bytes);
	offset += bytes;
	return true;
}

inline bool ParseAreaBlob(const std::vector<unsigned char> &blob, const SerializedStageState &state,
	std::size_t entryBytes, AreaData &area)
{
	if(entryBytes != sizeof(WORD) && entryBytes != sizeof(DWORD))
		return false;
	AreaData parsed;
	std::size_t offset = 0;
	for(int y = 0; y < MAP_SIZE; ++y)
	{
		for(int x = 0; x < MAP_SIZE; ++x)
		{
			if(!state.areaPresent[x][y])
				continue;
			int serializedLength = 0;
			if(!ReadBlobValue(blob, offset, &serializedLength, sizeof(serializedLength)) ||
				serializedLength <= 0 || static_cast<long long>(serializedLength) > static_cast<long long>(state.nFace) + 1)
				return false;
			const std::size_t listBytes = static_cast<std::size_t>(serializedLength) * entryBytes;
			if(offset > blob.size() || listBytes > blob.size() - offset)
				return false;

			const std::size_t cell = static_cast<std::size_t>(y) * MAP_SIZE + x;
			if(parsed.words.size() > static_cast<std::size_t>(INT_MAX) - serializedLength)
				return false;
			parsed.offsets[cell] = static_cast<int>(parsed.words.size());
			for(int index = 0; index < serializedLength; ++index)
			{
				DWORD value = 0;
				if(entryBytes == sizeof(DWORD))
				{
					if(!ReadBlobValue(blob, offset, &value, sizeof(DWORD)))
						return false;
				}
				else
				{
					WORD shortValue = 0;
					if(!ReadBlobValue(blob, offset, &shortValue, sizeof(WORD)))
						return false;
					value = shortValue;
				}
				if(index == 0)
				{
					if(value != static_cast<DWORD>(serializedLength - 1))
						return false;
				}
				else if(value >= static_cast<DWORD>(state.nFace))
					return false;
				parsed.words.push_back(value);
			}
		}
	}
	if(state.wAreaSize >= 0 && parsed.words.size() > static_cast<std::size_t>(state.wAreaSize))
		return false;
	area = parsed;
	return true;
}

inline bool ParseCompatibleAreaBlob(const std::vector<unsigned char> &blob,
	const SerializedStageState &state, AreaData &area)
{
	if(ParseAreaBlob(blob, state, sizeof(DWORD), area))
		return true;
	return ParseAreaBlob(blob, state, sizeof(WORD), area);
}

inline bool ReadRemainingBlob(HANDLE file, DWORD fileSize, std::vector<unsigned char> &blob)
{
	DWORD offset = 0;
	if(!summit::io::legacy_win32::Tell32(file, offset) || offset > fileSize)
		return false;
	const DWORD remaining = fileSize - offset;
	blob.assign(static_cast<std::size_t>(remaining), 0);
	return summit::io::legacy_binary::ReadExact(file, blob.empty() ? 0 : &blob[0], remaining);
}

inline bool BuildAreaStorage(const AreaData &area, std::unique_ptr<DWORD[]> &buffer)
{
	if(area.words.empty())
	{
		buffer.reset();
		return true;
	}
	buffer.reset(new(std::nothrow) DWORD[area.words.size()]);
	if(!buffer)
		return false;
	std::memcpy(buffer.get(), &area.words[0], area.words.size() * sizeof(DWORD));
	return true;
}

inline bool LoadStage(smSTAGE3D &stage, const char *fileName, const char *headerText)
{
	if(!fileName || !fileName[0])
		return false;
	using namespace summit::io::legacy_binary;
	using summit::io::legacy_win32::ScopedHandle;
	using summit::io::legacy_win32::FileSize32;
	ScopedHandle file(::CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
	if(!file.IsValid())
		return false;
	DWORD fileSize = 0;
	if(!FileSize32(file.Get(), fileSize))
		return false;

	smDFILE_HEADER header = smDFILE_HEADER();
	if(!ReadExact(file.Get(), &header, sizeof(header)) || !HeaderMatches(header, headerText))
		return false;
	LegacyStageDiskLayout diskLayout;
	if(!ReadExact(file.Get(), &diskLayout, sizeof(diskLayout)))
		return false;
	SerializedStageState state;
	if(!DecodeStageState(diskLayout, state) || !ValidateStageState(state, fileSize))
		return false;

	std::unique_ptr<smMATERIAL_GROUP> materialGroup;
	if(header.MatCounter > 0)
	{
		materialGroup.reset(new(std::nothrow) smMATERIAL_GROUP);
		if(!materialGroup || materialGroup->LoadFile(file.Get()) <= 0 ||
			static_cast<int>(materialGroup->MaterialCount) != header.MatCounter)
			return false;
	}

	std::unique_ptr<smSTAGE_VERTEX[]> vertices;
	std::unique_ptr<smSTAGE_FACE[]> faces;
	std::unique_ptr<smTEXLINK[]> links;
	std::unique_ptr<smLIGHT3D[]> lights;
	DWORD bytes = 0;
	if(state.nVertex > 0)
	{
		vertices.reset(new(std::nothrow) smSTAGE_VERTEX[static_cast<std::size_t>(state.nVertex)]);
		if(!vertices || !CheckedByteCount(sizeof(smSTAGE_VERTEX), state.nVertex, bytes) ||
			!ReadExact(file.Get(), vertices.get(), bytes)) return false;
	}
	if(state.nFace > 0)
	{
		faces.reset(new(std::nothrow) smSTAGE_FACE[static_cast<std::size_t>(state.nFace)]);
		if(!faces || !CheckedByteCount(sizeof(smSTAGE_FACE), state.nFace, bytes) ||
			!ReadExact(file.Get(), faces.get(), bytes)) return false;
	}
	if(state.nTexLink > 0)
	{
		links.reset(new(std::nothrow) smTEXLINK[static_cast<std::size_t>(state.nTexLink)]);
		if(!links || !CheckedByteCount(sizeof(smTEXLINK), state.nTexLink, bytes) ||
			!ReadExact(file.Get(), links.get(), bytes)) return false;
	}
	if(state.nLight > 0)
	{
		lights.reset(new(std::nothrow) smLIGHT3D[static_cast<std::size_t>(state.nLight)]);
		if(!lights || !CheckedByteCount(sizeof(smLIGHT3D), state.nLight, bytes) ||
			!ReadExact(file.Get(), lights.get(), bytes)) return false;
	}
	if(!ValidateFaceTopology(faces.get(), state.nFace, state.nVertex) ||
		!RebaseTextureLinks(state, faces.get(), links.get()))
		return false;
	ResetRuntimeFaceMarks(faces.get(), state.nFace);

	std::vector<unsigned char> areaBlob;
	AreaData area;
	if(!ReadRemainingBlob(file.Get(), fileSize, areaBlob) || !ParseCompatibleAreaBlob(areaBlob, state, area))
		return false;
	std::unique_ptr<DWORD[]> areaBuffer;
	if(!BuildAreaStorage(area, areaBuffer))
		return false;
	std::unique_ptr<smSTAGE_OBJECT> stageObject(new(std::nothrow) smSTAGE_OBJECT);
	if(!stageObject)
		return false;

	stage_storage::ReleaseOwnedStorage(stage);
	stage.Head = state.head;
	stage.MemMode = state.memMode;
	stage.SumCount = state.sumCount;
	stage.CalcSumCount = 1;
	stage.Vertex = vertices.release();
	stage.Face = faces.release();
	stage.TexLink = links.release();
	stage.smLight = lights.release();
	stage.smMaterialGroup = materialGroup.release();
	stage.smMaterial = stage.smMaterialGroup ? stage.smMaterialGroup->smMaterial : 0;
	stage.StageObject = stageObject.release();
	stage.nVertex = state.nVertex;
	stage.VertexCapacity = state.nVertex;
	stage.nFace = state.nFace;
	stage.FaceCapacity = state.nFace;
	stage.nTexLink = state.nTexLink;
	stage.TexLinkCapacity = state.nTexLink;
	stage.nLight = state.nLight;
	stage.LightCapacity = state.nLight;
	stage.nVertColor = state.nVertColor;
	stage.Contrast = state.contrast;
	stage.Bright = state.bright;
	stage.VectLight = state.vectLight;
	stage.StageMapRect = state.stageMapRect;
	stage.lpwAreaBuff = areaBuffer.release();
	stage.wAreaSize = static_cast<int>(area.words.size());
	std::memset(stage.StageArea, 0, sizeof(stage.StageArea));
	for(int y = 0; y < MAP_SIZE; ++y)
	{
		for(int x = 0; x < MAP_SIZE; ++x)
		{
			const int areaOffset = area.offsets[static_cast<std::size_t>(y) * MAP_SIZE + x];
			if(areaOffset >= 0)
				stage.StageArea[x][y] = stage.lpwAreaBuff + areaOffset;
		}
	}
	return true;
}

inline bool WriteAreaLists(HANDLE file, const smSTAGE3D &stage)
{
	using summit::io::legacy_binary::WriteExact;
	for(int y = 0; y < MAP_SIZE; ++y)
	{
		for(int x = 0; x < MAP_SIZE; ++x)
		{
			summit::rendering::legacy3d::stage_area_storage::AreaListView area;
			if(!summit::rendering::legacy3d::stage_area_storage::ReadAreaList(stage, x, y, area))
				return false;
			if(!area.data)
				continue;
			if(area.wordCount > static_cast<std::size_t>(std::numeric_limits<int>::max()))
				return false;
			const int serializedLength = static_cast<int>(area.wordCount);
			if(!WriteExact(file, &serializedLength, sizeof(serializedLength)) ||
				!WriteExact(file, area.data, static_cast<DWORD>(area.wordCount * sizeof(DWORD))))
				return false;
		}
	}
	return true;
}

inline bool SaveStage(smSTAGE3D &stage, const char *fileName, const char *headerText)
{
	if(!fileName || !fileName[0] || !stage_storage::HasConsistentRuntimeStorage(stage) ||
		stage.wAreaSize < 0 || !ValidateFaceTopology(stage.Face, stage.nFace, stage.nVertex))
		return false;
	smDFILE_HEADER header;
	if(!CopyHeaderText(header, headerText))
		return false;
	header.MatCounter = stage.smMaterialGroup ? static_cast<int>(stage.smMaterialGroup->MaterialCount) : 0;
	if(header.MatCounter < 0 || header.MatCounter > MATRIAL_MAX)
		return false;
	header.MatFilePoint = static_cast<int>(sizeof(smDFILE_HEADER));
	header.First_ObjInfoPoint = header.MatFilePoint;
	if(stage.smMaterialGroup)
	{
		const int materialBytes = stage.smMaterialGroup->GetSaveSize();
		if(materialBytes <= 0 || materialBytes > std::numeric_limits<int>::max() - header.First_ObjInfoPoint)
			return false;
		header.First_ObjInfoPoint += materialBytes;
	}

	using namespace summit::io::legacy_binary;
	using summit::io::legacy_win32::ScopedHandle;
	ScopedHandle file(::CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));
	if(!file.IsValid())
		return false;
	stage.Head = FALSE;
	const LegacyStageDiskLayout diskLayout = MakeLegacyStageDiskLayout(stage);
	if(!WriteExact(file.Get(), &header, sizeof(header)) ||
		!WriteExact(file.Get(), &diskLayout, sizeof(diskLayout)))
		return false;
	if(stage.smMaterialGroup && stage.smMaterialGroup->SaveFile(file.Get()) <= 0)
		return false;

	DWORD bytes = 0;
	if(!CheckedByteCount(sizeof(smSTAGE_VERTEX), stage.nVertex, bytes) ||
		!WriteExact(file.Get(), stage.Vertex, bytes)) return false;
	if(!CheckedByteCount(sizeof(smSTAGE_FACE), stage.nFace, bytes) ||
		!WriteExact(file.Get(), stage.Face, bytes)) return false;
	if(!CheckedByteCount(sizeof(smTEXLINK), stage.nTexLink, bytes) ||
		!WriteExact(file.Get(), stage.TexLink, bytes)) return false;
	if(!CheckedByteCount(sizeof(smLIGHT3D), stage.nLight, bytes) ||
		!WriteExact(file.Get(), stage.smLight, bytes)) return false;
	return WriteAreaLists(file.Get(), stage);
}

} // namespace stage_serialization
} // namespace legacy3d
} // namespace rendering
} // namespace summit
