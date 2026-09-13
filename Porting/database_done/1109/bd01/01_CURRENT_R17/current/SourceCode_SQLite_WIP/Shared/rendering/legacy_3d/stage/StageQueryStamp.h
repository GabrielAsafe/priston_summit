#pragma once

#include <climits>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_query
{

inline void ClearFaceMarks(smSTAGE3D &stage)
{
	if(stage.Face && stage.nFace > 0)
	{
		for(int index = 0; index < stage.nFace; ++index)
			stage.Face[index].CalcSum = 0;
	}
	stage.CalcSumCount = 1;
}

inline int BeginFaceQuery(smSTAGE3D &stage)
{
	if(stage.CalcSumCount <= 0 || stage.CalcSumCount == INT_MAX)
		ClearFaceMarks(stage);

	const int token = stage.CalcSumCount;
	++stage.CalcSumCount;
	return token;
}

inline bool MarkFaceOnce(smSTAGE_FACE &face, int token)
{
	if(face.CalcSum == token)
		return false;
	face.CalcSum = token;
	return true;
}

} // namespace stage_query
} // namespace legacy3d
} // namespace rendering
} // namespace summit
