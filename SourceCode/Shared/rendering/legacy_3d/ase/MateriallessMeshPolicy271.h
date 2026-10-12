#pragma once
// PATCH 271 — legitimate colourless ASE geometry may omit its entire
// MATERIAL_LIST and MATERIAL_REF. Keep SMD material count at zero and
// neutral face index at zero; never infer or fabricate texture references.
namespace summit_ase_mesh271 {
inline bool CanUseNeutral(int materialCount, bool hasRegisteredMaterials,
                          bool explicitMaterialRef, int declaredFaces,
                          int parsedFaces, const int* faceIds)
{
    if (materialCount != 0 || hasRegisteredMaterials || explicitMaterialRef)
        return false;
    if (declaredFaces < 0 || declaredFaces != parsedFaces)
        return false;
    if (declaredFaces > 0 && !faceIds)
        return false;
    for (int face = 0; face < declaredFaces; ++face) {
        if (faceIds[face] != 0)
            return false;
    }
    return true;
}
template<class Object>
inline bool AssignNeutralFaces(Object* obj, int faceCount)
{
    if (!obj || faceCount < 0 || obj->nFace != faceCount)
        return false;
    for (int i = 0; i < faceCount; ++i) {
        if (!obj->SetFaceMaterial(i, 0))
            return false;
    }
    return true;
}
}
