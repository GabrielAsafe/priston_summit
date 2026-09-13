#pragma once

#include <cctype>
#include <cstddef>
#include <cstring>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace pattern_hierarchy
{

static const int PatternObjectCapacity = 128;

inline bool FixedNodeNameIsTerminated(const char *name, std::size_t capacity)
{
	return name && std::memchr(name, 0, capacity) != 0;
}

inline unsigned char AsciiLower(unsigned char value)
{
	return (value >= 'A' && value <= 'Z') ? static_cast<unsigned char>(value - 'A' + 'a') : value;
}

inline bool NodeNameEquals(const char *left, const char *right)
{
	if(!left || !right)
		return false;
	while(*left && *right)
	{
		if(AsciiLower(static_cast<unsigned char>(*left)) != AsciiLower(static_cast<unsigned char>(*right)))
			return false;
		++left;
		++right;
	}
	return *left == *right;
}

inline int FindObjectIndexByName(smOBJ3D *const *objects, int objectCount, const char *name)
{
	if(!objects || objectCount < 0 || objectCount > PatternObjectCapacity || !name || !name[0])
		return -1;
	for(int index = 0; index < objectCount; ++index)
	{
		if(objects[index] && NodeNameEquals(objects[index]->NodeName, name))
			return index;
	}
	return -1;
}

inline bool ResolveParents(smOBJ3D **objects, int objectCount)
{
	if(!objects || objectCount < 0 || objectCount > PatternObjectCapacity)
		return false;

	for(int index = 0; index < objectCount; ++index)
	{
		smOBJ3D *object = objects[index];
		if(!object || !FixedNodeNameIsTerminated(object->NodeName, sizeof(object->NodeName)) ||
			!FixedNodeNameIsTerminated(object->NodeParent, sizeof(object->NodeParent)))
			return false;

		object->pParent = 0;
		if(!object->NodeParent[0])
			continue;
		const int parent = FindObjectIndexByName(objects, objectCount, object->NodeParent);
		if(parent >= 0)
			object->pParent = objects[parent];
		// Preserve the legacy behavior for an unresolved parent: the node becomes
		// a root instead of making an otherwise loadable model unusable.
	}
	return true;
}

inline int FindObjectIndex(smOBJ3D *const *objects, int objectCount, const smOBJ3D *object)
{
	if(!objects || !object)
		return -1;
	for(int index = 0; index < objectCount; ++index)
	{
		if(objects[index] == object)
			return index;
	}
	return -1;
}

inline bool BuildParentFirstOrder(smOBJ3D *const *objects, int objectCount, BYTE *order, int orderCapacity)
{
	if(!objects || !order || objectCount < 0 || objectCount > PatternObjectCapacity || orderCapacity < objectCount)
		return false;

	bool added[PatternObjectCapacity] = {};
	int outputCount = 0;

	for(int index = 0; index < objectCount; ++index)
	{
		if(!objects[index])
			return false;
		if(!objects[index]->pParent)
		{
			order[outputCount++] = static_cast<BYTE>(index);
			added[index] = true;
		}
	}

	while(outputCount < objectCount)
	{
		bool progressed = false;
		for(int index = 0; index < objectCount; ++index)
		{
			if(added[index])
				continue;
			const int parentIndex = FindObjectIndex(objects, objectCount, objects[index]->pParent);
			if(parentIndex < 0)
			{
				// A parent outside this pattern is an external dependency. Keep the
				// node renderable and order it as a root, as the old code effectively did.
				order[outputCount++] = static_cast<BYTE>(index);
				added[index] = true;
				progressed = true;
				continue;
			}
			if(added[parentIndex])
			{
				order[outputCount++] = static_cast<BYTE>(index);
				added[index] = true;
				progressed = true;
			}
		}
		if(!progressed)
			return false; // cycle
	}

	for(int index = outputCount; index < orderCapacity; ++index)
		order[index] = 0;
	return true;
}

inline bool LinkPattern(smOBJ3D **objects, int objectCount, BYTE *order, int orderCapacity)
{
	return ResolveParents(objects, objectCount) &&
		BuildParentFirstOrder(objects, objectCount, order, orderCapacity);
}

} // namespace pattern_hierarchy
} // namespace legacy3d
} // namespace rendering
} // namespace summit
