#pragma once

#include <limits>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_render_stamp
{

inline DWORD BeginRenderPass(smSTAGE3D &stage)
{
	if(stage.nFace < 0 || stage.nVertex < 0 ||
		(stage.nFace > 0 && !stage.Face) || (stage.nVertex > 0 && !stage.Vertex))
		return 0;
	if(stage.SumCount == std::numeric_limits<DWORD>::max())
	{
		for(int index = 0; index < stage.nFace; ++index)
			stage.Face[index].sum = 0;
		for(int index = 0; index < stage.nVertex; ++index)
			stage.Vertex[index].sum = 0;
		stage.SumCount = 1;
		return stage.SumCount;
	}
	++stage.SumCount;
	if(stage.SumCount == 0)
		stage.SumCount = 1;
	return stage.SumCount;
}

} // namespace stage_render_stamp
} // namespace legacy3d
} // namespace rendering
} // namespace summit
