#pragma once

#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <vector>

#include "io/LegacyBinaryIO.h"
#include "rendering/legacy_3d/object/LegacyObjectDiskLayout.h"

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace object_serialization
{

inline bool CheckedByteCount(std::size_t elementSize, int count, DWORD &bytes)
{
	return summit::io::legacy_binary::CheckedByteCount(elementSize, count, bytes);
}

inline bool ReadExact(HANDLE file, void *destination, DWORD bytes)
{
	return summit::io::legacy_binary::ReadExact(file, destination, bytes);
}

inline bool WriteExact(HANDLE file, const void *source, DWORD bytes)
{
	return summit::io::legacy_binary::WriteExact(file, source, bytes);
}

template <typename T>
inline bool ReadArray(HANDLE file, T *destination, int count, std::size_t &totalBytes)
{
	DWORD bytes = 0;
	if(!CheckedByteCount(sizeof(T), count, bytes) || !ReadExact(file, destination, bytes))
		return false;
	totalBytes += bytes;
	return true;
}

template <typename T>
inline bool WriteArray(HANDLE file, const T *source, int count, std::size_t &totalBytes)
{
	DWORD bytes = 0;
	if(!CheckedByteCount(sizeof(T), count, bytes) || !WriteExact(file, source, bytes))
		return false;
	totalBytes += bytes;
	return true;
}

inline bool FrameRangeFits(const smFRAME_POS &frame, int trackCount)
{
	return frame.PosNum >= 0 && frame.PosCnt >= 0 && frame.PosNum <= trackCount &&
		frame.PosCnt <= trackCount - frame.PosNum;
}

inline bool IsValidObjectCounts(const LegacyObjectDiskLayout &disk)
{
	using namespace summit::rendering::legacy3d::object_storage;
	if(!IsValidGeometryCapacity(disk.MaxVertex, disk.MaxFace) ||
		disk.MaxVertex > RENDER_VERTEX_MAX || disk.MaxFace > RENDER_FACE_MAX ||
		disk.nVertex < 0 || disk.nVertex > disk.MaxVertex ||
		disk.nFace < 0 || disk.nFace > disk.MaxFace ||
		disk.nTexLink < 0 || disk.nTexLink > disk.MaxFace * TextureLinksPerFaceCapacity ||
		disk.TmRotCnt < 0 || disk.TmPosCnt < 0 || disk.TmScaleCnt < 0 ||
		disk.TmFrameCnt < 0 || disk.TmFrameCnt > OBJ_FRAME_SEARCH_MAX ||
		!FixedStringIsTerminated(disk.NodeName, sizeof(disk.NodeName)) ||
		!FixedStringIsTerminated(disk.NodeParent, sizeof(disk.NodeParent)))
		return false;

	for(int index = 0; index < disk.TmFrameCnt; ++index)
	{
		if(!FrameRangeFits(disk.TmRotFrame[index], disk.TmRotCnt) ||
			!FrameRangeFits(disk.TmPosFrame[index], disk.TmPosCnt) ||
			!FrameRangeFits(disk.TmScaleFrame[index], disk.TmScaleCnt))
			return false;
	}

	DWORD ignored = 0;
	return CheckedByteCount(sizeof(smVERTEX), disk.nVertex, ignored) &&
		CheckedByteCount(sizeof(LegacyFaceDisk32), disk.nFace, ignored) &&
		CheckedByteCount(sizeof(LegacyTextureLinkDisk32), disk.nTexLink, ignored) &&
		CheckedByteCount(sizeof(smTM_ROT), disk.TmRotCnt, ignored) &&
		CheckedByteCount(sizeof(smTM_POS), disk.TmPosCnt, ignored) &&
		CheckedByteCount(sizeof(smTM_SCALE), disk.TmScaleCnt, ignored) &&
		CheckedByteCount(sizeof(smFMATRIX), disk.TmRotCnt, ignored) &&
		CheckedByteCount(32, disk.nVertex, ignored);
}

inline bool HasRequiredRuntimeStorage(const smOBJ3D &object)
{
	return (object.nVertex == 0 || object.Vertex != 0) &&
		(object.nFace == 0 || object.Face != 0) &&
		(object.nTexLink == 0 || object.TexLink != 0) &&
		(object.TmRotCnt == 0 || (object.TmRot != 0 && object.TmPrevRot != 0)) &&
		(object.TmPosCnt == 0 || object.TmPos != 0) &&
		(object.TmScaleCnt == 0 || object.TmScale != 0);
}

inline bool IsValidObjectCounts(const smOBJ3D &object)
{
	const LegacyObjectDiskLayout disk = MakeLegacyObjectDiskLayout(object, 0);
	return IsValidObjectCounts(disk) && HasRequiredRuntimeStorage(object);
}

inline bool RuntimeTextureIndex(const smOBJ3D &object, const smTEXLINK *pointer, int &index)
{
	if(!pointer)
	{
		index = -1;
		return true;
	}
	if(!object.TexLink || object.nTexLink <= 0)
		return false;

	const std::uintptr_t base = reinterpret_cast<std::uintptr_t>(object.TexLink);
	const std::uintptr_t address = reinterpret_cast<std::uintptr_t>(pointer);
	const std::uintptr_t bytes = sizeof(smTEXLINK) * static_cast<std::uintptr_t>(object.nTexLink);
	if(address < base || address >= base + bytes)
		return false;
	const std::uintptr_t offset = address - base;
	if((offset % sizeof(smTEXLINK)) != 0)
		return false;
	index = static_cast<int>(offset / sizeof(smTEXLINK));
	return index >= 0 && index < object.nTexLink;
}

inline bool EncodeTextureToken(int index, LegacyPointer32 &token)
{
	if(index < 0)
	{
		token = 0;
		return true;
	}
	const std::uint64_t offset = static_cast<std::uint64_t>(index) * LegacyTextureLinkBytes;
	const std::uint64_t value = static_cast<std::uint64_t>(LegacyTextureLinkBase) + offset;
	if(value > std::numeric_limits<LegacyPointer32>::max())
		return false;
	token = static_cast<LegacyPointer32>(value);
	return true;
}

inline bool DecodeTextureToken(LegacyPointer32 serializedBase, LegacyPointer32 token,
	int textureCount, int &index)
{
	if(token == 0)
	{
		index = -1;
		return true;
	}
	if(serializedBase == 0 || textureCount <= 0 || token < serializedBase)
		return false;
	const std::uint64_t offset = static_cast<std::uint64_t>(token) - serializedBase;
	if((offset % LegacyTextureLinkBytes) != 0)
		return false;
	const std::uint64_t decoded = offset / LegacyTextureLinkBytes;
	if(decoded >= static_cast<std::uint64_t>(textureCount))
		return false;
	index = static_cast<int>(decoded);
	return true;
}

inline bool BuildDiskFaces(const smOBJ3D &object, std::vector<LegacyFaceDisk32> &faces)
{
	faces.assign(static_cast<std::size_t>(object.nFace), LegacyFaceDisk32());
	for(int index = 0; index < object.nFace; ++index)
	{
		const smFACE &source = object.Face[index];
		LegacyFaceDisk32 &destination = faces[index];
		std::memcpy(destination.v, source.v, sizeof(destination.v));
		std::memcpy(destination.t, source.t, sizeof(destination.t));
		int textureIndex = -1;
		if(!RuntimeTextureIndex(object, source.lpTexLink, textureIndex) ||
			!EncodeTextureToken(textureIndex, destination.textureLink))
			return false;
	}
	return true;
}

inline bool BuildDiskTextureLinks(const smOBJ3D &object,
	std::vector<LegacyTextureLinkDisk32> &textureLinks)
{
	textureLinks.assign(static_cast<std::size_t>(object.nTexLink), LegacyTextureLinkDisk32());
	for(int index = 0; index < object.nTexLink; ++index)
	{
		const smTEXLINK &source = object.TexLink[index];
		LegacyTextureLinkDisk32 &destination = textureLinks[index];
		std::memcpy(destination.u, source.u, sizeof(destination.u));
		std::memcpy(destination.v, source.v, sizeof(destination.v));
		destination.hTexture = RuntimePointerValue32(source.hTexture);
		int nextIndex = -1;
		if(!RuntimeTextureIndex(object, source.NextTex, nextIndex) ||
			!EncodeTextureToken(nextIndex, destination.nextTexture))
			return false;
	}
	return true;
}

inline bool ReadDiskFaces(HANDLE file, const LegacyObjectDiskLayout &disk, smOBJ3D &object,
	std::size_t &totalBytes)
{
	std::vector<LegacyFaceDisk32> faces(static_cast<std::size_t>(disk.nFace));
	if(!ReadArray(file, faces.empty() ? 0 : &faces[0], disk.nFace, totalBytes))
		return false;
	for(int index = 0; index < disk.nFace; ++index)
	{
		const LegacyFaceDisk32 &source = faces[index];
		smFACE &destination = object.Face[index];
		std::memcpy(destination.v, source.v, sizeof(destination.v));
		std::memcpy(destination.t, source.t, sizeof(destination.t));
		int textureIndex = -1;
		if(!DecodeTextureToken(disk.TexLink, source.textureLink, disk.nTexLink, textureIndex))
			return false;
		destination.lpTexLink = textureIndex >= 0 ? &object.TexLink[textureIndex] : 0;
	}
	return true;
}

inline bool ReadDiskTextureLinks(HANDLE file, const LegacyObjectDiskLayout &disk, smOBJ3D &object,
	std::size_t &totalBytes)
{
	std::vector<LegacyTextureLinkDisk32> links(static_cast<std::size_t>(disk.nTexLink));
	if(!ReadArray(file, links.empty() ? 0 : &links[0], disk.nTexLink, totalBytes))
		return false;
	for(int index = 0; index < disk.nTexLink; ++index)
	{
		const LegacyTextureLinkDisk32 &source = links[index];
		smTEXLINK &destination = object.TexLink[index];
		std::memcpy(destination.u, source.u, sizeof(destination.u));
		std::memcpy(destination.v, source.v, sizeof(destination.v));
		destination.hTexture = reinterpret_cast<DWORD *>(static_cast<std::uintptr_t>(source.hTexture));
		int nextIndex = -1;
		if(!DecodeTextureToken(disk.TexLink, source.nextTexture, disk.nTexLink, nextIndex))
			return false;
		destination.NextTex = nextIndex >= 0 ? &object.TexLink[nextIndex] : 0;
	}
	return true;
}

inline bool ValidateFaceTopology(const smOBJ3D &object)
{
	if(object.nVertex < 0 || object.nFace < 0 || object.nTexLink < 0)
		return false;
	for(int faceIndex = 0; faceIndex < object.nFace; ++faceIndex)
	{
		const smFACE &face = object.Face[faceIndex];
		for(int vertex = 0; vertex < 3; ++vertex)
		{
			if(static_cast<int>(face.v[vertex]) >= object.nVertex)
				return false;
		}

		const smTEXLINK *link = face.lpTexLink;
		int linkCount = 0;
		while(link)
		{
			int linkIndex = -1;
			if(!RuntimeTextureIndex(object, link, linkIndex) ||
				++linkCount > object_storage::TextureLinksPerFaceCapacity)
				return false;
			link = link->NextTex;
		}
	}
	return true;
}

inline bool WritePhysiqueNames(HANDLE file, const smOBJ3D &object, std::size_t &totalBytes)
{
	if(!object.Physique)
		return true;

	char name[32];
	for(int index = 0; index < object.nVertex; ++index)
	{
		if(!object.Physique[index] ||
			!FixedStringIsTerminated(object.Physique[index]->NodeName, sizeof(object.Physique[index]->NodeName)))
			return false;
		std::memset(name, 0, sizeof(name));
		const char *sourceName = object.Physique[index]->NodeName;
		const char *nameEnd = static_cast<const char *>(std::memchr(sourceName, 0, sizeof(name)));
		const std::size_t nameLength = static_cast<std::size_t>(nameEnd - sourceName);
		if(nameLength > 0)
			std::memcpy(name, sourceName, nameLength);
		if(!WriteExact(file, name, sizeof(name)))
			return false;
		totalBytes += sizeof(name);
	}
	return true;
}

inline bool ReadPhysiqueNames(HANDLE file, smOBJ3D &object, bool serializedPhysique,
	smPAT3D *physiquePattern, std::size_t &totalBytes)
{
	if(!serializedPhysique)
		return true;

	DWORD bytes = 0;
	if(!CheckedByteCount(32, object.nVertex, bytes))
		return false;
	std::vector<char> names(bytes);
	if(!ReadExact(file, names.empty() ? 0 : &names[0], bytes))
		return false;
	totalBytes += bytes;

	for(int index = 0; index < object.nVertex; ++index)
	{
		char *name = &names[static_cast<std::size_t>(index) * 32];
		if(!FixedStringIsTerminated(name, 32))
			return false;
	}

	if(!physiquePattern)
		return true;
	if(object.nVertex > 0 && !object.Physique)
		return false;

	for(int index = 0; index < object.nVertex; ++index)
	{
		char *name = &names[static_cast<std::size_t>(index) * 32];
		object.Physique[index] = physiquePattern->GetObjectFromName(name);
	}
	return true;
}

inline bool CheckedSerializedSize(const smOBJ3D &object, std::size_t &totalBytes)
{
	if(!IsValidObjectCounts(object) || !ValidateFaceTopology(object))
		return false;

	totalBytes = LegacyObjectHeaderBytes;
	const std::size_t components[] = {
		static_cast<std::size_t>(object.nVertex) * sizeof(smVERTEX),
		static_cast<std::size_t>(object.nFace) * LegacyFaceBytes,
		static_cast<std::size_t>(object.nTexLink) * LegacyTextureLinkBytes,
		static_cast<std::size_t>(object.TmRotCnt) * sizeof(smTM_ROT),
		static_cast<std::size_t>(object.TmPosCnt) * sizeof(smTM_POS),
		static_cast<std::size_t>(object.TmScaleCnt) * sizeof(smTM_SCALE),
		static_cast<std::size_t>(object.TmRotCnt) * sizeof(smFMATRIX),
		object.Physique ? static_cast<std::size_t>(object.nVertex) * 32u : 0u
	};
	for(std::size_t index = 0; index < sizeof(components) / sizeof(components[0]); ++index)
	{
		if(components[index] > static_cast<std::size_t>(std::numeric_limits<int>::max()) - totalBytes)
			return false;
		totalBytes += components[index];
	}
	return totalBytes <= static_cast<std::size_t>(std::numeric_limits<int>::max());
}

inline int SerializedObjectSize(const smOBJ3D &object)
{
	std::size_t bytes = 0;
	return CheckedSerializedSize(object, bytes) ? static_cast<int>(bytes) : 0;
}

inline int SaveObject(HANDLE file, const smOBJ3D &object, DWORD head)
{
	if(file == INVALID_HANDLE_VALUE || !IsValidObjectCounts(object) || !ValidateFaceTopology(object))
		return 0;

	std::vector<LegacyFaceDisk32> faces;
	std::vector<LegacyTextureLinkDisk32> textureLinks;
	if(!BuildDiskFaces(object, faces) || !BuildDiskTextureLinks(object, textureLinks))
		return 0;

	const LegacyObjectDiskLayout disk = MakeLegacyObjectDiskLayout(object, head);
	std::size_t totalBytes = 0;
	if(!WriteExact(file, &disk, sizeof(disk)))
		return 0;
	totalBytes += sizeof(disk);
	if(!WriteArray(file, object.Vertex, object.nVertex, totalBytes) ||
		!WriteArray(file, faces.empty() ? 0 : &faces[0], object.nFace, totalBytes) ||
		!WriteArray(file, textureLinks.empty() ? 0 : &textureLinks[0], object.nTexLink, totalBytes) ||
		!WriteArray(file, object.TmRot, object.TmRotCnt, totalBytes) ||
		!WriteArray(file, object.TmPos, object.TmPosCnt, totalBytes) ||
		!WriteArray(file, object.TmScale, object.TmScaleCnt, totalBytes) ||
		!WriteArray(file, object.TmPrevRot, object.TmRotCnt, totalBytes) ||
		!WritePhysiqueNames(file, object, totalBytes) ||
		totalBytes > static_cast<std::size_t>(std::numeric_limits<int>::max()))
		return 0;
	return static_cast<int>(totalBytes);
}

inline void StealObjectStorage(smOBJ3D &destination, smOBJ3D &source)
{
	destination.Vertex = source.Vertex;
	destination.WorldVertex = source.WorldVertex;
	destination.WorldVertexCapacity = source.WorldVertexCapacity;
	destination.Face = source.Face;
	destination.TexLink = source.TexLink;
	destination.Physique = source.Physique;
	destination.TmRot = source.TmRot;
	destination.TmPos = source.TmPos;
	destination.TmScale = source.TmScale;
	destination.TmPrevRot = source.TmPrevRot;

	source.Vertex = 0;
	source.WorldVertex = 0;
	source.WorldVertexCapacity = 0;
	source.Face = 0;
	source.TexLink = 0;
	source.Physique = 0;
	source.TmRot = 0;
	source.TmPos = 0;
	source.TmScale = 0;
	source.TmPrevRot = 0;
}

inline int LoadObject(HANDLE file, smOBJ3D &object, smPAT3D *physiquePattern, DWORD *loadedHead = 0)
{
	using namespace summit::rendering::legacy3d::object_storage;
	if(file == INVALID_HANDLE_VALUE)
		return 0;

	LegacyObjectDiskLayout disk = {};
	if(!ReadExact(file, &disk, sizeof(disk)) || !IsValidObjectCounts(disk))
		return 0;

	const bool serializedPhysique = disk.Physique != 0;
	smOBJ3D staged;
	if(!AllocateGeometryStorage(staged, disk.MaxVertex, disk.MaxFace,
		serializedPhysique && physiquePattern != 0))
		return 0;
	ApplyLegacyObjectState(staged, disk);

	std::unique_ptr<smTM_ROT[]> rotations(disk.TmRotCnt > 0 ? new smTM_ROT[disk.TmRotCnt] : 0);
	std::unique_ptr<smTM_POS[]> positions(disk.TmPosCnt > 0 ? new smTM_POS[disk.TmPosCnt] : 0);
	std::unique_ptr<smTM_SCALE[]> scales(disk.TmScaleCnt > 0 ? new smTM_SCALE[disk.TmScaleCnt] : 0);
	std::unique_ptr<smFMATRIX[]> previousRotations(disk.TmRotCnt > 0 ? new smFMATRIX[disk.TmRotCnt] : 0);

	std::size_t totalBytes = sizeof(disk);
	if(!ReadArray(file, staged.Vertex, disk.nVertex, totalBytes) ||
		!ReadDiskFaces(file, disk, staged, totalBytes) ||
		!ReadDiskTextureLinks(file, disk, staged, totalBytes) ||
		!ReadArray(file, rotations.get(), disk.TmRotCnt, totalBytes) ||
		!ReadArray(file, positions.get(), disk.TmPosCnt, totalBytes) ||
		!ReadArray(file, scales.get(), disk.TmScaleCnt, totalBytes) ||
		!ReadArray(file, previousRotations.get(), disk.TmRotCnt, totalBytes) ||
		!ReadPhysiqueNames(file, staged, serializedPhysique, physiquePattern, totalBytes) ||
		!ValidateFaceTopology(staged) ||
		totalBytes > static_cast<std::size_t>(std::numeric_limits<int>::max()))
		return 0;

	staged.TmRot = rotations.release();
	staged.TmPos = positions.release();
	staged.TmScale = scales.release();
	staged.TmPrevRot = previousRotations.release();

	// Commit only after the complete serialized object has been validated.
	ReleaseOwnedStorage(object);
	ApplyLegacyObjectState(object, disk);
	StealObjectStorage(object, staged);
	if(loadedHead)
		*loadedHead = disk.Head;
	return static_cast<int>(totalBytes);
}

} // namespace object_serialization
} // namespace legacy3d
} // namespace rendering
} // namespace summit
