#include "gameplay/skills/v2/SkillRegistry.h"

#include <map>
#include <sstream>

namespace gameplay
{
namespace skills
{
namespace
{
void SetError(std::string* error, const std::string& value)
{
    if(error) *error = value;
}
}

const char* SkillBehaviorName(SkillBehavior behavior)
{
    switch(behavior)
    {
    case SkillBehavior::Passive: return "PASSIVE";
    case SkillBehavior::DamageSingle: return "DAMAGE_SINGLE";
    case SkillBehavior::DamageArea: return "DAMAGE_AOE";
    case SkillBehavior::Control: return "CONTROL";
    case SkillBehavior::BuffSelf: return "BUFF_SELF";
    case SkillBehavior::BuffGroup: return "BUFF_GROUP";
    case SkillBehavior::BuffTarget: return "BUFF_TARGET";
    case SkillBehavior::HealSingle: return "HEAL_SINGLE";
    case SkillBehavior::HealArea: return "HEAL_AOE";
    case SkillBehavior::DebuffSingle: return "DEBUFF_SINGLE";
    case SkillBehavior::DebuffArea: return "DEBUFF_AOE";
    case SkillBehavior::Resource: return "RESOURCE";
    case SkillBehavior::Summon: return "SUMMON";
    case SkillBehavior::Reactive: return "REACTIVE";
    case SkillBehavior::Utility: return "UTILITY";
    }
    return "UNKNOWN";
}

bool SkillRegistry::Register(const SkillDefinition& definition)
{
    if(definition.id == 0u || definition.ownerClassId <= 0 ||
       definition.useSlot <= 0 || definition.tier <= 0 || definition.name.empty())
        return false;

    for(std::vector<SkillDefinition>::const_iterator it = definitions_.begin();
        it != definitions_.end(); ++it)
    {
        if(it->id == definition.id)
            return false;
        if(it->ownerClassId == definition.ownerClassId &&
           it->useSlot == definition.useSlot)
            return false;
    }

    definitions_.push_back(definition);
    return true;
}

bool SkillRegistry::RegisterAll(const std::vector<SkillDefinition>& definitions, std::string* error)
{
    SkillRegistry candidate = *this;
    for(std::vector<SkillDefinition>::const_iterator it = definitions.begin();
        it != definitions.end(); ++it)
    {
        if(!candidate.Register(*it))
        {
            std::ostringstream stream;
            stream << "duplicate/invalid skill id=0x" << std::hex << it->id
                   << " class=" << std::dec << it->ownerClassId
                   << " slot=" << it->useSlot;
            SetError(error, stream.str());
            return false;
        }
    }
    if(!candidate.Validate(error))
        return false;
    *this = candidate;
    return true;
}

bool SkillRegistry::Validate(std::string* error) const
{
    std::map<SkillId, bool> ids;
    std::map<std::pair<int, int>, bool> slots;

    for(std::vector<SkillDefinition>::const_iterator it = definitions_.begin();
        it != definitions_.end(); ++it)
    {
        if(it->id == 0u || it->ownerClassId <= 0 || it->useSlot <= 0 ||
           it->tier <= 0 || it->name.empty())
        {
            SetError(error, "invalid skill definition");
            return false;
        }

        if(ids[it->id])
        {
            SetError(error, "duplicate logical skill id");
            return false;
        }
        ids[it->id] = true;

        const std::pair<int, int> slotKey(it->ownerClassId, it->useSlot);
        if(slots[slotKey])
        {
            SetError(error, "duplicate class/use-slot pair");
            return false;
        }
        slots[slotKey] = true;

        const int expectedTier = ((it->useSlot - 1) / 4) + 1;
        if(expectedTier != it->tier)
        {
            SetError(error, "tier/use-slot mismatch");
            return false;
        }

        if(it->IsExecutable() && it->legacy.playCode == 0u)
        {
            SetError(error, "executable skill has no legacy play-code binding");
            return false;
        }
    }

    if(error) error->clear();
    return true;
}

const SkillDefinition* SkillRegistry::Find(SkillId id) const
{
    for(std::vector<SkillDefinition>::const_iterator it = definitions_.begin();
        it != definitions_.end(); ++it)
        if(it->id == id)
            return &*it;
    return 0;
}

const SkillDefinition* SkillRegistry::FindByClassAndSlot(int ownerClassId, int useSlot) const
{
    for(std::vector<SkillDefinition>::const_iterator it = definitions_.begin();
        it != definitions_.end(); ++it)
        if(it->ownerClassId == ownerClassId && it->useSlot == useSlot)
            return &*it;
    return 0;
}

std::vector<const SkillDefinition*> SkillRegistry::SkillsForClass(int ownerClassId) const
{
    std::vector<const SkillDefinition*> result;
    for(std::vector<SkillDefinition>::const_iterator it = definitions_.begin();
        it != definitions_.end(); ++it)
        if(it->ownerClassId == ownerClassId)
            result.push_back(&*it);
    return result;
}

}
}
