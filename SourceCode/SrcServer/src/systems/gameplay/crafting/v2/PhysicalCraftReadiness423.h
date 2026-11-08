#pragma once
// SUMMIT423A: authenticated, READ ONLY preflight. No SQL UPDATE, DAT or item mutations.
// Included from the chat ingress after the legacy server player definitions.
#include "systems/gameplay/crafting/v2/PhysicalCraftInventoryPlan423.h"
// SUMMIT433A: not including the retired SQL recipe runtime
#include "systems/gameplay/gathering/v2/GmDropConfig415.h"
#include "systems/gameplay/inventory/v2/legacy/LegacyItemCatalogAdapter.h"
#include "systems/inventory/v2/legacy/LegacyInventoryRuntime.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include <set>
#include <string>
#include <vector>

namespace summit_craft_423 {
// SUMMIT433A: compatibility-only stub for pre-425A callers. Never queries SQL.
// The authenticated JSON-backed preview is summit_craft_425::Plan().
inline std::vector<std::string> PhysicalCraftReadiness(rsPLAYINFO*,
                                                      const std::string&) {
    return {"Craft check antigo retirado. Usar /craft plan CODE (JSON 424A). Sem consumo."};
}
} // namespace summit_craft_423
