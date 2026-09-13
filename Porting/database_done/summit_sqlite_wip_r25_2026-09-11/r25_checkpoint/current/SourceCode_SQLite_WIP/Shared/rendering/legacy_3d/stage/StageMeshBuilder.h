#pragma once

#include <cstdint>
#include <limits>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_mesh
{

static const int LegacyMaxTextureLinksPerFace = 9;

inline bool IsValidVertexIndex(const smSTAGE3D &stage, int index)
{
	return index >= 0 && index < stage.nVertex &&
		static_cast<unsigned long>(index) <= static_cast<unsigned long>(std::numeric_limits<WORD>::max());
}

inline bool IsValidMaterialIndex(int index)
{
	return index >= 0 &&
		static_cast<unsigned long>(index) <= static_cast<unsigned long>(std::numeric_limits<WORD>::max());
}

inline void ExpandMapBounds(smSTAGE3D &stage, int x, int z)
{
	if(stage.nVertex == 0)
	{
		stage.StageMapRect.top = z;
		stage.StageMapRect.bottom = z;
		stage.StageMapRect.left = x;
		stage.StageMapRect.right = x;
		return;
	}
	if(stage.StageMapRect.top > z) stage.StageMapRect.top = z;
	if(stage.StageMapRect.bottom < z) stage.StageMapRect.bottom = z;
	if(stage.StageMapRect.left > x) stage.StageMapRect.left = x;
	if(stage.StageMapRect.right < x) stage.StageMapRect.right = x;
}

inline int AddVertex(smSTAGE3D &stage, int x, int y, int z)
{
	if(stage.nVertex < 0 || stage.VertexCapacity < 0 || stage.nVertex >= stage.VertexCapacity || !stage.Vertex)
		return -1;

	smSTAGE_VERTEX &vertex = stage.Vertex[stage.nVertex];
	vertex.sum = 0;
	vertex.x = x;
	vertex.y = y;
	vertex.z = z;
	vertex.sDef_Color[SMC_R] = 255;
	vertex.sDef_Color[SMC_G] = 255;
	vertex.sDef_Color[SMC_B] = 255;
	vertex.sDef_Color[SMC_A] = 255;
	ExpandMapBounds(stage, x, z);
	return stage.nVertex++;
}

inline int AddFace(smSTAGE3D &stage, int a, int b, int c, int material)
{
	if(stage.nFace < 0 || stage.FaceCapacity < 0 || stage.nFace >= stage.FaceCapacity || !stage.Face ||
		!IsValidVertexIndex(stage, a) || !IsValidVertexIndex(stage, b) || !IsValidVertexIndex(stage, c) ||
		!IsValidMaterialIndex(material))
		return -1;

	smSTAGE_FACE &face = stage.Face[stage.nFace];
	face.sum = 0;
	face.CalcSum = 0;
	face.Vertex[0] = static_cast<WORD>(a);
	face.Vertex[1] = static_cast<WORD>(b);
	face.Vertex[2] = static_cast<WORD>(c);
	face.Vertex[3] = static_cast<WORD>(material);
	face.lpTexLink = 0;
	return stage.nFace++;
}

inline bool SetFaceMaterial(smSTAGE3D &stage, int faceIndex, int material)
{
	if(stage.nFace < 0 || stage.FaceCapacity < stage.nFace || faceIndex < 0 ||
		faceIndex >= stage.nFace || !stage.Face || !IsValidMaterialIndex(material))
		return false;
	stage.Face[faceIndex].Vertex[3] = static_cast<WORD>(material);
	return true;
}

inline bool SetVertexColor(smSTAGE3D &stage, DWORD vertexIndex, BYTE red, BYTE green, BYTE blue, BYTE alpha)
{
	if(stage.nVertex < 0 || stage.VertexCapacity < stage.nVertex || !stage.Vertex ||
		vertexIndex >= static_cast<DWORD>(stage.nVertex))
		return false;
	smSTAGE_VERTEX &vertex = stage.Vertex[vertexIndex];
	vertex.sDef_Color[SMC_R] = red;
	vertex.sDef_Color[SMC_G] = green;
	vertex.sDef_Color[SMC_B] = blue;
	vertex.sDef_Color[SMC_A] = alpha;
	return true;
}

inline bool IsOwnedTextureLink(const smSTAGE3D &stage, const smTEXLINK *link)
{
	if(!link || !stage.TexLink || stage.nTexLink <= 0)
		return false;
	const std::uintptr_t base = reinterpret_cast<std::uintptr_t>(stage.TexLink);
	const std::uintptr_t value = reinterpret_cast<std::uintptr_t>(link);
	const std::uintptr_t bytes = static_cast<std::uintptr_t>(stage.nTexLink) * sizeof(smTEXLINK);
	return value >= base && value < base + bytes && (value - base) % sizeof(smTEXLINK) == 0;
}

inline smTEXLINK **FindTextureLinkInsertionPoint(smSTAGE3D &stage, smSTAGE_FACE &face)
{
	if(!face.lpTexLink)
		return &face.lpTexLink;

	smTEXLINK *current = face.lpTexLink;
	for(int linkCount = 1; linkCount < LegacyMaxTextureLinksPerFace; ++linkCount)
	{
		if(!IsOwnedTextureLink(stage, current))
			return 0;
		if(!current->NextTex)
			return &current->NextTex;
		current = current->NextTex;
	}
	return 0;
}

inline int AddTextureLink(smSTAGE3D &stage, int faceIndex, DWORD *texture,
	const smFTPOINT *t1, const smFTPOINT *t2, const smFTPOINT *t3)
{
	if(faceIndex < 0 || faceIndex >= stage.nFace || !stage.Face || !t1 || !t2 || !t3 ||
		stage.nTexLink < 0 || stage.TexLinkCapacity < 0 ||
		stage.nTexLink >= stage.TexLinkCapacity || !stage.TexLink)
		return -1;

	smSTAGE_FACE &face = stage.Face[faceIndex];
	smTEXLINK **const insertionPoint = FindTextureLinkInsertionPoint(stage, face);
	if(!insertionPoint)
		return -1;

	smTEXLINK &link = stage.TexLink[stage.nTexLink];
	link.u[0] = t1->u; link.v[0] = t1->v;
	link.u[1] = t2->u; link.v[1] = t2->v;
	link.u[2] = t3->u; link.v[2] = t3->v;
	link.hTexture = texture;
	link.NextTex = 0;
	*insertionPoint = &link;
	return stage.nTexLink++;
}

} // namespace stage_mesh
} // namespace legacy3d
} // namespace rendering
} // namespace summit
