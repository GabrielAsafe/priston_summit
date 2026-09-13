#include "rendering/legacy_3d/object/ObjectVisibility.h"

#define		CLIP_OUT		-32767
#define		ANGCLIP			ANGLE_MASK


int	DEBUG_OBJSUM = 0;





DWORD		smObjFixPos = 0;







int TestScreenView(int x, int y, int z, int Bound, const POINT3D *LocalAngle, const POINT3D *GlobalAngle)
{
#if SUMMIT_LEGACY_OBJECT_VISIBILITY
	(void)LocalAngle;
	if(!GlobalAngle)
		return FALSE;

	const summit::rendering::legacy3d::object_visibility::Viewport viewport = {
		MidX, MidY, viewdistZ, ViewPointLen, smScreenWidth, smScreenHeight
	};
	return summit::rendering::legacy3d::object_visibility::ProjectSphere(
		x, y, z, Bound, GlobalAngle->x, GlobalAngle->y, GlobalAngle->z,
		GetSin, GetCos, ANGCLIP, FLOATDS, FLOATNS, viewport) ? TRUE : FALSE;
#else
	(void)x; (void)y; (void)z; (void)Bound; (void)LocalAngle; (void)GlobalAngle;
	return FALSE;
#endif
}
