#pragma once

#include <memory>
#include <new>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_dynamic_lights
{

inline bool Initialize(smSTAGE3D &stage, int capacity)
{
	if(capacity < 0)
		return false;

	std::unique_ptr<smLIGHT3D[]> lights;
	if(capacity > 0)
	{
		lights.reset(new(std::nothrow) smLIGHT3D[static_cast<std::size_t>(capacity)]);
		if(!lights)
			return false;
	}

	delete[] stage.smLight;
	stage.smLight = lights.release();
	stage.nLight = 0;
	stage.LightCapacity = capacity;
	return true;
}

inline bool Append(smSTAGE3D &stage, int type, const POINT3D &position,
	int red, int green, int blue, int range)
{
	if(stage.nLight < 0 || stage.LightCapacity < 0 || stage.nLight >= stage.LightCapacity ||
		!stage.smLight)
		return false;

	smLIGHT3D &light = stage.smLight[stage.nLight];
	light.type = type;
	light.x = position.x;
	light.y = position.y;
	light.z = position.z;
	light.r = red;
	light.g = green;
	light.b = blue;
	light.Range = range;
	++stage.nLight;
	return true;
}

} // namespace stage_dynamic_lights
} // namespace legacy3d
} // namespace rendering
} // namespace summit
