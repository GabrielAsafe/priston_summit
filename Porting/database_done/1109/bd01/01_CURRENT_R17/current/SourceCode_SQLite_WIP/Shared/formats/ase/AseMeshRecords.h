#pragma once

#include <cstddef>
#include <cstring>

#include "formats/ase/AseLineCursor.h"

namespace summit
{
namespace formats
{
namespace ase
{

struct VertexRecord
{
	VertexRecord() : sourceIndex(0), x(0.0f), y(0.0f), z(0.0f) {}

	int sourceIndex;
	float x;
	float y;
	float z;
};

struct FaceRecord
{
	FaceRecord()
		: sourceIndex(0), a(0), b(0), c(0), materialId(0), hasMaterialId(false)
	{
	}

	int sourceIndex;
	int a;
	int b;
	int c;
	int materialId;
	bool hasMaterialId;
};

struct TextureVertexRecord
{
	TextureVertexRecord() : sourceIndex(0), u(0.0f), v(0.0f), w(0.0f) {}

	int sourceIndex;
	float u;
	float v;
	float w;
};


struct ColorRecord
{
	ColorRecord() : sourceIndex(0), r(0.0f), g(0.0f), b(0.0f) {}

	int sourceIndex;
	float r;
	float g;
	float b;
};

struct TriangleIndexRecord
{
	TriangleIndexRecord() : sourceIndex(0), a(0), b(0), c(0) {}

	int sourceIndex;
	int a;
	int b;
	int c;
};

inline bool ParseVertexRecord(const char *payload, VertexRecord *record)
{
	if(!payload || !record)
		return false;

	LineCursor cursor(payload);
	return cursor.NextInt(&record->sourceIndex) &&
		cursor.NextFloat(&record->x) &&
		cursor.NextFloat(&record->y) &&
		cursor.NextFloat(&record->z);
}

inline bool ParseFaceRecord(const char *payload, FaceRecord *record)
{
	if(!payload || !record)
		return false;

	LineCursor cursor(payload);
	char label[64];
	if(!cursor.NextInt(&record->sourceIndex) ||
		!cursor.NextToken(label, sizeof(label)) || !cursor.NextInt(&record->a) ||
		!cursor.NextToken(label, sizeof(label)) || !cursor.NextInt(&record->b) ||
		!cursor.NextToken(label, sizeof(label)) || !cursor.NextInt(&record->c))
		return false;

	for(int scan = 0; scan < 16; ++scan)
	{
		if(!cursor.NextToken(label, sizeof(label)))
			break;
		if(std::strcmp(label, "*MESH_MTLID") != 0)
			continue;

		if(!cursor.NextInt(&record->materialId) || record->materialId < 0)
			return false;
		record->hasMaterialId = true;
		break;
	}
	return true;
}

inline bool ParseTextureVertexRecord(const char *payload, TextureVertexRecord *record)
{
	if(!payload || !record)
		return false;

	LineCursor cursor(payload);
	return cursor.NextInt(&record->sourceIndex) &&
		cursor.NextFloat(&record->u) &&
		cursor.NextFloat(&record->v) &&
		cursor.NextFloat(&record->w);
}


inline bool ParseColorRecord(const char *payload, ColorRecord *record)
{
	if(!payload || !record)
		return false;

	LineCursor cursor(payload);
	return cursor.NextInt(&record->sourceIndex) &&
		cursor.NextFloat(&record->r) &&
		cursor.NextFloat(&record->g) &&
		cursor.NextFloat(&record->b);
}

inline bool ParseTriangleIndexRecord(const char *payload, TriangleIndexRecord *record)
{
	if(!payload || !record)
		return false;

	LineCursor cursor(payload);
	return cursor.NextInt(&record->sourceIndex) &&
		cursor.NextInt(&record->a) &&
		cursor.NextInt(&record->b) &&
		cursor.NextInt(&record->c);
}

} // namespace ase
} // namespace formats
} // namespace summit
