// SUMMIT396A: GM58 combat resource test. Server authority only.
#include "systems/gameplay/gathering/v2/GmCombatResource396.h"
#include "systems/gameplay/gathering/v2/GmDropConfig415.h"
#include "systems/inventory/legacy/sinItem.h"
#include "modules/File/File.h"
#include "systems/network/protocol/server/opcodes.h"
#include "systems/inventory/v2/legacy/LegacyInventoryRuntime.h"
#include "systems/network/v2/legacy/LegacyPacketSender.h"
#include <cctype>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <mutex>
#include <string>
#include <unordered_map>

// Legacy rsPLAYINFO requires this proven server include order.
#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"
#include "core/gameplay/legacy_support/playsub.h"
#include "systems/gameplay/gathering/v2/GatheringRuntime.h"
#include "systems/gameplay/professions/v2/ProfessionRuntime.h" // SUMMIT404A_GM_XP_AFTER_SQLITE
#include "gameplay/professions/v2/ProfessionPolicy439.h" // SUMMIT439A


// SUMMIT414A_NATIVE_CRAFT_MATERIALS_BEGIN
int CreateItem(psITEM* item, char* code, int job);
int rsRegist_ItemSecCode(rsPLAYINFO* player, TRANS_ITEMINFO* packet, int newItem);
static bool GrantNativeCraftItem415(rsPLAYINFO* killer, const std::string& nativeCode) {
    if(!killer || !killer->lpsmSock || !killer->dwObjectSerial) return false;
    psITEM item{};
    if(nativeCode.size()!=5) return false;
    char code[16]{};
    std::memcpy(code,nativeCode.c_str(),nativeCode.size());
    if(CreateItem(&item,code,killer->smCharInfo.JOB_CODE)!=TRUE) return false;
    ReformItem(&item.ItemInfo);
    TRANS_ITEMINFO packet{};
    packet.code=OpCode::OPCODE_PUTITEM;
    packet.size=sizeof(TRANS_ITEMINFO);
    std::memcpy(&packet.Item,&item.ItemInfo,sizeof(sITEMINFO));
    // Resolve the authenticated online inventory, never a player-supplied name.
    auto playerId=gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(killer);
    if(!playerId.Valid())return false;
    rsRegist_ItemSecCode(killer,&packet,1);
    gameplay::inventory::ItemInstanceId id(
        gameplay::ItemId(static_cast<std::uint32_t>(packet.Item.CODE)),
        static_cast<std::uint32_t>(packet.Item.ItemHeader.Head),
        static_cast<std::uint32_t>(packet.Item.ItemHeader.dwChkSum));
    auto& inv=gameplay::inventory::legacy::ServerLegacyInventoryRuntime();
    const auto added=inv.Add(playerId,id);
    if(added.code!=gameplay::inventory::InventoryMutationCode::Applied)return false;
    const bool sent=gameplay::network::legacy::ServerLegacyPacketSender().Send(
        playerId,gameplay::network::PacketView(&packet,sizeof(packet),0u)).Succeeded();
    if(!sent)inv.Remove(playerId,id);
    return sent;
}
// SUMMIT414A_NATIVE_CRAFT_MATERIALS_END

namespace summit_gm396 {
namespace {
std::mutex& Guard() { static std::mutex guard; return guard; }
std::unordered_map<unsigned int, summit_gm415::Rule>& Active() {
    static std::unordered_map<unsigned int, summit_gm415::Rule> active;
    return active;
}
bool Identify(const char* model, const char* display, summit_gm415::Rule& out) {
    // GM spawn registrations only; native IDs resolved against generated 89-item registry.
    return summit_gm415::match(model,display,out);
}

} // namespace

void RegisterSpawn(unsigned int serial, const char* model, const char* display) {
    if (!serial) return;
    summit_gm415::Rule rule;
    if (!Identify(model,display,rule)) {
        std::fprintf(stderr,"[SUMMIT_GM_396] UNMAPPED serial=%u model=%s\n",
                     serial, model ? model : "(null)");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(Guard());
        Active()[serial]=rule;
    }
    std::fprintf(stderr,"[SUMMIT415] REGISTER serial=%u map=58 material=%s native=%s mode=%s\n",
                 serial,rule.resource.c_str(),rule.native.c_str(),rule.physical?"PHYSICAL":"SQLITE");
}

bool ProcessDeath(unsigned int serial, rsPLAYINFO* killer) {
    if (!serial) return false;
    summit_gm415::Rule rule;
    {
        std::lock_guard<std::mutex> lock(Guard());
        const auto it=Active().find(serial);
        if (it==Active().end()) return false; // normal monster: untouched
        rule=it->second;
        Active().erase(it); // consume before DB operation: never reward twice
    }
    // Always suppress legacy loot for managed GM instances, even on failed DB grants.
    if (!killer || !killer->lpsmSock || !killer->dwObjectSerial ||
        !killer->getName() || !*killer->getName()) {
        std::fprintf(stderr,"[SUMMIT415] NO_KILLER serial=%u resource=%s\n",serial,rule.resource.c_str());
        return true;
    }
    if(!summit_gm415::roll(rule.chance)) {
        std::fprintf(stderr,"[SUMMIT415] ROLL_EMPTY serial=%u resource=%s\n",serial,rule.resource.c_str());
        return true;
    }
    const std::string player(killer->getName());
    const std::string& resource=rule.resource;
    // No SQLite fallback for a configured PHYSICAL rule: never conceal an
    // ambiguous failed delivery with a second, independent resource credit.
    const bool nativeGranted=rule.physical && GrantNativeCraftItem415(killer,rule.native);
    const bool ok=rule.physical ? nativeGranted :
        gameplay::gathering::ServerGatheringRepository().AddInventory(player,resource,rule.quantity);
    // SUMMIT439A: XP after the accepted physical/virtual award, NEVER on a failed grant.
    // Crafts only award XP after a durable DAT receipt (not wired yet).
    if(ok && rule.physical) {
        const auto prof=summit_prof439::ForGrantedNativeCI(rule.native);
        if(prof!=gameplay::professions::ProfessionType::None) {
            const bool xp=gameplay::professions::ServerProfessionService().AddExperience(player,prof,1u);
            std::fprintf(stderr,"[SUMMIT439A_PROF] code=%s type=%s xp=1 persisted=%d\n",
                         rule.native.c_str(),summit_prof439::Name(prof),xp?1:0);
        }
    } else if(ok && !rule.physical) {
        // Legacy virtual resource award: preserve the original XP semantics.
        if(resource=="raw_ore_crystalline" || resource=="common_logs") {
            const auto prof=resource=="raw_ore_crystalline" ?
                gameplay::professions::ProfessionType::Miner :
                gameplay::professions::ProfessionType::Lumberjack;
            (void)gameplay::professions::ServerProfessionService().AddExperience(player,prof,1u);
        }
    }
    std::fprintf(stderr,"[SUMMIT415] KILL serial=%u player=%s resource=%s amount=%u storage=%s native=%s normal_loot=0\n",
                 serial,player.c_str(),resource.c_str(),rule.quantity,
                 ok?(rule.physical?"PHYSICAL":"SQLITE"):"FAILED",rule.native.c_str());
    std::fflush(stderr);
    return true;
}
} // namespace summit_gm396
