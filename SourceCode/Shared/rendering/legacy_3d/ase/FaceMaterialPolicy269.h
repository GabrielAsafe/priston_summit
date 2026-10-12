#ifndef SUMMIT_FACE_MATERIAL_POLICY_269_H
#define SUMMIT_FACE_MATERIAL_POLICY_269_H

// 3ds Max ASE material IDs on faces can exceed the number of available
// submaterials. The renderer uses the face ID modulo that number. Only
// multi/sub-object material blocks use this rule. Invalid structural bounds
// or negative IDs must never be silently normalized.
namespace summit_ase_face269 {

inline bool Resolve(int parent, int subBase, int subCount,
                    int faceId, int materialCount, int* index) {
    if (!index || materialCount <= 0 || parent < 0 ||
        parent >= materialCount || faceId < 0 || subBase < 0 || subCount < 0)
        return false;
    if (subBase == 0) {
        // An ordinary material ignores *MESH_MTLID (legacy semantics).
        if (subCount != 0) return false;
        *index = parent;
        return true;
    }
    if (subCount == 0 || subBase >= materialCount ||
        subCount > materialCount - subBase)
        return false;
    const int slot = faceId % subCount;
    *index = subBase + slot;
    return *index >= 0 && *index < materialCount;
}

} // namespace summit_ase_face269
#endif
