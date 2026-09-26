#pragma once

#include <cstdint>
#include <string>

namespace gameplay
{
namespace skills
{

typedef std::uint32_t SkillId;

enum class SkillBehavior
{
    Passive = 0,
    DamageSingle,
    DamageArea,
    Control,
    BuffSelf,
    BuffGroup,
    BuffTarget,
    HealSingle,
    HealArea,
    DebuffSingle,
    DebuffArea,
    Resource,
    Summon,
    Reactive,
    Utility
};

struct SkillLegacyBinding
{
    // Protocol/play code used by the legacy network/runtime. This is not the
    // logical skill identity and must never replace SkillDefinition::id.
    std::uint32_t playCode;

    // Visual-only alias used when the old client has no native effect/motion
    // for the logical skill. Network/gameplay continue using playCode.
    std::uint32_t presentationPlayCode;

    int finalMotionEvent;
    bool logicalMotionEvents;

    SkillLegacyBinding()
        : playCode(0u), presentationPlayCode(0u), finalMotionEvent(0),
          logicalMotionEvents(false)
    {
    }
};

struct SkillPresentationProfile
{
    std::string actorParticle;
    std::string targetParticle;
    bool persistentVisual;
    int castSound;

    // Timer/icon compatibility is presentation metadata, never gameplay logic.
    std::string canonicalTimerPath;
    std::string fallbackTimerPath;

    SkillPresentationProfile()
        : persistentVisual(false), castSound(0)
    {
    }
};

struct SkillDefinition
{
    SkillId id;
    int ownerClassId;
    int useSlot;
    int tier;
    SkillBehavior behavior;
    bool executable;
    std::string name;

    SkillLegacyBinding legacy;
    SkillPresentationProfile presentation;

    SkillDefinition()
        : id(0u), ownerClassId(0), useSlot(0), tier(0),
          behavior(SkillBehavior::Passive), executable(false)
    {
    }

    bool IsPassive() const { return behavior == SkillBehavior::Passive; }
    bool IsExecutable() const { return executable && !IsPassive(); }
};

inline bool IsOffensiveBehavior(SkillBehavior behavior)
{
    return behavior == SkillBehavior::DamageSingle ||
           behavior == SkillBehavior::DamageArea ||
           behavior == SkillBehavior::Control ||
           behavior == SkillBehavior::DebuffSingle ||
           behavior == SkillBehavior::DebuffArea;
}

inline bool IsBuffBehavior(SkillBehavior behavior)
{
    return behavior == SkillBehavior::BuffSelf ||
           behavior == SkillBehavior::BuffGroup ||
           behavior == SkillBehavior::BuffTarget ||
           behavior == SkillBehavior::Reactive;
}

inline bool IsSupportBehavior(SkillBehavior behavior)
{
    return IsBuffBehavior(behavior) ||
           behavior == SkillBehavior::HealSingle ||
           behavior == SkillBehavior::HealArea ||
           behavior == SkillBehavior::Resource ||
           behavior == SkillBehavior::Summon ||
           behavior == SkillBehavior::Utility;
}

const char* SkillBehaviorName(SkillBehavior behavior);

}
}
