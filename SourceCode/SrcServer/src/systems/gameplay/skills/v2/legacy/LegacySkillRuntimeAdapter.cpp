#include "systems/gameplay/skills/v2/legacy/LegacySkillRuntimeAdapter.h"

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/combat/skills/legacy/sinSkill.h"
#include "gameplay/skills/Tempskron_legacy_server.h"
#include "gameplay/skills/Morayion_legacy_server.h"
#include "gameplay/skills/v2/services/SkillService.h"

namespace gameplay
{
namespace skills
{
namespace legacy
{
namespace
{
SkillRuntimeResult Reject(SkillRuntimeCode code)
{
    SkillRuntimeResult result;
    result.code = code;
    return result;
}

std::uint32_t LegacyMotionState(SkillId skill)
{
    // These projectile/weapon skills are driven by the legacy ATTACK clip.
    // All other executable skills use the legacy SKILL clip, including the
    // registry-backed Tier-5 extension set.
    if(skill == SKILL_RAVING || skill == SKILL_CRITICAL_HIT || skill == SKILL_FARINA ||
       skill == SKILL_WIND_ARROW || skill == SKILL_PERFECT_AIM ||
       skill == SKILL_BOMB_SHOT || skill == SKILL_ARROW_OF_RAGE || skill == SKILL_AVALANCHE ||
       skill == SKILL_TWIST_JAVELIN || skill == SKILL_FIRE_JAVELIN || skill == SKILL_SPLIT_JAVELIN ||
       skill == SKILL_ELEMENTAL_SHOT || skill == SKILL_PHOENIX_SHOT ||
       skill == SKILL_VIGOR_SPEAR || skill == SKILL_LIGHTNING_JAVELIN || skill == SKILL_VENGEANCE)
        return CHRMOTION_STATE_ATTACK;
    return CHRMOTION_STATE_SKILL;
}
}

SkillRuntimeResult LegacySkillRuntimeAdapter::Begin(const SkillRuntimeRequest& request)
{
    if(!request.caster.Valid() || request.skill == 0u || request.point < 1 || request.point > 10)
        return Reject(SkillRuntimeCode::InvalidRequest);

    smCHAR* caster = srFindCharFromSerial(static_cast<DWORD>(request.caster.value));
    if(!caster)
        return Reject(SkillRuntimeCode::CasterNotFound);
    if(!caster->Flag)
        return Reject(SkillRuntimeCode::CasterInactive);

    const SkillDefinition* definition = DefaultSkillService().Find(request.skill);
    if(!definition)
        return Reject(SkillRuntimeCode::SkillNotFound);

    SkillRuntimeResult result;
    result.playCode = DefaultSkillService().LegacyPlayCode(request.skill) |
        (static_cast<std::uint32_t>(request.visualParam & 0xF) << 12);
    result.motionState = LegacyMotionState(request.skill);

    const int encodedPlaySkill = static_cast<int>(result.playCode |
        (static_cast<std::uint32_t>(request.point) << 8));
    caster->AttackSkil = encodedPlaySkill;
    caster->SetMotionFromCode(static_cast<DWORD>(result.motionState));
    result.desiredMotionSelected = caster->MotionInfo &&
        static_cast<std::uint32_t>(caster->MotionInfo->State) == result.motionState;
    if(!result.desiredMotionSelected)
    {
        caster->SetMotionFromCode(CHRMOTION_STATE_ATTACK);
        result.fallbackAttackMotion = true;
    }
    caster->AttackSkil = 0;

    result.code = SkillRuntimeCode::Ok;
    return result;
}

LegacySkillRuntimeAdapter& ServerLegacySkillRuntimeAdapter()
{
    static LegacySkillRuntimeAdapter adapter;
    return adapter;
}
}
}
}
