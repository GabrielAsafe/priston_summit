#pragma once

#include <algorithm>
#include <vector>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace object_compaction
{

inline bool GeometryStateIsValid(const smOBJ3D &object)
{
	if(object.nVertex < 0 || object.nVertex > object.MaxVertex || object.nFace < 0 || object.nFace > object.MaxFace)
		return false;
	if((object.nVertex > 0 && !object.Vertex) || (object.nFace > 0 && !object.Face))
		return false;
	for(int faceIndex = 0; faceIndex < object.nFace; ++faceIndex)
	{
		const smFACE &face = object.Face[faceIndex];
		if(face.v[0] >= object.nVertex || face.v[1] >= object.nVertex || face.v[2] >= object.nVertex)
			return false;
	}
	return true;
}

inline bool SamePosition(const smVERTEX &left, const smVERTEX &right)
{
	return left.x == right.x && left.y == right.y && left.z == right.z;
}

inline bool CompactVertices(smOBJ3D &object)
{
	if(!GeometryStateIsValid(object))
		return false;
	if(object.nVertex == 0)
		return true;

	const int originalCount = object.nVertex;
	std::vector<smVERTEX> uniqueVertices;
	uniqueVertices.reserve(static_cast<std::size_t>(originalCount));
	std::vector<smOBJ3D *> uniquePhysique;
	if(object.Physique)
		uniquePhysique.reserve(static_cast<std::size_t>(originalCount));
	std::vector<int> oldToUnique(static_cast<std::size_t>(originalCount), -1);

	for(int source = 0; source < originalCount; ++source)
	{
		int match = -1;
		for(int candidate = 0; candidate < static_cast<int>(uniqueVertices.size()); ++candidate)
		{
			if(SamePosition(object.Vertex[source], uniqueVertices[candidate]))
			{
				match = candidate;
				break;
			}
		}
		if(match < 0)
		{
			match = static_cast<int>(uniqueVertices.size());
			uniqueVertices.push_back(object.Vertex[source]);
			if(object.Physique)
				uniquePhysique.push_back(object.Physique[source]);
		}
		oldToUnique[source] = match;
	}

	std::vector<smFACE> remappedFaces(object.Face, object.Face + object.nFace);
	std::vector<unsigned char> used(uniqueVertices.size(), 0);
	for(int faceIndex = 0; faceIndex < object.nFace; ++faceIndex)
	{
		for(int corner = 0; corner < 3; ++corner)
		{
			const int mapped = oldToUnique[remappedFaces[faceIndex].v[corner]];
			if(mapped < 0 || mapped > 0xFFFF)
				return false;
			remappedFaces[faceIndex].v[corner] = static_cast<WORD>(mapped);
			used[static_cast<std::size_t>(mapped)] = 1;
		}
	}

	std::vector<int> uniqueToFinal(uniqueVertices.size(), -1);
	std::vector<smVERTEX> finalVertices;
	finalVertices.reserve(uniqueVertices.size());
	std::vector<smOBJ3D *> finalPhysique;
	if(object.Physique)
		finalPhysique.reserve(uniqueVertices.size());
	for(int source = 0; source < static_cast<int>(uniqueVertices.size()); ++source)
	{
		if(!used[static_cast<std::size_t>(source)])
			continue;
		uniqueToFinal[source] = static_cast<int>(finalVertices.size());
		finalVertices.push_back(uniqueVertices[source]);
		if(object.Physique)
			finalPhysique.push_back(uniquePhysique[source]);
	}

	for(int faceIndex = 0; faceIndex < object.nFace; ++faceIndex)
	{
		for(int corner = 0; corner < 3; ++corner)
		{
			const int mapped = uniqueToFinal[remappedFaces[faceIndex].v[corner]];
			if(mapped < 0 || mapped > 0xFFFF)
				return false;
			remappedFaces[faceIndex].v[corner] = static_cast<WORD>(mapped);
		}
	}

	std::copy(finalVertices.begin(), finalVertices.end(), object.Vertex);
	std::copy(remappedFaces.begin(), remappedFaces.end(), object.Face);
	if(object.Physique)
		std::copy(finalPhysique.begin(), finalPhysique.end(), object.Physique);
	object.nVertex = static_cast<int>(finalVertices.size());
	return true;
}

inline bool SortFacesByMaterial(smOBJ3D &object)
{
	if(!GeometryStateIsValid(object))
		return false;
	std::stable_sort(object.Face, object.Face + object.nFace,
		[](const smFACE &left, const smFACE &right) { return left.v[3] < right.v[3]; });
	return true;
}

} // namespace object_compaction
} // namespace legacy3d
} // namespace rendering
} // namespace summit
