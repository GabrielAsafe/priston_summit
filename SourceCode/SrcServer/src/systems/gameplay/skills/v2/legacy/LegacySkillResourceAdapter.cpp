#include "systems/gameplay/skills/v2/legacy/LegacySkillResourceAdapter.h"

#include <algorithm>

#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

namespace gameplay
{
namespace skills
{
namespace legacy
{
namespace
{
SkillResourceCommandResult InvalidResult(SkillResourceCommandCode code)
{
    SkillResourceCommandResult result;
    result.code = code;
    return result;
}

SkillResourceState Snapshot(const smCHAR* caster)
{
    if(!caster)
        return SkillResourceState();
    return SkillResourceState(
        static_cast<int>(caster->smCharInfo.Mana[0]),
        static_cast<int>(caster->smCharInfo.Mana[1]),
        static_cast<int>(caster->smCharInfo.Stamina[0]),
        static_cast<int>(caster->smCharInfo.Stamina[1]));
}

smCHAR* Resolve(EntityId caster)
{
    if(!caster.Valid())
        return 0;
    return srFindCharFromSerial(static_cast<DWORD>(caster.value));
}
}

SkillResourceCommandResult LegacySkillResourceAdapter::Inspect(EntityId caster)
{
    if(!caster.Valid())
        return InvalidResult(SkillResourceCommandCode::InvalidRequest);
    smCHAR* resolved = Resolve(caster);
    if(!resolved)
        return InvalidResult(SkillResourceCommandCode::NotFound);
    if(!resolved->Flag)
        return InvalidResult(SkillResourceCommandCode::Inactive);

    SkillResourceCommandResult result;
    result.code = SkillResourceCommandCode::Ok;
    result.before = Snapshot(resolved);
    result.after = result.before;
    return result;
}

SkillResourceCommandResult LegacySkillResourceAdapter::Spend(EntityId caster, int mana, int stamina)
{
    if(!caster.Valid() || mana < 0 || stamina < 0)
        return InvalidResult(SkillResourceCommandCode::InvalidRequest);
    smCHAR* resolved = Resolve(caster);
    if(!resolved)
        return InvalidResult(SkillResourceCommandCode::NotFound);
    if(!resolved->Flag)
        return InvalidResult(SkillResourceCommandCode::Inactive);

    SkillResourceCommandResult result;
    result.before = Snapshot(resolved);
    result.after = result.before;

    if(result.before.mana < mana)
    {
        result.code = SkillResourceCommandCode::InsufficientMana;
        return result;
    }
    if(result.before.stamina < stamina)
    {
        result.code = SkillResourceCommandCode::InsufficientStamina;
        return result;
    }

    resolved->smCharInfo.Mana[0] = static_cast<short>(result.before.mana - mana);
    resolved->smCharInfo.Stamina[0] = static_cast<short>(result.before.stamina - stamina);
    result.after = Snapshot(resolved);
    result.code = SkillResourceCommandCode::Ok;
    return result;
}

SkillResourceCommandResult LegacySkillResourceAdapter::Restore(EntityId caster, int mana, int stamina)
{
    if(!caster.Valid() || mana < 0 || stamina < 0)
        return InvalidResult(SkillResourceCommandCode::InvalidRequest);
    smCHAR* resolved = Resolve(caster);
    if(!resolved)
        return InvalidResult(SkillResourceCommandCode::NotFound);
    if(!resolved->Flag)
        return InvalidResult(SkillResourceCommandCode::Inactive);

    SkillResourceCommandResult result;
    result.before = Snapshot(resolved);
    resolved->smCharInfo.Mana[0] = static_cast<short>(
        std::min(result.before.maxMana, result.before.mana + mana));
    resolved->smCharInfo.Stamina[0] = static_cast<short>(
        std::min(result.before.maxStamina, result.before.stamina + stamina));
    result.after = Snapshot(resolved);
    result.code = SkillResourceCommandCode::Ok;
    return result;
}

SkillResourceCommandResult LegacySkillResourceAdapter::Drain(EntityId caster, int mana, int stamina)
{
    if(!caster.Valid() || mana < 0 || stamina < 0)
        return InvalidResult(SkillResourceCommandCode::InvalidRequest);
    smCHAR* resolved = Resolve(caster);
    if(!resolved)
        return InvalidResult(SkillResourceCommandCode::NotFound);
    if(!resolved->Flag)
        return InvalidResult(SkillResourceCommandCode::Inactive);

    SkillResourceCommandResult result;
    result.before = Snapshot(resolved);
    resolved->smCharInfo.Mana[0] = static_cast<short>(std::max(0, result.before.mana - mana));
    resolved->smCharInfo.Stamina[0] = static_cast<short>(std::max(0, result.before.stamina - stamina));
    result.after = Snapshot(resolved);
    result.code = SkillResourceCommandCode::Ok;
    return result;
}

LegacySkillResourceAdapter& ServerLegacySkillResourceAdapter()
{
    static LegacySkillResourceAdapter adapter;
    return adapter;
}
}
}
}
