#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace object_serialization
{

// Summit Tale SMD files were produced by the 32-bit Windows build.  Keep every
// serialized pointer-sized field explicitly 32-bit so the file format is no
// longer coupled to sizeof(void*) or to the runtime layout of smOBJ3D.
typedef std::uint32_t LegacyPointer32;

static const std::size_t LegacyObjectHeaderBytes = 2236;
static const std::size_t LegacyFaceBytes = 36;
static const std::size_t LegacyTextureLinkBytes = 32;
static const LegacyPointer32 LegacyTextureLinkBase = 0x10000000u;

#pragma pack(push, 4)
struct LegacyTextureLinkDisk32
{
	float u[3];
	float v[3];
	LegacyPointer32 hTexture;
	LegacyPointer32 nextTexture;
};

struct LegacyFaceDisk32
{
	WORD v[4];
	smFTPOINT t[3];
	LegacyPointer32 textureLink;
};

// Frozen representation of the historical raw smOBJ3D header stored in SMD
// files.  Runtime-only members belong in smOBJ3D, never in this type.
struct LegacyObjectDiskLayout
{
	DWORD Head;

	LegacyPointer32 Vertex;
	LegacyPointer32 Face;
	LegacyPointer32 TexLink;
	LegacyPointer32 Physique;

	smVERTEX ZeroVertex;

	int maxZ, minZ;
	int maxY, minY;
	int maxX, minX;

	int dBound;
	int Bound;

	int MaxVertex;
	int MaxFace;

	int nVertex;
	int nFace;
	int nTexLink;

	int ColorEffect;
	DWORD ClipStates;

	POINT3D Posi;
	POINT3D CameraPosi;
	POINT3D Angle;
	int Trig[8];

	char NodeName[32];
	char NodeParent[32];
	LegacyPointer32 pParent;

	smMATRIX Tm;
	smMATRIX TmInvert;

	smFMATRIX TmResult;
	smMATRIX TmRotate;

	smMATRIX mWorld;
	smMATRIX mLocal;

	int lFrame;

	float qx, qy, qz, qw;
	int sx, sy, sz;
	int px, py, pz;

	LegacyPointer32 TmRot;
	LegacyPointer32 TmPos;
	LegacyPointer32 TmScale;
	LegacyPointer32 TmPrevRot;

	int TmRotCnt;
	int TmPosCnt;
	int TmScaleCnt;

	smFRAME_POS TmRotFrame[OBJ_FRAME_SEARCH_MAX];
	smFRAME_POS TmPosFrame[OBJ_FRAME_SEARCH_MAX];
	smFRAME_POS TmScaleFrame[OBJ_FRAME_SEARCH_MAX];
	int TmFrameCnt;
};
#pragma pack(pop)

static_assert(sizeof(LegacyTextureLinkDisk32) == LegacyTextureLinkBytes,
	"Legacy SMD texture-link layout changed");
static_assert(sizeof(LegacyFaceDisk32) == LegacyFaceBytes,
	"Legacy SMD face layout changed");
static_assert(sizeof(LegacyObjectDiskLayout) == LegacyObjectHeaderBytes,
	"Legacy SMD object header layout changed");
static_assert(offsetof(LegacyTextureLinkDisk32, hTexture) == 24, "Legacy texture handle offset changed");
static_assert(offsetof(LegacyTextureLinkDisk32, nextTexture) == 28, "Legacy next-texture offset changed");
static_assert(offsetof(LegacyFaceDisk32, textureLink) == 32, "Legacy face texture-link offset changed");
static_assert(offsetof(LegacyObjectDiskLayout, TexLink) == 12, "Legacy object TexLink offset changed");
static_assert(offsetof(LegacyObjectDiskLayout, Physique) == 16, "Legacy object Physique offset changed");
static_assert(offsetof(LegacyObjectDiskLayout, NodeName) == 172, "Legacy object NodeName offset changed");
static_assert(offsetof(LegacyObjectDiskLayout, pParent) == 236, "Legacy object parent offset changed");
static_assert(offsetof(LegacyObjectDiskLayout, Tm) == 240, "Legacy object transform offset changed");
static_assert(offsetof(LegacyObjectDiskLayout, TmRot) == 668, "Legacy object animation pointer offset changed");
static_assert(offsetof(LegacyObjectDiskLayout, TmRotFrame) == 696, "Legacy object frame table offset changed");
static_assert(offsetof(LegacyObjectDiskLayout, TmFrameCnt) == 2232, "Legacy object frame count offset changed");

inline LegacyPointer32 PointerPresence(const void *pointer)
{
	return pointer ? 1u : 0u;
}

inline LegacyPointer32 RuntimePointerValue32(const void *pointer)
{
	if(!pointer)
		return 0;
	const std::uintptr_t value = reinterpret_cast<std::uintptr_t>(pointer);
	return value <= static_cast<std::uintptr_t>(std::numeric_limits<LegacyPointer32>::max())
		? static_cast<LegacyPointer32>(value) : 0u;
}

inline bool FixedStringIsTerminated(const char *text, std::size_t capacity)
{
	return text && std::memchr(text, 0, capacity) != 0;
}

inline LegacyObjectDiskLayout MakeLegacyObjectDiskLayout(const smOBJ3D &object, DWORD head)
{
	LegacyObjectDiskLayout disk = {};
	disk.Head = head;
	// Most pointer values in the old header were only presence flags after load.
	// Use deterministic non-zero sentinels instead of leaking process addresses.
	disk.Vertex = PointerPresence(object.Vertex);
	disk.Face = PointerPresence(object.Face);
	disk.TexLink = object.nTexLink > 0 ? LegacyTextureLinkBase : 0u;
	disk.Physique = PointerPresence(object.Physique);
	disk.ZeroVertex = object.ZeroVertex;
	disk.maxZ = object.maxZ;
	disk.minZ = object.minZ;
	disk.maxY = object.maxY;
	disk.minY = object.minY;
	disk.maxX = object.maxX;
	disk.minX = object.minX;
	disk.dBound = object.dBound;
	disk.Bound = object.Bound;
	disk.MaxVertex = object.MaxVertex;
	disk.MaxFace = object.MaxFace;
	disk.nVertex = object.nVertex;
	disk.nFace = object.nFace;
	disk.nTexLink = object.nTexLink;
	disk.ColorEffect = object.ColorEffect;
	disk.ClipStates = object.ClipStates;
	disk.Posi = object.Posi;
	disk.CameraPosi = object.CameraPosi;
	disk.Angle = object.Angle;
	// Trig[] was transient visibility state serialized accidentally by the legacy class dump.
	// Keep the bytes in the disk contract but write deterministic zeros.
	for(int index = 0; index < 8; ++index)
		disk.Trig[index] = 0;
	std::memcpy(disk.NodeName, object.NodeName, sizeof(disk.NodeName));
	std::memcpy(disk.NodeParent, object.NodeParent, sizeof(disk.NodeParent));
	// Parent links are rebuilt by smPAT3D::LinkObject after a full pattern load.
	disk.pParent = 0;
	disk.Tm = object.Tm;
	disk.TmInvert = object.TmInvert;
	disk.TmResult = object.TmResult;
	disk.TmRotate = object.TmRotate;
	disk.mWorld = object.mWorld;
	disk.mLocal = object.mLocal;
	disk.lFrame = object.lFrame;
	disk.qx = object.qx;
	disk.qy = object.qy;
	disk.qz = object.qz;
	disk.qw = object.qw;
	disk.sx = object.sx;
	disk.sy = object.sy;
	disk.sz = object.sz;
	disk.px = object.px;
	disk.py = object.py;
	disk.pz = object.pz;
	disk.TmRot = PointerPresence(object.TmRot);
	disk.TmPos = PointerPresence(object.TmPos);
	disk.TmScale = PointerPresence(object.TmScale);
	disk.TmPrevRot = PointerPresence(object.TmPrevRot);
	disk.TmRotCnt = object.TmRotCnt;
	disk.TmPosCnt = object.TmPosCnt;
	disk.TmScaleCnt = object.TmScaleCnt;
	std::memcpy(disk.TmRotFrame, object.TmRotFrame, sizeof(disk.TmRotFrame));
	std::memcpy(disk.TmPosFrame, object.TmPosFrame, sizeof(disk.TmPosFrame));
	std::memcpy(disk.TmScaleFrame, object.TmScaleFrame, sizeof(disk.TmScaleFrame));
	disk.TmFrameCnt = object.TmFrameCnt;
	return disk;
}

inline void ApplyLegacyObjectState(smOBJ3D &object, const LegacyObjectDiskLayout &disk)
{
	object.ZeroVertex = disk.ZeroVertex;
	object.maxZ = disk.maxZ;
	object.minZ = disk.minZ;
	object.maxY = disk.maxY;
	object.minY = disk.minY;
	object.maxX = disk.maxX;
	object.minX = disk.minX;
	object.dBound = disk.dBound;
	object.Bound = disk.Bound;
	object.MaxVertex = disk.MaxVertex;
	object.MaxFace = disk.MaxFace;
	object.nVertex = disk.nVertex;
	object.nFace = disk.nFace;
	object.nTexLink = disk.nTexLink;
	object.ColorEffect = disk.ColorEffect;
	object.ClipStates = disk.ClipStates;
	object.Posi = disk.Posi;
	object.CameraPosi = disk.CameraPosi;
	object.Angle = disk.Angle;
	std::memcpy(object.NodeName, disk.NodeName, sizeof(object.NodeName));
	std::memcpy(object.NodeParent, disk.NodeParent, sizeof(object.NodeParent));
	object.NodeName[sizeof(object.NodeName) - 1] = 0;
	object.NodeParent[sizeof(object.NodeParent) - 1] = 0;
	object.pParent = 0;
	object.Tm = disk.Tm;
	object.TmInvert = disk.TmInvert;
	object.TmResult = disk.TmResult;
	object.TmRotate = disk.TmRotate;
	object.mWorld = disk.mWorld;
	object.mLocal = disk.mLocal;
	object.lFrame = disk.lFrame;
	object.qx = disk.qx;
	object.qy = disk.qy;
	object.qz = disk.qz;
	object.qw = disk.qw;
	object.sx = disk.sx;
	object.sy = disk.sy;
	object.sz = disk.sz;
	object.px = disk.px;
	object.py = disk.py;
	object.pz = disk.pz;
	object.TmRotCnt = disk.TmRotCnt;
	object.TmPosCnt = disk.TmPosCnt;
	object.TmScaleCnt = disk.TmScaleCnt;
	std::memcpy(object.TmRotFrame, disk.TmRotFrame, sizeof(object.TmRotFrame));
	std::memcpy(object.TmPosFrame, disk.TmPosFrame, sizeof(object.TmPosFrame));
	std::memcpy(object.TmScaleFrame, disk.TmScaleFrame, sizeof(object.TmScaleFrame));
	object.TmFrameCnt = disk.TmFrameCnt;
}

} // namespace object_serialization
} // namespace legacy3d
} // namespace rendering
} // namespace summit
