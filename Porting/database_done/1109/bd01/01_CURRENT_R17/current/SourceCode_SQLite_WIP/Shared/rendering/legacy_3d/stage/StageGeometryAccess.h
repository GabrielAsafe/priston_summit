#pragma once

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_geometry
{

inline bool IsFaceTopologyValid(const smSTAGE3D &stage, const smSTAGE_FACE &face)
{
	if(!stage.Vertex || stage.nVertex <= 0)
		return false;
	for(int corner = 0; corner < 3; ++corner)
	{
		if(static_cast<int>(face.Vertex[corner]) >= stage.nVertex)
			return false;
	}
	return true;
}

inline bool FaceVertices(const smSTAGE3D &stage, const smSTAGE_FACE &face,
	const smSTAGE_VERTEX *(&vertices)[3])
{
	if(!IsFaceTopologyValid(stage, face))
		return false;
	for(int corner = 0; corner < 3; ++corner)
		vertices[corner] = &stage.Vertex[face.Vertex[corner]];
	return true;
}


} // namespace stage_geometry
} // namespace legacy3d
} // namespace rendering
} // namespace summit
