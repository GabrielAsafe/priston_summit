#pragma once

namespace
{

struct AseStageGeometryContext
{
	AseStageGeometryContext(smSTAGE3D *stage, AseImportContext &importContext)
		: import(importContext),
		  baseVertex(stage ? stage->nVertex : 0),
		  baseFace(stage ? stage->nFace : 0),
		  expectedVertexCount(-1),
		  expectedFaceCount(-1),
		  expectedTextureVertexCount(-1),
		  expectedColorVertexCount(-1),
		  expectedColorFaceCount(-1),
		  vertexCount(0),
		  faceCount(0),
		  textureVertexCount(0),
		  textureFaceCount(0),
		  colorVertexCount(0),
		  colorFaceCount(0),
		  materialRef(0),
		  blockDepth(0),
		  lightMapChannel(false),
		  textureVertices(0),
		  textureFaces(0)
	{
	}

	AseImportContext &import;

	int baseVertex;
	int baseFace;
	int expectedVertexCount;
	int expectedFaceCount;
	int expectedTextureVertexCount;
	int expectedColorVertexCount;
	int expectedColorFaceCount;
	int vertexCount;
	int faceCount;
	int textureVertexCount;
	int textureFaceCount;
	int colorVertexCount;
	int colorFaceCount;
	int materialRef;
	int blockDepth;
	bool lightMapChannel;

	std::unique_ptr<smFTPOINT[]> baseTextureVertices;
	std::unique_ptr<smTFACE[]> baseTextureFaces;
	std::unique_ptr<smFTPOINT[]> lightMapVertices;
	std::unique_ptr<smTFACE[]> lightMapFaces;
	std::unique_ptr<smFCPOINT[]> colorVertices;
	std::unique_ptr<smTFACE[]> colorFaces;
	std::unique_ptr<int[]> faceMaterials;

	smFTPOINT *textureVertices;
	smTFACE *textureFaces;
};

AseLineStatus ParseStageMeshSizeDeclaration(const char *tag, const char *payload,
	AseStageGeometryContext &context)
{
	summit::formats::ase::LineCursor cursor(payload);
	char token[64];

	if(summit::formats::ase::TagEquals(tag, "*MESH_NUMVERTEX"))
	{
		if(!cursor.NextToken(token, sizeof(token)) ||
			!summit::io::ParseNonNegativeInt(token, &context.expectedVertexCount))
			return AseLineStatus::Error;
		return AseLineStatus::Handled;
	}

	if(!summit::formats::ase::TagEquals(tag, "*MESH_NUMFACES"))
		return AseLineStatus::NotHandled;
	if(!cursor.NextToken(token, sizeof(token)) ||
		!summit::io::ParseNonNegativeInt(token, &context.expectedFaceCount))
		return AseLineStatus::Error;

	context.faceMaterials.reset(new int[context.expectedFaceCount]);
	if(context.expectedFaceCount > 0)
		ZeroMemory(context.faceMaterials.get(), sizeof(int) * context.expectedFaceCount);
	return AseLineStatus::Handled;
}

void SelectStageTextureChannel(AseStageGeometryContext &context)
{
	if(context.lightMapChannel)
	{
		context.lightMapVertices.reset(new smFTPOINT[context.expectedTextureVertexCount]);
		context.lightMapFaces.reset(new smTFACE[context.expectedFaceCount]);
		context.textureVertices = context.lightMapVertices.get();
		context.textureFaces = context.lightMapFaces.get();
	}
	else
	{
		context.baseTextureVertices.reset(new smFTPOINT[context.expectedTextureVertexCount]);
		context.baseTextureFaces.reset(new smTFACE[context.expectedFaceCount]);
		context.textureVertices = context.baseTextureVertices.get();
		context.textureFaces = context.baseTextureFaces.get();
	}
}

AseLineStatus ParseStageTextureDeclaration(const char *tag, const char *payload,
	AseStageGeometryContext &context)
{
	if(summit::formats::ase::TagEquals(tag, "*MESH_MAPPINGCHANNEL"))
	{
		context.lightMapChannel = true;
		return AseLineStatus::Handled;
	}
	if(!summit::formats::ase::TagEquals(tag, "*MESH_NUMTVERTEX"))
		return AseLineStatus::NotHandled;

	summit::formats::ase::LineCursor cursor(payload);
	char token[64];
	if(!cursor.NextToken(token, sizeof(token)) ||
		!summit::io::ParseNonNegativeInt(token, &context.expectedTextureVertexCount) ||
		context.expectedFaceCount < 0)
		return AseLineStatus::Error;

	SelectStageTextureChannel(context);
	context.textureVertexCount = 0;
	context.textureFaceCount = 0;
	if(context.expectedTextureVertexCount > 0)
		ZeroMemory(context.textureVertices, sizeof(smFTPOINT) * context.expectedTextureVertexCount);
	if(context.expectedFaceCount > 0)
		ZeroMemory(context.textureFaces, sizeof(smTFACE) * context.expectedFaceCount);
	return AseLineStatus::Handled;
}

AseLineStatus ParseStageMeshDeclaration(const char *tag, const char *payload, AseStageGeometryContext &context)
{
	AseLineStatus result = ParseStageMeshSizeDeclaration(tag, payload, context);
	if(result != AseLineStatus::NotHandled)
		return result;
	return ParseStageTextureDeclaration(tag, payload, context);
}

AseLineStatus ParseStageVertex(const char *tag, const char *payload, AseStageGeometryContext &context, smSTAGE3D *stage)
{
	if(!summit::formats::ase::TagEquals(tag, "*MESH_VERTEX"))
		return AseLineStatus::NotHandled;
	if(context.expectedVertexCount < 0 || context.vertexCount >= context.expectedVertexCount)
		return AseLineStatus::Error;

	summit::formats::ase::VertexRecord vertex;
	if(!summit::formats::ase::ParseVertexRecord(payload, &vertex))
		return AseLineStatus::Error;

	// Stage coordinates historically swap ASE Y/Z. Preserve that conversion here.
	const int addedVertex = stage->AddVertex(static_cast<int>(vertex.x * fONE),
		static_cast<int>(vertex.z * fONE), static_cast<int>(vertex.y * fONE));
	if(addedVertex != context.baseVertex + context.vertexCount)
		return AseLineStatus::Error;
	++context.vertexCount;
	return AseLineStatus::Handled;
}

AseLineStatus ParseStageFace(const char *tag, const char *payload, AseStageGeometryContext &context, smSTAGE3D *stage)
{
	if(!summit::formats::ase::TagEquals(tag, "*MESH_FACE"))
		return AseLineStatus::NotHandled;
	if(context.expectedFaceCount < 0 || !context.faceMaterials || context.faceCount >= context.expectedFaceCount)
		return AseLineStatus::Error;

	summit::formats::ase::FaceRecord face;
	if(!summit::formats::ase::ParseFaceRecord(payload, &face))
		return AseLineStatus::Error;
	if(!summit::io::IndexInRange(face.a, context.expectedVertexCount) ||
		!summit::io::IndexInRange(face.b, context.expectedVertexCount) ||
		!summit::io::IndexInRange(face.c, context.expectedVertexCount))
		return AseLineStatus::Error;

	if(face.hasMaterialId)
		context.faceMaterials[context.faceCount] = face.materialId;
	const int addedFace = stage->AddFace(face.a + context.baseVertex, face.b + context.baseVertex,
		face.c + context.baseVertex);
	if(addedFace != context.baseFace + context.faceCount)
		return AseLineStatus::Error;
	++context.faceCount;
	return AseLineStatus::Handled;
}

AseLineStatus ParseStageTextureVertex(const char *tag, const char *payload, AseStageGeometryContext &context)
{
	if(!summit::formats::ase::TagEquals(tag, "*MESH_TVERT"))
		return AseLineStatus::NotHandled;
	if(!context.textureVertices || context.textureVertexCount >= context.expectedTextureVertexCount)
		return AseLineStatus::Error;

	summit::formats::ase::TextureVertexRecord vertex;
	if(!summit::formats::ase::ParseTextureVertexRecord(payload, &vertex))
		return AseLineStatus::Error;

	context.textureVertices[context.textureVertexCount].u = vertex.u;
	context.textureVertices[context.textureVertexCount].v = -vertex.v;
	++context.textureVertexCount;
	return AseLineStatus::Handled;
}

AseLineStatus ParseStageTextureFace(const char *tag, const char *payload, AseStageGeometryContext &context)
{
	if(!summit::formats::ase::TagEquals(tag, "*MESH_TFACE"))
		return AseLineStatus::NotHandled;
	if(!context.textureFaces || context.textureFaceCount >= context.expectedFaceCount)
		return AseLineStatus::Error;

	summit::formats::ase::TriangleIndexRecord face;
	if(!summit::formats::ase::ParseTriangleIndexRecord(payload, &face))
		return AseLineStatus::Error;
	if(!summit::io::IndexInRange(face.a, context.expectedTextureVertexCount) ||
		!summit::io::IndexInRange(face.b, context.expectedTextureVertexCount) ||
		!summit::io::IndexInRange(face.c, context.expectedTextureVertexCount))
		return AseLineStatus::Error;

	context.textureFaces[context.textureFaceCount].t[0] = face.a;
	context.textureFaces[context.textureFaceCount].t[1] = face.b;
	context.textureFaces[context.textureFaceCount].t[2] = face.c;
	++context.textureFaceCount;
	return AseLineStatus::Handled;
}

AseLineStatus ParseStageMaterialReference(const char *tag, const char *payload, AseStageGeometryContext &context)
{
	if(!summit::formats::ase::TagEquals(tag, "*MATERIAL_REF"))
		return AseLineStatus::NotHandled;

	summit::formats::ase::LineCursor cursor(payload);
	char token[64];
	if(!cursor.NextToken(token, sizeof(token)) || !summit::io::ParseNonNegativeInt(token, &context.materialRef))
		return AseLineStatus::Error;
	return AseLineStatus::Handled;
}

AseLineStatus ParseStageMeshData(const char *tag, const char *payload, AseStageGeometryContext &context, smSTAGE3D *stage)
{
	AseLineStatus result = ParseStageVertex(tag, payload, context, stage);
	if(result != AseLineStatus::NotHandled) return result;
	result = ParseStageFace(tag, payload, context, stage);
	if(result != AseLineStatus::NotHandled) return result;
	result = ParseStageTextureVertex(tag, payload, context);
	if(result != AseLineStatus::NotHandled) return result;
	result = ParseStageTextureFace(tag, payload, context);
	if(result != AseLineStatus::NotHandled) return result;
	return ParseStageMaterialReference(tag, payload, context);
}

AseLineStatus ParseStageColorDeclaration(const char *tag, const char *payload, AseStageGeometryContext &context)
{
	summit::formats::ase::LineCursor cursor(payload);
	char token[64];

	if(summit::formats::ase::TagEquals(tag, "*MESH_NUMCVERTEX"))
	{
		if(!cursor.NextToken(token, sizeof(token)) ||
			!summit::io::ParseNonNegativeInt(token, &context.expectedColorVertexCount))
			return AseLineStatus::Error;
		context.colorVertices.reset(new smFCPOINT[context.expectedColorVertexCount]);
		return AseLineStatus::Handled;
	}

	if(!summit::formats::ase::TagEquals(tag, "*MESH_NUMCVFACES"))
		return AseLineStatus::NotHandled;
	if(!cursor.NextToken(token, sizeof(token)) ||
		!summit::io::ParseNonNegativeInt(token, &context.expectedColorFaceCount))
		return AseLineStatus::Error;
	context.colorFaces.reset(new smTFACE[context.expectedColorFaceCount]);
	return AseLineStatus::Handled;
}

AseLineStatus ParseStageVertexColor(const char *tag, const char *payload, AseStageGeometryContext &context)
{
	if(!summit::formats::ase::TagEquals(tag, "*MESH_VERTCOL"))
		return AseLineStatus::NotHandled;
	if(!context.colorVertices || context.colorVertexCount >= context.expectedColorVertexCount)
		return AseLineStatus::Error;

	summit::formats::ase::ColorRecord color;
	if(!summit::formats::ase::ParseColorRecord(payload, &color))
		return AseLineStatus::Error;

	context.colorVertices[context.colorVertexCount].r = color.r;
	context.colorVertices[context.colorVertexCount].g = color.g;
	context.colorVertices[context.colorVertexCount].b = color.b;
	++context.colorVertexCount;
	return AseLineStatus::Handled;
}

AseLineStatus ParseStageColorFace(const char *tag, const char *payload, AseStageGeometryContext &context)
{
	if(!summit::formats::ase::TagEquals(tag, "*MESH_CFACE"))
		return AseLineStatus::NotHandled;
	if(!context.colorFaces || context.colorFaceCount >= context.expectedColorFaceCount)
		return AseLineStatus::Error;

	summit::formats::ase::TriangleIndexRecord face;
	if(!summit::formats::ase::ParseTriangleIndexRecord(payload, &face))
		return AseLineStatus::Error;
	if(!summit::io::IndexInRange(face.a, context.expectedColorVertexCount) ||
		!summit::io::IndexInRange(face.b, context.expectedColorVertexCount) ||
		!summit::io::IndexInRange(face.c, context.expectedColorVertexCount))
		return AseLineStatus::Error;

	context.colorFaces[context.colorFaceCount].t[0] = face.a;
	context.colorFaces[context.colorFaceCount].t[1] = face.b;
	context.colorFaces[context.colorFaceCount].t[2] = face.c;
	++context.colorFaceCount;
	return AseLineStatus::Handled;
}

AseLineStatus ParseStageColorData(const char *tag, const char *payload, AseStageGeometryContext &context)
{
	AseLineStatus result = ParseStageColorDeclaration(tag, payload, context);
	if(result != AseLineStatus::NotHandled) return result;
	result = ParseStageVertexColor(tag, payload, context);
	if(result != AseLineStatus::NotHandled) return result;
	return ParseStageColorFace(tag, payload, context);
}

bool AddStageTextureLink(smSTAGE3D *stage, const AseStageGeometryContext &context,
	int faceIndex, int materialIndex, int registeredMaterial, int textureIndex,
	const smFTPOINT *textureVertices, const smTFACE *textureFaces)
{
	smFTPOINT transformed[3];
	if(!TransformTextureFace(context.import, materialIndex, textureIndex, faceIndex,
		textureVertices, textureFaces, false, transformed))
		return false;

	return stage->AddTexLink(faceIndex + context.baseFace,
		(DWORD *)&stage->smMaterial[registeredMaterial].smTexture[textureIndex],
		&transformed[0], &transformed[1], &transformed[2]) >= 0;
}

bool BuildUntexturedStageFaces(smSTAGE3D *stage, const AseStageGeometryContext &context)
{
	smFTPOINT emptyTexturePoint;
	emptyTexturePoint.u = 0.0f;
	emptyTexturePoint.v = 0.0f;
	for(int face = 0; face < context.expectedFaceCount; ++face)
	{
		if(!stage->SetFaceMaterial(face + context.baseFace, 0) ||
			stage->AddTexLink(face + context.baseFace, 0,
				&emptyTexturePoint, &emptyTexturePoint, &emptyTexturePoint) < 0)
			return false;
	}
	return true;
}

bool ResolveStageMaterialIndex(const AseStageGeometryContext &context, int face, int *materialIndex)
{
	return context.faceMaterials &&
		ResolveAseFaceMaterial(context.import, context.materialRef, context.faceMaterials[face], materialIndex);
}

bool ResolveRegisteredStageMaterial(smSTAGE3D *stage, const AseImportContext &import, int materialIndex,
	int stageMaterialCount, int *registeredMaterial, int *textureCount)
{
	if(!stage || !registeredMaterial || !textureCount ||
		!import.HasMaterial(materialIndex))
		return false;

	*registeredMaterial = import.Material(materialIndex).RegistNum;
	if(!summit::io::IndexInRange(*registeredMaterial, stageMaterialCount))
		return false;

	*textureCount = stage->smMaterial[*registeredMaterial].TextureCounter;
	return *textureCount >= 0 && *textureCount <= ASE_TEXTURE_SLOT_MAX;
}

bool BuildStageFaceTextureLinks(smSTAGE3D *stage, const AseStageGeometryContext &context,
	int face, int materialIndex, int registeredMaterial, int textureCount)
{
	if(textureCount > 0 && (!context.baseTextureVertices || !context.baseTextureFaces))
		return false;

	int regularTextureCount = textureCount;
	if(context.lightMapChannel)
	{
		if(textureCount <= 0 || !context.lightMapVertices || !context.lightMapFaces)
			return false;
		--regularTextureCount;
	}

	for(int texture = 0; texture < regularTextureCount; ++texture)
	{
		if(!AddStageTextureLink(stage, context, face, materialIndex, registeredMaterial, texture,
			context.baseTextureVertices.get(), context.baseTextureFaces.get()))
			return false;
	}

	if(context.lightMapChannel &&
		!AddStageTextureLink(stage, context, face, materialIndex, registeredMaterial, regularTextureCount,
			context.lightMapVertices.get(), context.lightMapFaces.get()))
		return false;

	return true;
}

bool BuildStageMaterials(smSTAGE3D *stage, const AseStageGeometryContext &context)
{
	if(context.expectedFaceCount < 0)
		return false;
	if(context.expectedTextureVertexCount <= 0)
		return BuildUntexturedStageFaces(stage, context);
	if(!stage->smMaterialGroup || !stage->smMaterial || !context.faceMaterials)
		return false;

	const int stageMaterialCount = static_cast<int>(stage->smMaterialGroup->MaterialCount);
	if(stageMaterialCount <= 0)
		return false;

	for(int face = 0; face < context.expectedFaceCount; ++face)
	{
		int materialIndex = 0;
		int registeredMaterial = 0;
		int textureCount = 0;
		if(!ResolveStageMaterialIndex(context, face, &materialIndex) ||
			!ResolveRegisteredStageMaterial(stage, context.import, materialIndex, stageMaterialCount,
				&registeredMaterial, &textureCount) ||
			!BuildStageFaceTextureLinks(stage, context, face, materialIndex, registeredMaterial, textureCount))
			return false;

		if(!stage->SetFaceMaterial(face + context.baseFace, registeredMaterial))
			return false;
		stage->smMaterial[registeredMaterial].InUse++;
	}
	return true;
}

void InitializeStageVertexColors(std::vector<smFCPOINT> &colors, std::vector<int> &sampleCounts)
{
	for(std::size_t vertex = 0; vertex < colors.size(); ++vertex)
	{
		colors[vertex].r = 1.0f;
		colors[vertex].g = 1.0f;
		colors[vertex].b = 1.0f;
		sampleCounts[vertex] = 1;
	}
}

bool AccumulateStageVertexColors(smSTAGE3D *stage, const AseStageGeometryContext &context,
	std::vector<smFCPOINT> &colors, std::vector<int> &sampleCounts)
{
	for(int face = 0; face < context.expectedFaceCount; ++face)
	{
		for(int corner = 0; corner < 3; ++corner)
		{
			const int vertexIndex = stage->Face[face + context.baseFace].Vertex[corner] - context.baseVertex;
			if(!summit::io::IndexInRange(vertexIndex, context.expectedVertexCount))
				return false;

			const DWORD colorIndex = context.colorFaces[face].t[corner];
			if(!summit::io::IndexInRange(static_cast<int>(colorIndex), context.expectedColorVertexCount))
				return false;

			colors[vertexIndex].r += context.colorVertices[colorIndex].r;
			colors[vertexIndex].g += context.colorVertices[colorIndex].g;
			colors[vertexIndex].b += context.colorVertices[colorIndex].b;
			++sampleCounts[vertexIndex];
		}
	}
	return true;
}

void WriteStageVertexColors(smSTAGE3D *stage, const AseStageGeometryContext &context,
	const std::vector<smFCPOINT> &colors, const std::vector<int> &sampleCounts)
{
	for(int vertex = 0; vertex < context.expectedVertexCount; ++vertex)
	{
		const int sampleCount = sampleCounts[vertex];
		if(sampleCount <= 0)
			continue;

		const int red = static_cast<BYTE>((colors[vertex].r / sampleCount) * 255.0f);
		const int green = static_cast<BYTE>((colors[vertex].g / sampleCount) * 255.0f);
		const int blue = static_cast<BYTE>((colors[vertex].b / sampleCount) * 255.0f);
		stage->SetVertexColor(context.baseVertex + vertex, red, green, blue);
	}
}

bool ApplyStageVertexColors(smSTAGE3D *stage, const AseStageGeometryContext &context)
{
	if(context.colorFaceCount == 0)
		return true;
	if(context.expectedVertexCount < 0 || context.expectedFaceCount < 0 ||
		context.colorFaceCount < context.expectedFaceCount || !context.colorVertices || !context.colorFaces)
		return false;

	std::vector<smFCPOINT> colors(context.expectedVertexCount);
	std::vector<int> sampleCounts(context.expectedVertexCount);
	InitializeStageVertexColors(colors, sampleCounts);
	if(!AccumulateStageVertexColors(stage, context, colors, sampleCounts))
		return false;

	WriteStageVertexColors(stage, context, colors, sampleCounts);
	return true;
}

bool ValidateStageGeometryCompleteness(const AseStageGeometryContext &context)
{
	if(context.expectedVertexCount < 0 || context.expectedFaceCount < 0)
		return false;
	if(context.vertexCount != context.expectedVertexCount || context.faceCount != context.expectedFaceCount)
		return false;
	return true;
}

} // anonymous namespace


FILE *smSTAGE3D_ReadASE_GEOMOBJECT(FILE *fp, AseImportContext &import, smSTAGE3D *smStage)
{
	if(!fp || !smStage)
		return FALSE;

	AseStageGeometryContext context(smStage, import);
	char inputLine[256];
	bool objectFinished = false;

	while(fgets(inputLine, sizeof(inputLine), fp) != NULL)
	{
		summit::formats::ase::LineCursor lineCursor(inputLine);
		char tag[64];
		if(!lineCursor.NextToken(tag, sizeof(tag)))
			continue;

		const char *payload = lineCursor.Position();
		AseLineStatus status = ParseStageMeshDeclaration(tag, payload, context);
		if(status == AseLineStatus::NotHandled)
			status = ParseStageMeshData(tag, payload, context, smStage);
		if(status == AseLineStatus::NotHandled)
			status = ParseStageColorData(tag, payload, context);

		if(status == AseLineStatus::Error)
			return FALSE;

		const AseLineStatus blockStatus = UpdateBlockDepth(tag, payload, context.blockDepth);
		if(blockStatus == AseLineStatus::FinishObject)
		{
			objectFinished = true;
			break;
		}
	}

	if(!objectFinished || !ValidateStageGeometryCompleteness(context))
		return FALSE;
	if(!BuildStageMaterials(smStage, context))
		return FALSE;
	if(!ApplyStageVertexColors(smStage, context))
		return FALSE;

	return fp;
}

namespace
{
