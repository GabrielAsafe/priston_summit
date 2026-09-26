#pragma once

thread_local smPAT3D *smPhysique = 0;

#define SCENE_TICKSPERFRAME		160
static const int ASE_TM_SAMPLE_MAX = 4096;
namespace
{

enum class AseLineStatus
{
	NotHandled,
	Handled,
	Error,
	FinishObject,
	SkipObject
};

struct AseGeometryContext
{
	explicit AseGeometryContext(AseImportContext &importContext)
		: import(importContext),
		  expectedVertexCount(-1),
		  expectedFaceCount(-1),
		  expectedTextureVertexCount(-1),
		  vertexCount(0),
		  faceCount(0),
		  textureVertexCount(0),
		  textureFaceCount(0),
		  materialRef(0),
		  currentPhysiqueVertex(0),
		  blockDepth(0),
		  meshInitialized(false),
		  lightMapChannel(false),
		  textureVertices(0),
		  textureFaces(0),
		  rotationCount(0),
		  positionCount(0),
		  scaleCount(0)
	{
	}

	AseImportContext &import;

	int expectedVertexCount;
	int expectedFaceCount;
	int expectedTextureVertexCount;

	int vertexCount;
	int faceCount;
	int textureVertexCount;
	int textureFaceCount;
	int materialRef;
	int currentPhysiqueVertex;
	int blockDepth;
	bool meshInitialized;
	bool lightMapChannel;

	std::unique_ptr<smFTPOINT[]> baseTextureVertices;
	std::unique_ptr<smTFACE[]> baseTextureFaces;
	std::unique_ptr<smFTPOINT[]> lightMapVertices;
	std::unique_ptr<smTFACE[]> lightMapFaces;
	std::unique_ptr<int[]> faceMaterials;

	smFTPOINT *textureVertices;
	smTFACE *textureFaces;

	smTM_ROT rotations[ASE_TM_SAMPLE_MAX];
	smDTM_ROT deltaRotations[ASE_TM_SAMPLE_MAX];
	smTM_POS positions[ASE_TM_SAMPLE_MAX];
	smTM_SCALE scales[ASE_TM_SAMPLE_MAX];
	int rotationCount;
	int positionCount;
	int scaleCount;
};

bool ReadFixedVector3(summit::formats::ase::LineCursor &cursor, int &x, int &y, int &z)
{
	float fx = 0.0f;
	float fy = 0.0f;
	float fz = 0.0f;
	if(!cursor.NextFloat(&fx) || !cursor.NextFloat(&fy) || !cursor.NextFloat(&fz))
		return false;

	x = static_cast<int>(fx * fONE);
	y = static_cast<int>(fy * fONE);
	z = static_cast<int>(fz * fONE);
	return true;
}

bool ReadFloatVector3(summit::formats::ase::LineCursor &cursor, float &x, float &y, float &z)
{
	return cursor.NextFloat(&x) && cursor.NextFloat(&y) && cursor.NextFloat(&z);
}

bool EnsureMeshStorage(AseGeometryContext &context, smOBJ3D *object)
{
	if(context.meshInitialized || context.expectedVertexCount < 0 || context.expectedFaceCount < 0)
		return true;

	object->Init(context.expectedVertexCount, context.expectedFaceCount, context.import.physique ? 1 : 0);
	context.faceMaterials.reset(new int[context.expectedFaceCount]);
	if(context.expectedFaceCount > 0)
		ZeroMemory(context.faceMaterials.get(), sizeof(int) * context.expectedFaceCount);

	context.meshInitialized = true;
	return true;
}

AseLineStatus ParseObjectMetadata(const char *tag, const char *payload, AseGeometryContext &, smOBJ3D *object, const char *requestedNodeName)
{
	if(summit::formats::ase::TagEquals(tag, "*NODE_NAME"))
	{
		char nodeName[256];
		summit::formats::ase::LineCursor cursor(payload);
		if(!cursor.NextQuoted(nodeName, sizeof(nodeName)))
			return AseLineStatus::Error;

		nodeName[31] = '\0';
		strcpy_s(object->NodeName, nodeName);
		if(requestedNodeName && lstrcmpi(nodeName, requestedNodeName) != 0)
		{
			object->nFace = 0;
			object->nVertex = 0;
			return AseLineStatus::SkipObject;
		}
		return AseLineStatus::Handled;
	}

	if(summit::formats::ase::TagEquals(tag, "*NODE_PARENT"))
	{
		char parentName[256];
		summit::formats::ase::LineCursor cursor(payload);
		if(!cursor.NextQuoted(parentName, sizeof(parentName)))
			return AseLineStatus::Error;

		parentName[31] = '\0';
		strcpy_s(object->NodeParent, parentName);
		return AseLineStatus::Handled;
	}

	return AseLineStatus::NotHandled;
}

AseLineStatus ParseMeshSizeDeclaration(const char *tag, const char *payload,
	AseGeometryContext &context, smOBJ3D *object)
{
	int *target = 0;
	if(summit::formats::ase::TagEquals(tag, "*MESH_NUMVERTEX"))
		target = &context.expectedVertexCount;
	else if(summit::formats::ase::TagEquals(tag, "*MESH_NUMFACES"))
		target = &context.expectedFaceCount;
	else
		return AseLineStatus::NotHandled;

	summit::formats::ase::LineCursor cursor(payload);
	char token[64];
	if(!cursor.NextToken(token, sizeof(token)) || !summit::io::ParseNonNegativeInt(token, target))
		return AseLineStatus::Error;
	return EnsureMeshStorage(context, object) ? AseLineStatus::Handled : AseLineStatus::Error;
}

void SelectObjectTextureChannel(AseGeometryContext &context)
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

AseLineStatus ParseTextureMeshDeclaration(const char *tag, const char *payload, AseGeometryContext &context)
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

	SelectObjectTextureChannel(context);
	context.textureVertexCount = 0;
	context.textureFaceCount = 0;
	if(context.expectedTextureVertexCount > 0)
		ZeroMemory(context.textureVertices, sizeof(smFTPOINT) * context.expectedTextureVertexCount);
	if(context.expectedFaceCount > 0)
		ZeroMemory(context.textureFaces, sizeof(smTFACE) * context.expectedFaceCount);
	return AseLineStatus::Handled;
}

AseLineStatus ParseMeshDeclaration(const char *tag, const char *payload, AseGeometryContext &context, smOBJ3D *object)
{
	AseLineStatus result = ParseMeshSizeDeclaration(tag, payload, context, object);
	if(result != AseLineStatus::NotHandled)
		return result;
	return ParseTextureMeshDeclaration(tag, payload, context);
}

AseLineStatus ParseTransformMatrixRow(const char *tag, const char *payload, smOBJ3D *object)
{
	int row = -1;
	if(summit::formats::ase::TagEquals(tag, "*TM_ROW0")) row = 0;
	else if(summit::formats::ase::TagEquals(tag, "*TM_ROW1")) row = 1;
	else if(summit::formats::ase::TagEquals(tag, "*TM_ROW2")) row = 2;
	else if(summit::formats::ase::TagEquals(tag, "*TM_ROW3")) row = 3;
	else return AseLineStatus::NotHandled;

	summit::formats::ase::LineCursor cursor(payload);
	int x = 0;
	int y = 0;
	int z = 0;
	if(!ReadFixedVector3(cursor, x, y, z))
		return AseLineStatus::Error;

	switch(row)
	{
	case 0: object->Tm._11 = x; object->Tm._12 = y; object->Tm._13 = z; break;
	case 1: object->Tm._21 = x; object->Tm._22 = y; object->Tm._23 = z; break;
	case 2: object->Tm._31 = x; object->Tm._32 = y; object->Tm._33 = z; break;
	case 3: object->Tm._41 = x; object->Tm._42 = y; object->Tm._43 = z; break;
	}
	return AseLineStatus::Handled;
}

AseLineStatus ParseTransformPosition(const char *tag, const char *payload, smOBJ3D *object)
{
	if(!summit::formats::ase::TagEquals(tag, "*TM_POS"))
		return AseLineStatus::NotHandled;

	summit::formats::ase::LineCursor cursor(payload);
	if(!ReadFixedVector3(cursor, object->px, object->py, object->pz))
		return AseLineStatus::Error;
	return AseLineStatus::Handled;
}

AseLineStatus ParseTransformRotation(const char *tag, const char *payload, smOBJ3D *object)
{
	summit::formats::ase::LineCursor cursor(payload);
	if(summit::formats::ase::TagEquals(tag, "*TM_ROTAXIS"))
	{
		if(!ReadFloatVector3(cursor, object->qx, object->qy, object->qz))
			return AseLineStatus::Error;
		return AseLineStatus::Handled;
	}

	if(!summit::formats::ase::TagEquals(tag, "*TM_ROTANGLE"))
		return AseLineStatus::NotHandled;
	if(!cursor.NextFloat(&object->qw))
		return AseLineStatus::Error;

	smQuaternionFromAxis(object->qx, object->qy, object->qz, object->qw);
	smMatrixFromQuaternion(object->TmRotate, object->qx, object->qy, object->qz, object->qw);
	return AseLineStatus::Handled;
}

AseLineStatus ParseTransformScale(const char *tag, const char *payload, smOBJ3D *object)
{
	if(!summit::formats::ase::TagEquals(tag, "*TM_SCALE"))
		return AseLineStatus::NotHandled;

	summit::formats::ase::LineCursor cursor(payload);
	if(!ReadFixedVector3(cursor, object->sx, object->sy, object->sz))
		return AseLineStatus::Error;
	return AseLineStatus::Handled;
}

AseLineStatus ParseTransform(const char *tag, const char *payload, smOBJ3D *object)
{
	AseLineStatus result = ParseTransformMatrixRow(tag, payload, object);
	if(result != AseLineStatus::NotHandled) return result;
	result = ParseTransformPosition(tag, payload, object);
	if(result != AseLineStatus::NotHandled) return result;
	result = ParseTransformRotation(tag, payload, object);
	if(result != AseLineStatus::NotHandled) return result;
	return ParseTransformScale(tag, payload, object);
}

bool IsRotationSampleTag(const char *tag)
{
	return summit::formats::ase::TagEquals(tag, "*CONTROL_ROT_SAMPLE") ||
		summit::formats::ase::TagEquals(tag, "*CONTROL_TCB_ROT_KEY");
}

bool IsPositionSampleTag(const char *tag)
{
	return summit::formats::ase::TagEquals(tag, "*CONTROL_POS_SAMPLE") ||
		summit::formats::ase::TagEquals(tag, "*CONTROL_TCB_POS_KEY") ||
		summit::formats::ase::TagEquals(tag, "*CONTROL_BEZIER_POS_KEY");
}

bool IsScaleSampleTag(const char *tag)
{
	return summit::formats::ase::TagEquals(tag, "*CONTROL_SCALE_SAMPLE") ||
		summit::formats::ase::TagEquals(tag, "*CONTROL_TCB_SCALE_KEY") ||
		summit::formats::ase::TagEquals(tag, "*CONTROL_BEZIER_SCALE_KEY");
}

AseLineStatus ParseRotationSample(const char *tag, const char *payload, AseGeometryContext &context)
{
	if(!IsRotationSampleTag(tag))
		return AseLineStatus::NotHandled;
	if(context.rotationCount >= ASE_TM_SAMPLE_MAX)
		return AseLineStatus::Error;

	summit::formats::ase::LineCursor cursor(payload);
	smDTM_ROT &delta = context.deltaRotations[context.rotationCount];
	smTM_ROT &rotation = context.rotations[context.rotationCount];
	if(!cursor.NextInt(&delta.frame) ||
		!cursor.NextDouble(&delta.x) || !cursor.NextDouble(&delta.y) ||
		!cursor.NextDouble(&delta.z) || !cursor.NextDouble(&delta.w))
		return AseLineStatus::Error;

	rotation.frame = delta.frame;
	rotation.x = static_cast<float>(delta.x);
	rotation.y = static_cast<float>(delta.y);
	rotation.z = static_cast<float>(delta.z);
	rotation.w = static_cast<float>(delta.w);
	++context.rotationCount;
	return AseLineStatus::Handled;
}

AseLineStatus ParsePositionSample(const char *tag, const char *payload, AseGeometryContext &context)
{
	if(!IsPositionSampleTag(tag))
		return AseLineStatus::NotHandled;
	if(context.positionCount >= ASE_TM_SAMPLE_MAX)
		return AseLineStatus::Error;

	summit::formats::ase::LineCursor cursor(payload);
	smTM_POS &position = context.positions[context.positionCount];
	if(!cursor.NextInt(&position.frame) ||
		!cursor.NextFloat(&position.x) || !cursor.NextFloat(&position.y) || !cursor.NextFloat(&position.z))
		return AseLineStatus::Error;

	++context.positionCount;
	return AseLineStatus::Handled;
}

AseLineStatus ParseScaleSample(const char *tag, const char *payload, AseGeometryContext &context)
{
	if(!IsScaleSampleTag(tag))
		return AseLineStatus::NotHandled;
	if(context.scaleCount >= ASE_TM_SAMPLE_MAX)
		return AseLineStatus::Error;

	summit::formats::ase::LineCursor cursor(payload);
	smTM_SCALE &scale = context.scales[context.scaleCount];
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	if(!cursor.NextInt(&scale.frame) || !ReadFloatVector3(cursor, x, y, z))
		return AseLineStatus::Error;

	scale.x = static_cast<int>(x * fONE);
	scale.y = static_cast<int>(y * fONE);
	scale.z = static_cast<int>(z * fONE);
	++context.scaleCount;
	return AseLineStatus::Handled;
}

AseLineStatus ParseAnimationSample(const char *tag, const char *payload, AseGeometryContext &context)
{
	AseLineStatus result = ParseRotationSample(tag, payload, context);
	if(result != AseLineStatus::NotHandled) return result;
	result = ParsePositionSample(tag, payload, context);
	if(result != AseLineStatus::NotHandled) return result;
	return ParseScaleSample(tag, payload, context);
}

AseLineStatus ParseMeshVertex(const char *tag, const char *payload, AseGeometryContext &context, smOBJ3D *object)
{
	if(!summit::formats::ase::TagEquals(tag, "*MESH_VERTEX"))
		return AseLineStatus::NotHandled;
	if(!context.meshInitialized || context.vertexCount >= context.expectedVertexCount)
		return AseLineStatus::Error;

	summit::formats::ase::VertexRecord vertex;
	if(!summit::formats::ase::ParseVertexRecord(payload, &vertex))
		return AseLineStatus::Error;

	object->AddVertex(static_cast<int>(vertex.x * fONE),
		static_cast<int>(vertex.y * fONE), static_cast<int>(vertex.z * fONE));
	++context.vertexCount;
	return AseLineStatus::Handled;
}

AseLineStatus ParseMeshFace(const char *tag, const char *payload, AseGeometryContext &context, smOBJ3D *object)
{
	if(!summit::formats::ase::TagEquals(tag, "*MESH_FACE"))
		return AseLineStatus::NotHandled;
	if(!context.meshInitialized || context.faceCount >= context.expectedFaceCount || !context.faceMaterials)
		return AseLineStatus::Error;

	summit::formats::ase::FaceRecord face;
	if(!summit::formats::ase::ParseFaceRecord(payload, &face))
		return AseLineStatus::Error;
	if(!summit::io::IndexInRange(face.a, context.expectedVertexCount) ||
		!summit::io::IndexInRange(face.b, context.expectedVertexCount) ||
		!summit::io::IndexInRange(face.c, context.expectedVertexCount))
		return AseLineStatus::Error;

	object->AddFace(face.a, face.b, face.c);
	if(face.hasMaterialId)
		context.faceMaterials[context.faceCount] = face.materialId;
	++context.faceCount;
	return AseLineStatus::Handled;
}

AseLineStatus ParseMeshTextureVertex(const char *tag, const char *payload, AseGeometryContext &context)
{
	if(!summit::formats::ase::TagEquals(tag, "*MESH_TVERT"))
		return AseLineStatus::NotHandled;
	if(!context.textureVertices || context.textureVertexCount >= context.expectedTextureVertexCount)
		return AseLineStatus::Error;

	summit::formats::ase::TextureVertexRecord vertex;
	if(!summit::formats::ase::ParseTextureVertexRecord(payload, &vertex))
		return AseLineStatus::Error;

	context.textureVertices[context.textureVertexCount].u = vertex.u;
	context.textureVertices[context.textureVertexCount].v = vertex.v;
	++context.textureVertexCount;
	return AseLineStatus::Handled;
}

AseLineStatus ParseMeshTextureFace(const char *tag, const char *payload, AseGeometryContext &context)
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

AseLineStatus ParseMeshMaterialReference(const char *tag, const char *payload, AseGeometryContext &context)
{
	if(!summit::formats::ase::TagEquals(tag, "*MATERIAL_REF"))
		return AseLineStatus::NotHandled;

	summit::formats::ase::LineCursor cursor(payload);
	char token[64];
	if(!cursor.NextToken(token, sizeof(token)) || !summit::io::ParseNonNegativeInt(token, &context.materialRef))
		return AseLineStatus::Error;
	return AseLineStatus::Handled;
}

AseLineStatus ParseMeshData(const char *tag, const char *payload, AseGeometryContext &context, smOBJ3D *object)
{
	AseLineStatus result = ParseMeshVertex(tag, payload, context, object);
	if(result != AseLineStatus::NotHandled) return result;
	result = ParseMeshFace(tag, payload, context, object);
	if(result != AseLineStatus::NotHandled) return result;
	result = ParseMeshTextureVertex(tag, payload, context);
	if(result != AseLineStatus::NotHandled) return result;
	result = ParseMeshTextureFace(tag, payload, context);
	if(result != AseLineStatus::NotHandled) return result;
	return ParseMeshMaterialReference(tag, payload, context);
}

AseLineStatus ParsePhysiqueAssignmentCount(const char *tag, const char *payload,
	const AseGeometryContext &context)
{
	if(!summit::formats::ase::TagEquals(tag, "*PHYSIQUE_NUMVERTEXASSIGNMENT"))
		return AseLineStatus::NotHandled;

	summit::formats::ase::LineCursor cursor(payload);
	int assignmentCount = 0;
	if(!cursor.NextInt(&assignmentCount) || assignmentCount < 0 ||
		context.expectedVertexCount < 0 || assignmentCount > context.expectedVertexCount)
		return AseLineStatus::Error;
	return AseLineStatus::Handled;
}

AseLineStatus ParseRigidPhysiqueAssignment(const char *tag, const char *payload,
	AseGeometryContext &context, smOBJ3D *object)
{
	if(!summit::formats::ase::TagEquals(tag, "*PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE"))
		return AseLineStatus::NotHandled;

	summit::formats::ase::LineCursor cursor(payload);
	char nodeName[256];
	if(!cursor.NextInt(&context.currentPhysiqueVertex) || !object->Physique ||
		!summit::io::IndexInRange(context.currentPhysiqueVertex, context.expectedVertexCount) ||
		!cursor.NextQuoted(nodeName, sizeof(nodeName)))
		return AseLineStatus::Error;

	object->Physique[context.currentPhysiqueVertex] = context.import.physique->GetObjectFromName(nodeName);
	return AseLineStatus::Handled;
}

AseLineStatus ParseBlendedPhysiqueStart(const char *tag, const char *payload,
	AseGeometryContext &context, smOBJ3D *object)
{
	if(!summit::formats::ase::TagEquals(tag, "*PHYSIQUE_VERTEXASSIGNMENT_BLENDED_RIGIDTYPE"))
		return AseLineStatus::NotHandled;

	summit::formats::ase::LineCursor cursor(payload);
	if(!cursor.NextInt(&context.currentPhysiqueVertex) || !object->Physique ||
		!summit::io::IndexInRange(context.currentPhysiqueVertex, context.expectedVertexCount))
		return AseLineStatus::Error;
	return AseLineStatus::Handled;
}

AseLineStatus ParseBlendedPhysiqueNode(const char *tag, const char *payload,
	AseGeometryContext &context, smOBJ3D *object)
{
	if(!summit::formats::ase::TagEquals(tag, "*PHYSIQUE_VERTEXASSIGNMENT_NODE"))
		return AseLineStatus::NotHandled;
	if(!object->Physique || !summit::io::IndexInRange(context.currentPhysiqueVertex, context.expectedVertexCount))
		return AseLineStatus::Error;

	summit::formats::ase::LineCursor cursor(payload);
	int nodeSlot = 0;
	int ignoredWeight = 0;
	char nodeName[256];
	if(!cursor.NextInt(&nodeSlot) || !cursor.NextInt(&ignoredWeight) ||
		!cursor.NextQuoted(nodeName, sizeof(nodeName)))
		return AseLineStatus::Error;

	if(nodeSlot == 0)
		object->Physique[context.currentPhysiqueVertex] = context.import.physique->GetObjectFromName(nodeName);
	return AseLineStatus::Handled;
}

AseLineStatus ParsePhysique(const char *tag, const char *payload, AseGeometryContext &context, smOBJ3D *object)
{
	if(!context.import.physique)
		return AseLineStatus::NotHandled;

	AseLineStatus result = ParsePhysiqueAssignmentCount(tag, payload, context);
	if(result != AseLineStatus::NotHandled) return result;
	result = ParseRigidPhysiqueAssignment(tag, payload, context, object);
	if(result != AseLineStatus::NotHandled) return result;
	result = ParseBlendedPhysiqueStart(tag, payload, context, object);
	if(result != AseLineStatus::NotHandled) return result;
	return ParseBlendedPhysiqueNode(tag, payload, context, object);
}

AseLineStatus UpdateBlockDepth(const char *tag, const char *payload, int &blockDepth)
{
	if(summit::formats::ase::TagEquals(tag, "{"))
	{
		++blockDepth;
		return AseLineStatus::Handled;
	}
	if(summit::formats::ase::TagEquals(tag, "}"))
	{
		--blockDepth;
		return blockDepth < 0 ? AseLineStatus::FinishObject : AseLineStatus::Handled;
	}

	summit::formats::ase::LineCursor cursor(payload);
	char token[64];
	while(cursor.NextToken(token, sizeof(token)))
	{
		if(summit::formats::ase::TagEquals(token, "{"))
			++blockDepth;
		else if(summit::formats::ase::TagEquals(token, "}"))
		{
			--blockDepth;
			if(blockDepth < 0)
				return AseLineStatus::FinishObject;
		}
	}

	return AseLineStatus::NotHandled;
}

bool TransformTextureFace(const AseImportContext &import, int materialIndex, int textureIndex, int faceIndex,
	const smFTPOINT *textureVertices, const smTFACE *textureFaces, bool flipOutputV,
	smFTPOINT (&transformed)[3])
{
	if(!textureVertices || !textureFaces || !import.HasMaterial(materialIndex) ||
		!summit::io::IndexInRange(textureIndex, ASE_TEXTURE_SLOT_MAX))
		return false;

	const float offsetU = import.Material(materialIndex).UVW_U_OFFSET[textureIndex];
	const float offsetV = import.Material(materialIndex).UVW_V_OFFSET[textureIndex];
	const float tilingU = import.Material(materialIndex).UVW_U_TILING[textureIndex];
	const float tilingV = import.Material(materialIndex).UVW_V_TILING[textureIndex];
	const float angle = static_cast<float>(import.Material(materialIndex).UVW_ANGLE[textureIndex] * smMULT_PIE);
	const float sinValue = static_cast<float>(GetSin[static_cast<int>(angle) & ANGLE_MASK]) / 65536.0f;
	const float cosValue = static_cast<float>(GetCos[static_cast<int>(angle) & ANGLE_MASK]) / 65536.0f;

	for(int point = 0; point < 3; ++point)
	{
		const DWORD vertexIndex = textureFaces[faceIndex].t[point];
		const float centeredU = textureVertices[vertexIndex].u - 0.5f;
		const float centeredV = textureVertices[vertexIndex].v - 0.5f;
		float u = (centeredU * cosValue) + (centeredV * sinValue);
		float v = (centeredV * cosValue) - (centeredU * sinValue);

		u = (u - offsetU) * tilingU + 0.5f;
		v = (v - offsetV) * tilingV + 0.5f;
		transformed[point].u = u;
		transformed[point].v = flipOutputV ? (1.0f - v) : v;
	}

	return true;
}

bool AddObjectTextureLink(smOBJ3D *object, const AseImportContext &import, int faceIndex, int materialIndex, int textureIndex,
	const smFTPOINT *textureVertices, const smTFACE *textureFaces)
{
	smFTPOINT transformed[3];
	if(!TransformTextureFace(import, materialIndex, textureIndex, faceIndex,
		textureVertices, textureFaces, true, transformed))
		return false;

	object->AddTexLink(faceIndex,
		(DWORD *)&import.materialPattern[import.Material(materialIndex).RegistNum].smTexture[textureIndex],
		&transformed[0], &transformed[1], &transformed[2]);
	return true;
}

bool ResolveObjectMaterial(const AseGeometryContext &context, int face,
	int *materialIndex, int *registeredMaterial, int *textureCount)
{
	if(!materialIndex || !registeredMaterial || !textureCount || !context.faceMaterials ||
		!ResolveAseFaceMaterial(context.import, context.materialRef, context.faceMaterials[face], materialIndex))
		return false;

	*registeredMaterial = context.import.Material(*materialIndex).RegistNum;
	if(!summit::io::IndexInRange(*registeredMaterial, context.import.materialPatternCount))
		return false;

	*textureCount = context.import.materialPattern[*registeredMaterial].TextureCounter;
	return *textureCount >= 0 && *textureCount <= ASE_TEXTURE_SLOT_MAX;
}

bool BuildObjectFaceTextureLinks(smOBJ3D *object, const AseGeometryContext &context,
	int face, int materialIndex, int textureCount)
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
		if(!AddObjectTextureLink(object, context.import, face, materialIndex, texture,
			context.baseTextureVertices.get(), context.baseTextureFaces.get()))
			return false;
	}

	if(context.lightMapChannel &&
		!AddObjectTextureLink(object, context.import, face, materialIndex, regularTextureCount,
			context.lightMapVertices.get(), context.lightMapFaces.get()))
		return false;
	return true;
}

bool BuildObjectMaterials(smOBJ3D *object, const AseGeometryContext &context)
{
	if(context.expectedFaceCount <= 0)
		return true;
	if(!context.faceMaterials || !context.import.materialPattern || context.import.materialPatternCount <= 0)
		return false;

	for(int face = 0; face < context.expectedFaceCount; ++face)
	{
		int materialIndex = 0;
		int registeredMaterial = 0;
		int textureCount = 0;
		if(!ResolveObjectMaterial(context, face, &materialIndex, &registeredMaterial, &textureCount) ||
			!BuildObjectFaceTextureLinks(object, context, face, materialIndex, textureCount))
			return false;

		context.import.materialPattern[registeredMaterial].InUse++;
		object->SetFaceMaterial(face, registeredMaterial);
	}
	return true;
}

bool AppendLastAnimationFrame(AseGeometryContext &context, int lastFrame)
{
	if(context.rotationCount > 0 && context.rotations[context.rotationCount - 1].frame < lastFrame)
	{
		if(context.rotationCount >= ASE_TM_SAMPLE_MAX)
			return false;
		context.rotations[context.rotationCount] = context.rotations[context.rotationCount - 1];
		context.deltaRotations[context.rotationCount] = context.deltaRotations[context.rotationCount - 1];
		context.rotations[context.rotationCount].frame = lastFrame;
		context.deltaRotations[context.rotationCount].frame = lastFrame;
		++context.rotationCount;
	}

	if(context.positionCount > 0 && context.positions[context.positionCount - 1].frame < lastFrame)
	{
		if(context.positionCount >= ASE_TM_SAMPLE_MAX)
			return false;
		context.positions[context.positionCount] = context.positions[context.positionCount - 1];
		context.positions[context.positionCount].frame = lastFrame;
		++context.positionCount;
	}

	if(context.scaleCount > 0 && context.scales[context.scaleCount - 1].frame < lastFrame)
	{
		if(context.scaleCount >= ASE_TM_SAMPLE_MAX)
			return false;
		context.scales[context.scaleCount] = context.scales[context.scaleCount - 1];
		context.scales[context.scaleCount].frame = lastFrame;
		++context.scaleCount;
	}

	return true;
}

bool BuildRotationAnimation(AseGeometryContext &context,
	std::unique_ptr<smFMATRIX[]> &previousRotations, std::unique_ptr<smTM_ROT[]> &rotations)
{
	if(context.rotationCount <= 0)
		return true;

	previousRotations.reset(new smFMATRIX[context.rotationCount]);
	rotations.reset(new smTM_ROT[context.rotationCount]);

	smDMATRIX previousMatrix;
	smDIdentityMatrix(previousMatrix);
	for(int index = 0; index < context.rotationCount; ++index)
	{
		smDTM_ROT rotation = context.deltaRotations[index];
		smDMATRIX rotationMatrix;
		smDQuaternionFromAxis(rotation.x, rotation.y, rotation.z, rotation.w);
		smDMatrixFromQuaternion(rotationMatrix, rotation.x, rotation.y, rotation.z, rotation.w);
		smDMatrixMult(previousMatrix, previousMatrix, rotationMatrix);
		smFMatrixFromDMatrix(previousRotations[index], previousMatrix);
	}

	for(int index = 0; index < context.rotationCount; ++index)
		smQuaternionFromAxis(context.rotations[index].x, context.rotations[index].y,
			context.rotations[index].z, context.rotations[index].w);

	memcpy(rotations.get(), context.rotations, sizeof(smTM_ROT) * context.rotationCount);
	return true;
}

template <typename T>
std::unique_ptr<T[]> CopyAnimationTrack(const T *source, int count)
{
	if(count <= 0)
		return std::unique_ptr<T[]>();

	std::unique_ptr<T[]> destination(new T[count]);
	memcpy(destination.get(), source, sizeof(T) * count);
	return destination;
}

bool CommitAnimation(smOBJ3D *object, AseGeometryContext &context, int lastFrame)
{
	if(!object || !AppendLastAnimationFrame(context, lastFrame))
		return false;

	std::unique_ptr<smFMATRIX[]> previousRotations;
	std::unique_ptr<smTM_ROT[]> rotations;
	if(!BuildRotationAnimation(context, previousRotations, rotations))
		return false;

	std::unique_ptr<smTM_POS[]> positions = CopyAnimationTrack(context.positions, context.positionCount);
	std::unique_ptr<smTM_SCALE[]> scales = CopyAnimationTrack(context.scales, context.scaleCount);

	object->TmPrevRot = previousRotations.release();
	object->TmRot = rotations.release();
	object->TmRotCnt = context.rotationCount;
	object->TmPos = positions.release();
	object->TmPosCnt = context.positionCount;
	object->TmScale = scales.release();
	object->TmScaleCnt = context.scaleCount;
	return true;
}

} // anonymous namespace


FILE *ReadASE_GEOMOBJECT(FILE *fp, AseImportContext &import, smOBJ3D *obj, char *szNodeName = 0)
{
	if(!fp || !obj)
		return FALSE;

	AseGeometryContext context(import);
	char inputLine[256];
	bool objectFinished = false;

	while(fgets(inputLine, sizeof(inputLine), fp) != NULL)
	{
		summit::formats::ase::LineCursor lineCursor(inputLine);
		char tag[64];
		if(!lineCursor.NextToken(tag, sizeof(tag)))
			continue;

		const char *payload = lineCursor.Position();
		AseLineStatus status = ParseObjectMetadata(tag, payload, context, obj, szNodeName);
		if(status == AseLineStatus::NotHandled)
			status = ParseMeshDeclaration(tag, payload, context, obj);
		if(status == AseLineStatus::NotHandled)
			status = ParseTransform(tag, payload, obj);
		if(status == AseLineStatus::NotHandled)
			status = ParseAnimationSample(tag, payload, context);
		if(status == AseLineStatus::NotHandled)
			status = ParseMeshData(tag, payload, context, obj);
		if(status == AseLineStatus::NotHandled)
			status = ParsePhysique(tag, payload, context, obj);

		if(status == AseLineStatus::Error)
			return FALSE;
		if(status == AseLineStatus::SkipObject)
			return fp;

		const AseLineStatus blockStatus = UpdateBlockDepth(tag, payload, context.blockDepth);
		if(blockStatus == AseLineStatus::FinishObject)
		{
			objectFinished = true;
			break;
		}
	}

	if(!objectFinished)
		return FALSE;
	if(!BuildObjectMaterials(obj, context))
		return FALSE;
	if(!CommitAnimation(obj, context, import.sceneLastFrame))
		return FALSE;

	return fp;
}
