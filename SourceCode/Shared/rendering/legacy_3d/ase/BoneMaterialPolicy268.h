#pragma once
// PATCH 268: Bone-only ASE is animation data; its geometry has no material
// registry. Never read the global ASE material table or create texture links.
// The smOBJ3D faces already exist and SetFaceMaterial(0) selects a neutral
// placeholder, not a diffuse texture. Geometry-less animation nodes are legal.
namespace summit_ase_bone268 {
template<class Object>
inline bool AssignNeutralFaces(Object *obj, int declaredFaces)
{
    if (!obj || declaredFaces < -1)
        return false;
    if (declaredFaces == -1)
        return true; // no *MESH block in this bone transform
    if (declaredFaces != obj->nFace)
        return false;
    for (int face = 0; face < declaredFaces; ++face) {
        if (!obj->SetFaceMaterial(face, 0))
            return false;
    }
    return true;
}
}
