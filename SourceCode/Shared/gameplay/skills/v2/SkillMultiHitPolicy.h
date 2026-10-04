#pragma once

namespace gameplay
{
namespace skills
{
class SkillMultiHitPolicy
{
public:
    static bool IsWithinHitWindow(int motionEvent, int hitEventCount)
    {
        return motionEvent < hitEventCount;
    }
};
}
}
