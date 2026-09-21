#include "systems/ui/v2/legacy/LegacyCharacterStatsGateway.h"

#include <algorithm>
#include "compat/legacy/sin/sinLinkHeader.h"

extern int TempStatePoint[5];

namespace ui::v2::legacy
{
CharacterStatsSnapshot LegacyCharacterStatsGateway::GetSnapshot() const
{
    CharacterStatsSnapshot out;
    if (!sinChar)
        return out;

    out.valid = true;
    out.name = sinChar->szName;
    out.jobName = cInvenTory.JobName;
    out.level = sinChar->Level;
    out.strength = sinChar->Strength;
    out.spirit = sinChar->Spirit;
    out.talent = sinChar->Talent;
    out.dexterity = sinChar->Dexterity;
    out.health = sinChar->Health;
    out.availablePoints = sinChar->StatePoint;

    out.lifeCurrent = sinGetLife();
    out.lifeMax = sinCharDisplay.Life[1];
    out.manaCurrent = sinGetMana();
    out.manaMax = sinCharDisplay.Mana[1];
    out.staminaCurrent = sinGetStamina();
    out.staminaMax = sinCharDisplay.Stamina[1];

    out.attackRating = sinCharDisplay.Attack_Rating;
    out.damageMin = sinCharDisplay.Attack_Damage[0];
    out.damageMax = sinCharDisplay.Attack_Damage[1];
    out.defense = sinCharDisplay.Defence;
    out.absorption = sinCharDisplay.Absorption;
    out.moveSpeed = sinCharDisplay.Move_Speed;
    out.critical = sinCharDisplay.Critical_Hit;
    out.block = sinCharDisplay.Chance_Block;
    out.weightCurrent = sinChar->Weight[0];
    out.weightMax = sinChar->Weight[1];
    for (std::size_t i = 0; i < out.resistances.size(); ++i)
        out.resistances[i] = sinCharDisplay.Resistance[i];

    // Expose the base slopes used by the legacy character formulas. These are
    // descriptive deltas per attribute point; integer truncation, equipment,
    // class/weapon damage formulas and active buffs can make the visible value
    // change in steps rather than on every single point.
    auto& strength = out.attributeImpacts[static_cast<std::size_t>(CharacterAttribute::Strength)];
    strength.life = sinChar->LifeFunction == 1 ? 0.8f : (sinChar->LifeFunction == 2 ? 0.6f : 0.0f);
    strength.stamina = 0.5f;
    strength.maxWeight = 2.0f;
    strength.affectsDamage = true;
    strength.affectsAbsorption = true;

    auto& spirit = out.attributeImpacts[static_cast<std::size_t>(CharacterAttribute::Spirit)];
    if (sinChar->ManaFunction == 1) spirit.mana = 3.8f;
    else if (sinChar->ManaFunction == 2) spirit.mana = 2.7f;
    else if (sinChar->ManaFunction == 3) spirit.mana = 2.2f;
    spirit.stamina = 1.0f;
    spirit.maxWeight = 1.5f;
    spirit.affectsDamage = true;

    auto& talent = out.attributeImpacts[static_cast<std::size_t>(CharacterAttribute::Talent)];
    talent.stamina = 0.5f;
    talent.attackRating = 1.5f;
    talent.defense = 0.25f;
    talent.affectsDamage = true;
    talent.affectsAbsorption = true;
    talent.affectsMoveSpeed = true;

    auto& dexterity = out.attributeImpacts[static_cast<std::size_t>(CharacterAttribute::Dexterity)];
    dexterity.life = sinChar->LifeFunction == 3 ? 0.3f : 0.0f;
    dexterity.attackRating = 3.1f;
    dexterity.defense = 0.5f;
    dexterity.maxWeight = 1.2f;
    dexterity.affectsDamage = true;

    auto& health = out.attributeImpacts[static_cast<std::size_t>(CharacterAttribute::Health)];
    switch (sinChar->LifeFunction)
    {
        case 1: health.life = 2.4f; break;
        case 2: health.life = 2.2f; break;
        case 3: health.life = 2.1f; break;
        case 4: health.life = 2.1f; break;
        case 5: health.life = 1.9f; break;
        default: break;
    }
    health.stamina = 1.4f;
    health.maxWeight = 1.5f;
    health.affectsMoveSpeed = true;

    return out;
}

bool LegacyCharacterStatsGateway::AddAttribute(CharacterAttribute attribute, int amount)
{
    if (!sinChar || amount <= 0 || sinChar->StatePoint < amount)
        return false;

    CheckCharForm();
    for (int step = 0; step < amount; ++step)
    {
        int legacyPoint = 0;
        switch (attribute)
        {
            case CharacterAttribute::Strength: ++sinChar->Strength; legacyPoint = POINT_STRENGTH; break;
            case CharacterAttribute::Spirit: ++sinChar->Spirit; legacyPoint = POINT_SPIRIT; break;
            case CharacterAttribute::Talent: ++sinChar->Talent; legacyPoint = POINT_TALENT; break;
            case CharacterAttribute::Dexterity: ++sinChar->Dexterity; legacyPoint = POINT_DEXTERITY; break;
            case CharacterAttribute::Health: ++sinChar->Health; legacyPoint = POINT_HEALTH; break;
        }
        --sinChar->StatePoint;
        for (int i = 0; i < 4; ++i)
            TempStatePoint[i] = TempStatePoint[i + 1];
        TempStatePoint[4] = legacyPoint;
    }
    Recalculate();
    return true;
}

bool LegacyCharacterStatsGateway::UndoLastAttributePoint()
{
    if (!sinChar)
        return false;

    for (int i = 4; i >= 0; --i)
    {
        if (!TempStatePoint[i])
            continue;

        CheckCharForm();
        switch (TempStatePoint[i])
        {
            case POINT_STRENGTH: if (sinChar->Strength > 0) --sinChar->Strength; break;
            case POINT_SPIRIT: if (sinChar->Spirit > 0) --sinChar->Spirit; break;
            case POINT_TALENT: if (sinChar->Talent > 0) --sinChar->Talent; break;
            case POINT_DEXTERITY: if (sinChar->Dexterity > 0) --sinChar->Dexterity; break;
            case POINT_HEALTH: if (sinChar->Health > 0) --sinChar->Health; break;
            default: return false;
        }
        ++sinChar->StatePoint;
        TempStatePoint[i] = 0;
        Recalculate();
        return true;
    }
    return false;
}

void LegacyCharacterStatsGateway::Recalculate()
{
    ReformCharForm();
    cInvenTory.SetItemToChar();
    cInvenTory.CheckDamage();
}
}
