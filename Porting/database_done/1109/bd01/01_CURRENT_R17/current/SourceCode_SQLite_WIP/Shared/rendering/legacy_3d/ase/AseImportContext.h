#pragma once

#include <algorithm>
#include <vector>

namespace summit
{
namespace rendering
{
namespace legacy3d
{

static const int AseMaterialCapacity = MATRIAL_MAX * 2;
static const int AseTextureSlotMax = 8;
static const int AseLightMapReferenceCapacity = 4000;

struct AseLightMapReference
{
	AseLightMapReference() : isLightMap(0), subPoint(0), subMaterialCount(0) {}

	int isLightMap;
	int subPoint;
	int subMaterialCount;
};

// Mutable state for one ASE import operation. Keeping parser state here makes
// dependencies explicit and prevents one import from silently depending on
// material/animation globals left behind by a previous import.
class AseImportContext
{
public:
	AseImportContext()
		: materials(AseMaterialCapacity), materialCount(0), materialPattern(0),
		  materialPatternCount(0), physique(0), sceneLastFrame(0),
		  lightMapStage(false), lightMapReferences(AseLightMapReferenceCapacity),
		  lightMapReferenceCount(0)
	{
		ResetMaterials();
		ResetLightMapState();
	}

	void ResetMaterials()
	{
		std::fill(materials.begin(), materials.end(), ASE_MATERIAL());
		materialCount = 0;
	}

	void ResetLightMapState()
	{
		lightMapStage = false;
		lightMapReferenceCount = 0;
		std::fill(lightMapReferences.begin(), lightMapReferences.end(), AseLightMapReference());
	}

	bool HasMaterial(int index) const
	{
		return index >= 0 && index < materialCount && index < static_cast<int>(materials.size());
	}

	ASE_MATERIAL &Material(int index)
	{
		return materials[index];
	}

	const ASE_MATERIAL &Material(int index) const
	{
		return materials[index];
	}

	void BindMaterialPattern(smMATERIAL *pattern, int count)
	{
		materialPattern = pattern;
		materialPatternCount = count > 0 ? count : 0;
	}

	std::vector<ASE_MATERIAL> materials;
	int materialCount;
	smMATERIAL *materialPattern;
	int materialPatternCount;
	smPAT3D *physique;
	int sceneLastFrame;
	bool lightMapStage;
	std::vector<AseLightMapReference> lightMapReferences;
	int lightMapReferenceCount;
};

} // namespace legacy3d
} // namespace rendering
} // namespace summit
