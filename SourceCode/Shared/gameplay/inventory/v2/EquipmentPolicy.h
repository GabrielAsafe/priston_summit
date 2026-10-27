#pragma once

#include <cstdint>
#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay {
namespace inventory {

// Pure V2 domain contract. No DAT structs, sItem, legacy class flags or IO.
// Equipment-slot bits use the existing wire-visible slot numbering.
struct EquipmentDefinition {
    ItemId item;
    std::uint32_t allowedSlots;
    std::uint16_t widthPixels;
    std::uint16_t heightPixels;
    bool twoHanded;
    EquipmentDefinition() : item(), allowedSlots(0u), widthPixels(0u), heightPixels(0u),
        twoHanded(false) {}
};

class IEquipmentCatalog {
public:
    virtual ~IEquipmentCatalog() {}
    virtual bool TryGetEquipment(ItemId item, EquipmentDefinition& out) const = 0;
};

struct EquipmentAttributes {
    int level, strength, spirit, talent, dexterity, health;
    EquipmentAttributes()
        : level(0), strength(0), spirit(0), talent(0), dexterity(0), health(0) {}
};

struct EquipmentRequest {
    ItemId item;
    int slot;
    EquipmentAttributes required, available;
    bool restricted;
    EquipmentRequest() : item(), slot(0), required(), available(), restricted(false) {}
};

enum class EquipmentDecisionCode {
    Allowed, InvalidItem, UnsupportedSlot, UnknownItem, WrongSlot, Restricted,
    LevelTooLow, StrengthTooLow, SpiritTooLow, TalentTooLow,
    DexterityTooLow, HealthTooLow
};

struct EquipmentDecision {
    EquipmentDecisionCode code;
    explicit EquipmentDecision(EquipmentDecisionCode c) : code(c) {}
    bool Allowed() const { return code == EquipmentDecisionCode::Allowed; }
};

class EquipmentPolicy final {
public:
    static bool SupportedSlot(int slot) {
        // Hands use the same V2 eligibility policy as physical equipment,
        // but their persistence is a separate Companion loadout. Potion stacks
        // have neither ordinary equipment nor hand loadout semantics.
        return slot >= 1 && slot <= 16 && !(slot >= 11 && slot <= 13);
    }
    static EquipmentDecision Evaluate(const IEquipmentCatalog& catalog,
                                      const EquipmentRequest& request) {
        if(!request.item.Valid()) return EquipmentDecision(EquipmentDecisionCode::InvalidItem);
        if(!SupportedSlot(request.slot)) return EquipmentDecision(EquipmentDecisionCode::UnsupportedSlot);
        EquipmentDefinition d;
        if(!catalog.TryGetEquipment(request.item, d) || !d.item.Valid() ||
           d.item.value != request.item.value)
            return EquipmentDecision(EquipmentDecisionCode::UnknownItem);
        if((d.allowedSlots & (std::uint32_t(1) << request.slot)) == 0u)
            return EquipmentDecision(EquipmentDecisionCode::WrongSlot);
        if(request.restricted) return EquipmentDecision(EquipmentDecisionCode::Restricted);
        if(request.required.level > request.available.level) return EquipmentDecision(EquipmentDecisionCode::LevelTooLow);
        if(request.required.strength > request.available.strength) return EquipmentDecision(EquipmentDecisionCode::StrengthTooLow);
        if(request.required.spirit > request.available.spirit) return EquipmentDecision(EquipmentDecisionCode::SpiritTooLow);
        if(request.required.talent > request.available.talent) return EquipmentDecision(EquipmentDecisionCode::TalentTooLow);
        if(request.required.dexterity > request.available.dexterity) return EquipmentDecision(EquipmentDecisionCode::DexterityTooLow);
        if(request.required.health > request.available.health) return EquipmentDecision(EquipmentDecisionCode::HealthTooLow);
        return EquipmentDecision(EquipmentDecisionCode::Allowed);
    }
};

} // namespace inventory
} // namespace gameplay
