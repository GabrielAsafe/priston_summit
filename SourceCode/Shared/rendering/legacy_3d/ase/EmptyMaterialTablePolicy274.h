#pragma once
// PATCH 274: Offline-only normalization of unbound MESH_MTLID placeholders
// when the ASE explicitly declares *MATERIAL_COUNT 0, no *MATERIAL records,
// and no GEOMOBJECT provides a *MATERIAL_REF.  No texture is fabricated.
namespace summit_ase_mesh274 {
inline bool CanNormalizeUnbound(bool offline, bool explicitlyEmptyTable,
                               int actualMaterialCount, bool explicitReference,
                               int declaredFaces, int parsedFaces,
                               const int* faceMaterialIds)
{
    if (!offline || !explicitlyEmptyTable || actualMaterialCount != 0 ||
        explicitReference || declaredFaces < 0 || declaredFaces != parsedFaces ||
        (declaredFaces > 0 && !faceMaterialIds))
        return false;
    for (int i = 0; i < declaredFaces; ++i) {
        // The field is kept as a 16-bit ID by legacy mesh structures.
        // A negative or unrepresentable raw ID is malformed even if unbound.
        if (faceMaterialIds[i] < 0 || faceMaterialIds[i] > 65535)
            return false;
    }
    return true;
}
}
