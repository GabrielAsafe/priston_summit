#pragma once
// PATCH 266: geometry-local primary ASE UV policy. No synthetic UV generation.
// A face without a texture link is a supported smOBJ3D representation; the
// legacy SMD serializer writes its textureLink token as zero.
namespace summit_ase_uv266 {
enum Mode { Invalid = 0, Absent = 1, Mapped = 2 };
inline Mode Classify(int declared, int readVertices, int readFaces,
                    int geometryFaces, bool pointsAvailable, bool facesAvailable)
{
    if (geometryFaces < 0 || readVertices < 0 || readFaces < 0 || declared < -1)
        return Invalid;
    if (declared <= 0 && readVertices == 0 && readFaces == 0)
        return Absent;
    if (declared > 0 && pointsAvailable && facesAvailable &&
        readVertices == declared && readFaces == geometryFaces)
        return Mapped;
    return Invalid;
}
inline bool FaceIndicesValid(unsigned int a, unsigned int b,
                             unsigned int c, int vertexCount)
{
    return vertexCount > 0 && a < static_cast<unsigned int>(vertexCount) &&
           b < static_cast<unsigned int>(vertexCount) &&
           c < static_cast<unsigned int>(vertexCount);
}
}
