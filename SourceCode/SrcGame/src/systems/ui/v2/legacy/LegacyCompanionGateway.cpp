#include "systems/ui/v2/legacy/LegacyCompanionGateway.h"

#include <algorithm>
#include <cstring>
#include <windows.h>
#include "gameplay/companions/v2/CompanionDiagnostic315.h"
#include "gameplay/companions/v2/CompanionTradePagePolicy.h"
#include "gameplay/companions/v2/CompanionTradeConfirmPolicy.h"
#include "gameplay/companions/v2/CompanionTradeClientFreeze.h"
#include "gameplay/companions/v2/CompanionTradeLiveRetry350X.h"
#include "gameplay/companions/v2/CompanionTradeClientSaveReceipt.h"
#include "gameplay/companions/v2/CompanionTradeCanonicalInventoryDigest.h"

// sinSkill.h is a legacy, non-self-contained header. Pull in the same minimal
// type providers used by the legacy client before exposing its skill-code
// constants to this UI adapter.
#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "core/data/legacy_loader/fileread.h"
#include "systems/inventory/legacy/sinItem.h"
#include "systems/commerce/legacy/sinTrade.h"
#include "systems/combat/skills/legacy_sin/sinSkill.h"
#include "systems/network/client/netplay.h"
#include "core/gameplay/runtime/playmain.h"
#include "systems/gameplay/character/core/character.h"
#include "systems/persistence/character_record/record.h"
#include "systems/network/v2/legacy/LegacyClientCommandSender.h"
#include "systems/network/v2/legacy/LegacyBackendChannelSender.h"

extern rsRECORD_DBASE rsRecorder;

namespace ui::v2
{
namespace
{
using namespace gameplay::companions::protocol;


std::size_t CountBoundedChars(const char* data, std::size_t capacity)
{
    std::size_t count = 0;
    while (count < capacity && data[count]) ++count;
    return count;
}

const char* StableCompanionSkillName(std::uint32_t skillCode)
{
    switch (skillCode)
    {
        case SKILL_EXTREME_SHIELD: return "Extreme Shield";
        case SKILL_MECHANIC_BOMB: return "Mechanic Bomb";
        case SKILL_PHYSICAL_ABSORB: return "Physical Absorption";
        case SKILL_POISON_ATTRIBUTE: return "Poison Resistance";
        case SKILL_GREAT_SMASH: return "Great Smash";
        case SKILL_MAXIMIZE: return "Maximize";
        case SKILL_AUTOMATION: return "Automation";
        case SKILL_SPARK: return "Spark";
        case SKILL_METAL_ARMOR: return "Metal Armor";
        case SKILL_GRAND_SMASH: return "Grand Smash";
        case SKILL_MECHANIC_WEAPON: return "Mechanic Weapon Mastery";
        case SKILL_SPARK_SHIELD: return "Spark Shield";
        case SKILL_IMPULSION: return "Impulsion";
        case SKILL_COMPULSION: return "Compulsion";
        case SKILL_MAGNETIC_SPHERE: return "Magnetic Sphere";
        case SKILL_METAL_GOLEM: return "Metal Golem";
        case SKILL_MELEE_MASTERY: return "Weapon Mastery";
        case SKILL_FIRE_ATTRIBUTE: return "Fire Resistance";
        case SKILL_RAVING: return "Raving";
        case SKILL_IMPACT: return "Impact";
        case SKILL_TRIPLE_IMPACT: return "Triple Impact";
        case SKILL_BRUTAL_SWING: return "Brutal Swing";
        case SKILL_ROAR: return "Roar";
        case SKILL_RAGE_OF_ZECRAM: return "Rage of Zecram";
        case SKILL_CONCENTRATION: return "Concentration";
        case SKILL_AVANGING_CRASH: return "Avenging Crash";
        case SKILL_SWIFT_AXE: return "Swift Axe";
        case SKILL_BONE_CRASH: return "Bone Crash";
        case SKILL_DETORYER: return "Destroyer";
        case SKILL_BERSERKER: return "Berserker";
        case SKILL_CYCLONE_STRIKE: return "Cyclone Strike";
        case SKILL_BOOST_HEALTH: return "Boost Health";
        case SKILL_PIKE_WIND: return "Pike Wind";
        case SKILL_ICE_ATTRIBUTE: return "Ice Resistance";
        case SKILL_CRITICAL_HIT: return "Critical Hit";
        case SKILL_JUMPING_CRASH: return "Jumping Crash";
        case SKILL_GROUND_PIKE: return "Ground Pike";
        case SKILL_TORNADO: return "Tornado";
        case SKILL_WEAPONE_DEFENCE_MASTERY: return "Weapon Defence Mastery";
        case SKILL_EXPANSION: return "Expansion";
        case SKILL_VENOM_SPEAR: return "Venom Spear";
        case SKILL_VANISH: return "Vanish";
        case SKILL_CRITICAL_MASTERY: return "Critical Mastery";
        case SKILL_CHAIN_LANCE: return "Chain Lance";
        case SKILL_ASSASSIN_EYE: return "Assassin Eye";
        case SKILL_CHARGING_STRIKE: return "Charging Strike";
        case SKILL_VAGUE: return "Vague";
        case SKILL_SHADOW_MASTER: return "Shadow Master";
        case SKILL_SCOUT_HAWK: return "Scout Hawk";
        case SKILL_SHOOTING_MASTERY: return "Shooting Mastery";
        case SKILL_WIND_ARROW: return "Wind Arrow";
        case SKILL_PERFECT_AIM: return "Perfect Aim";
        case SKILL_DIONS_EYE: return "Dions Eye";
        case SKILL_FALCON: return "Falcon";
        case SKILL_ARROW_OF_RAGE: return "Arrow of Rage";
        case SKILL_AVALANCHE: return "Avalanche";
        case SKILL_ELEMENTAL_SHOT: return "Elemental Shot";
        case SKILL_GOLDEN_FALCON: return "Golden Falcon";
        case SKILL_BOMB_SHOT: return "Bomb Shot";
        case SKILL_PERFORATION: return "Perforation";
        case SKILL_RECALL_WOLVERIN: return "Recall Wolverine";
        case SKILL_EVASION_MASTERY: return "Evasion Mastery";
        case SKILL_PHOENIX_SHOT: return "Phoenix Shot";
        case SKILL_FORCE_OF_NATURE: return "Force of Nature";
        case SKILL_SWORD_BLAST: return "Sword Blast";
        case SKILL_HOLY_BODY: return "Holy Body";
        case SKILL_PHYSICAL_TRANING: return "Physical Training";
        case SKILL_DOUBLE_CRASH: return "Double Crash";
        case SKILL_HOLY_VALOR: return "Holy Valor";
        case SKILL_BRANDISH: return "Brandish";
        case SKILL_PIERCING: return "Piercing";
        case SKILL_DRASTIC_SPIRIT: return "Drastic Spirit";
        case SKILL_SWORD_MASTERY: return "Sword Mastery";
        case SKILL_DIVINE_INHALATION: return "Divine Inhalation";
        case SKILL_HOLY_INCANTATION: return "Holy Incantation";
        case SKILL_GRAND_CROSS: return "Grand Cross";
        case SKILL_SWORD_OF_JUSTICE: return "Sword of Justice";
        case SKILL_GODLY_SHIELD: return "Godly Shield";
        case SKILL_GOD_BLESS: return "God Bless";
        case SKILL_DIVINE_PIERCING: return "Divine Piercing";
        case SKILL_SHIELD_STRIKE: return "Shield Strike";
        case SKILL_FARINA: return "Farina";
        case SKILL_THROWING_MASTERY: return "Throwing Mastery";
        case SKILL_VIGOR_SPEAR: return "Vigor Spear";
        case SKILL_WINDY: return "Windy";
        case SKILL_TWIST_JAVELIN: return "Twist Javelin";
        case SKILL_SOUL_SUCKER: return "Soul Sucker";
        case SKILL_FIRE_JAVELIN: return "Fire Javelin";
        case SKILL_SPLIT_JAVELIN: return "Split Javelin";
        case SKILL_TRIUMPH_OF_VALHALLA: return "Triumph of Valhalla";
        case SKILL_LIGHTNING_JAVELIN: return "Lightning Javelin";
        case SKILL_STORM_JAVELIN: return "Storm Javelin";
        case SKILL_HALL_OF_VALHALLA: return "Hall of Valhalla";
        case SKILL_X_RAGE: return "X Rage";
        case SKILL_FROST_JAVELIN: return "Frost Javelin";
        case SKILL_VENGEANCE: return "Vengeance";
        case SKILL_HEALING: return "Healing";
        case SKILL_HOLY_BOLT: return "Holy Bolt";
        case SKILL_MULTISPARK: return "Multi Spark";
        case SKILL_HOLY_MIND: return "Holy Mind";
        case SKILL_MEDITATION: return "Meditation";
        case SKILL_DIVINE_LIGHTNING: return "Divine Lightning";
        case SKILL_HOLY_REFLECTION: return "Holy Reflection";
        case SKILL_GRAND_HEALING: return "Grand Healing";
        case SKILL_VIGOR_BALL: return "Vigor Ball";
        case SKILL_RESURRECTION: return "Resurrection";
        case SKILL_EXTINCTION: return "Extinction";
        case SKILL_VIRTUAL_LIFE: return "Virtual Life";
        case SKILL_GLACIAL_SPIKE: return "Glacial Spike";
        case SKILL_REGENERATION_FIELD: return "Regeneration Field";
        case SKILL_CHAIN_LIGHTNING: return "Chain Lightning";
        case SKILL_SUMMON_MUSPELL: return "Summon Muspell";
        case SKILL_AGONY: return "Agony";
        case SKILL_FIRE_BOLT: return "Fire Bolt";
        case SKILL_ZENITH: return "Zenith";
        case SKILL_FIRE_BALL: return "Fire Ball";
        case SKILL_MENTAL_MASTERY: return "Mental Mastery";
        case SKILL_WATORNADO: return "Watornado";
        case SKILL_ENCHANT_WEAPON: return "Enchant Weapon";
        case SKILL_DEAD_RAY: return "Dead Ray";
        case SKILL_ENERGY_SHIELD: return "Energy Shield";
        case SKILL_DIASTROPHISM: return "Diastrophism";
        case SKILL_SPIRIT_ELEMENTAL: return "Spirit Elemental";
        case SKILL_DANCING_SWORD: return "Dancing Sword";
        case SKILL_FIRE_ELEMENTAL: return "Fire Elemental";
        case SKILL_FLAME_WAVE: return "Flame Wave";
        case SKILL_DISTORTION: return "Distortion";
        case SKILL_M_METEO: return "Meteor";
        default: return "";
    }
}

std::int32_t ToWireAction(CompanionUiAction action)
{
    switch (action)
    {
        case CompanionUiAction::Refresh: return COMPANION_UI_REFRESH;
        case CompanionUiAction::Select: return COMPANION_UI_SELECT;
        case CompanionUiAction::Summon: return COMPANION_UI_SUMMON;
        case CompanionUiAction::Dismiss: return COMPANION_UI_DISMISS;
        case CompanionUiAction::DismissAll: return COMPANION_UI_DISMISS_ALL;
        case CompanionUiAction::SetMode: return COMPANION_UI_SET_MODE;
        case CompanionUiAction::EquipRightNext: return COMPANION_UI_EQUIP_RIGHT_NEXT;
        case CompanionUiAction::EquipRightPrev: return COMPANION_UI_EQUIP_RIGHT_PREV;
        case CompanionUiAction::EquipLeftNext: return COMPANION_UI_EQUIP_LEFT_NEXT;
        case CompanionUiAction::EquipLeftPrev: return COMPANION_UI_EQUIP_LEFT_PREV;
        case CompanionUiAction::ResetEquipment: return COMPANION_UI_RESET_EQUIPMENT;
        case CompanionUiAction::ResyncData: return COMPANION_UI_RESYNC_DAT;
        case CompanionUiAction::SetTactic: return COMPANION_UI_SET_TACTIC;
        case CompanionUiAction::SetRetreatHp: return COMPANION_UI_SET_RETREAT_HP;
        case CompanionUiAction::SetPreferredRange: return COMPANION_UI_SET_PREFERRED_RANGE;
        case CompanionUiAction::CycleActiveSkill: return COMPANION_UI_CYCLE_ACTIVE_SKILL;
        case CompanionUiAction::ResetActiveSkills: return COMPANION_UI_RESET_ACTIVE_SKILLS;
        case CompanionUiAction::CycleBuffSkill: return COMPANION_UI_CYCLE_BUFF_SKILL;
        case CompanionUiAction::AdjustBuffInterval: return COMPANION_UI_ADJUST_BUFF_INTERVAL;
        case CompanionUiAction::ResetBuffs: return COMPANION_UI_RESET_BUFFS;
        case CompanionUiAction::CycleActiveMinResource: return COMPANION_UI_CYCLE_ACTIVE_MIN_RESOURCE;
        case CompanionUiAction::CycleActiveMinTargets: return COMPANION_UI_CYCLE_ACTIVE_MIN_TARGETS;
        case CompanionUiAction::CycleActiveTargetMode: return COMPANION_UI_CYCLE_ACTIVE_TARGET_MODE;
        case CompanionUiAction::ToggleCustomTactics: return COMPANION_UI_TOGGLE_CUSTOM_TACTICS;
        case CompanionUiAction::ToggleTacticRule: return COMPANION_UI_TOGGLE_TACTIC_RULE;
        case CompanionUiAction::CycleTacticCondition: return COMPANION_UI_CYCLE_TACTIC_CONDITION;
        case CompanionUiAction::CycleTacticAction: return COMPANION_UI_CYCLE_TACTIC_ACTION;
        case CompanionUiAction::CycleTacticThreshold: return COMPANION_UI_CYCLE_TACTIC_THRESHOLD;
        case CompanionUiAction::MoveTacticRule: return COMPANION_UI_MOVE_TACTIC_RULE;
        case CompanionUiAction::InventoryPage: return COMPANION_UI_INVENTORY_PAGE;
        case CompanionUiAction::EquipInventoryRight: return COMPANION_UI_EQUIP_INVENTORY_RIGHT;
        case CompanionUiAction::EquipInventoryLeft: return COMPANION_UI_EQUIP_INVENTORY_LEFT;
        case CompanionUiAction::SetInventorySlot: return COMPANION_UI_SET_INVENTORY_SLOT;
        case CompanionUiAction::TradePage: return COMPANION_UI_TRADE_PAGE;
        case CompanionUiAction::MoveBag: return COMPANION_UI_MOVE_BAG;
        case CompanionUiAction::OrganizeBag: return COMPANION_UI_ORGANIZE_BAG;
        case CompanionUiAction::MoveBagCell: return COMPANION_UI_MOVE_BAG_CELL;
    }
    return COMPANION_UI_REFRESH;
}
}

LegacyCompanionGateway& GetLegacyCompanionGateway()
{
    static LegacyCompanionGateway gateway;
    return gateway;
}

void LegacyCompanionGateway::RequestRefresh()
{
    Execute(CompanionUiAction::Refresh);
}

void LegacyCompanionGateway::Execute(CompanionUiAction action, const std::string& characterName, int value)
{
    using namespace gameplay::companions::protocol;
    auto& sender = gameplay::network::legacy::ClientLegacyCommandSender();
    if (!sender.Available())
    {
        summit::companion::diagnostic::Trace("CLIENT", "send-disconnected",
            ToWireAction(action), value, -100);
        summit::companion::diagnostic::Errors().Record(-100);
        return;
    }

    if (action == CompanionUiAction::Select || action == CompanionUiAction::DismissAll ||
        action == CompanionUiAction::InventoryPage)
        inventorySnapshot_ = CompanionInventorySnapshot{};
    if (action == CompanionUiAction::Select || action == CompanionUiAction::DismissAll)
    { tradePages_[0] = CompanionTradePreviewPage{};
      tradePages_[1] = CompanionTradePreviewPage{}; }
    CompanionUIActionPacket packet{};
    packet.size = sizeof(packet);
    packet.code = OPCODE_COMPANION_UI_ACTION;
    packet.version = COMPANION_UI_PROTOCOL_VERSION;
    packet.action = ToWireAction(action);
    packet.value = value;
    if (!characterName.empty())
        std::strncpy(packet.characterName, characterName.c_str(), sizeof(packet.characterName) - 1);

    const auto send = sender.Send(gameplay::network::PacketView(&packet, packet.size, packet.code));
    summit::companion::diagnostic::Trace("CLIENT", "send-ui-action",packet.action,
        value,send.Succeeded() ? 1 : -101);
    if(!send.Succeeded()) summit::companion::diagnostic::Errors().Record(-101);
}

bool LegacyCompanionGateway::RequestTradeAttempt(const std::string& name,
    int side, int index, std::uint32_t code, std::uint32_t key, std::uint32_t checksum,
    std::uint64_t ownerRev, std::uint64_t companionRev)
{
    using namespace gameplay::companions::protocol;
    namespace trade = gameplay::companions::v2;
    // 350X: a live after-image cannot safely replace the running inventory
    // while an item is held by the cursor or another legacy trade/storage UI
    // owns item mutations. Reject BEFORE the client is frozen or a DAT commits.
    if(!trade::TradeLiveCanStart350X(MouseItem.Flag!=0,
            cTrade.OpenFlag!=0,cWareHouse.OpenFlag!=0) ||
       tradeSavingBeforeConfirm_ || tradeSavingAfterApply_ || tradeCommitted_ ||
       trade::TradeClientFrozen() || !lpCurPlayer ||
       !tradePages_[0].received || !tradePages_[1].received ||
       tradePages_[0].status != 1 || tradePages_[1].status != 1 ||
       tradePages_[0].characterName != name || tradePages_[1].characterName != name ||
       tradePages_[0].revision != ownerRev || tradePages_[1].revision != companionRev ||
       name.empty() || name.size() >= 32 || !code || !key || !checksum || index < 0 || index >= 4096 ||
       (side != 0 && side != 1)) return false;
    // Trade and the accepted fresh save MUST use the same authenticated
    // connection.  The primary gameplay socket can differ from the data
    // socket even when both point at the same server/port.
    if(!gameplay::network::legacy::ClientLegacyBackendChannelSender().Available(
           gameplay::network::BackendChannel::Data)) return false;
    CompanionTradeConfirmPacket packet{};
    packet.size = sizeof(packet);
    packet.code = OPCODE_COMPANION_TRADE_CONFIRM;
    packet.version = COMPANION_TRADE_CONFIRM_VERSION;
    packet.side = side;
    packet.itemIndex = index;
    packet.itemCode = code;
    packet.itemKey = key;
    packet.itemChecksum = checksum;
    packet.ownerRevision = ownerRev;
    packet.companionRevision = companionRev;
    ++tradeRequestId_;
    if(!tradeRequestId_) ++tradeRequestId_;
    packet.requestId = tradeRequestId_;
    std::strncpy(packet.characterName, name.c_str(), sizeof(packet.characterName)-1);
    if(!trade::ValidTradeConfirmRequest(packet)) return false;

    // The input callback must never serialize or write character state.
    // Arm the freeze and defer the fresh save to the normal UI Update tick.
    // Server-side receipt + the exclusive lease remain mandatory.
    trade::SetTradeClientFrozen(true);
    tradeAttempt_ = CompanionTradeAttemptResult{};
    tradeReceiver_.Reset();
    pendingTradeConfirm_ = packet;
    tradeSaveScheduled_ = true;
    tradeSavingBeforeConfirm_ = false;
    tradeSavingAfterApply_ = false;
    tradeLiveAwaitingServer350Q_ = false;
    tradeLivePostSaveScheduled350Q_ = false;
    tradeLiveApplyAckPending350X_=false;
    tradeLiveApplyLastAttempt350X_=0;
    tradeLivePostSaveAttempt350X_=0;
    tradeLiveAckRetryTick350X_=0;
    tradeCommitted_ = false;
    pendingTradePlayerImage_.clear();
    tradeHandshakeTick_ = GetTickCount();
    tradeConfirmSent_ = false;
    tradeWaitReported_ = false;
    summit::companion::diagnostic::Trace("CLIENT", "trade-armed",
        side, static_cast<int>(tradeRequestId_), 1);
    return true;
}

namespace {
bool CurrentSavedInventoryDigest(gameplay::companions::v2::TradeSha256Digest& result)
{
    namespace trade = gameplay::companions::v2;
    if(!lpCurPlayer) return false;
    const TRANS_RECORD_DATA& record=rsRecorder.TransRecordData;
    if(record.size < static_cast<int>(offsetof(TRANS_RECORD_DATA,Data)) ||
       record.size > static_cast<int>(sizeof(TRANS_RECORD_DATA))) return false;
    const auto* begin=reinterpret_cast<const std::uint8_t*>(&record);
    std::vector<std::uint8_t> bytes(begin,begin+record.size);
    const trade::BufferedTradeEnvelopeLayout layout(
        offsetof(TRANS_RECORD_DATA,size),offsetof(TRANS_RECORD_DATA,ItemCount),
        offsetof(TRANS_RECORD_DATA,ItemSubStart),offsetof(TRANS_RECORD_DATA,DataSize),
        offsetof(TRANS_RECORD_DATA,smCharInfo)+offsetof(smCHAR_INFO,szName),
        sizeof(((smCHAR_INFO*)0)->szName),offsetof(TRANS_RECORD_DATA,Data),
        sizeof(TRANS_RECORD_DATA));
    return trade::TradeCanonicalInventoryDigest::Compute(bytes,layout,result);
}
}

void LegacyCompanionGateway::ApplyCommittedDeliveryChunk(
    const gameplay::companions::v2::TradeDeliveryChunk& chunk)
{
    namespace trade = gameplay::companions::v2;
    using namespace gameplay::companions::protocol;
    if(!trade::TradeClientFrozen() || !lpCurPlayer ||
       !lpCurPlayer->smCharInfo.szName[0] ||
       !tradePages_[0].received || !tradePages_[1].received ||
       tradePages_[0].characterName!=tradePages_[1].characterName ||
       chunk.requestId!=tradeRequestId_) return;
    std::lock_guard<std::mutex> guard(tradeDeliveryMutex_);
    std::vector<std::uint8_t> completed;
    const auto status=tradeReceiver_.Accept(chunk,OPCODE_COMPANION_TRADE_DELIVERY,
        COMPANION_TRADE_DELIVERY_VERSION,tradeRequestId_,
        lpCurPlayer->smCharInfo.szName,tradePages_[0].characterName.c_str(),completed);
    if(status==trade::TradeDeliveryPacketStatus::Complete) {
        pendingTradePlayerImage_=std::move(completed);
        pendingTradeTxnId_=tradeReceiver_.TransactionId();
        deliveredBeforeDigest_=tradeReceiver_.BeforeInventoryDigest();
    }
}

void LegacyCompanionGateway::PumpTradeExecution()
{
    namespace trade = gameplay::companions::v2;
    using namespace gameplay::companions::protocol;
    auto& sender=gameplay::network::legacy::ClientLegacyCommandSender();
    // 350X: if the first live APPLY ACK could not be transmitted, resend ONLY
    // the authenticated ACK. Do not reapply the image or touch DAT again.
    if(tradeLiveApplyAckPending350X_ && tradeCommitted_) {
        const DWORD now=GetTickCount();
        if(trade::TradeLiveRetryDue350X(now,tradeLiveAckRetryTick350X_,1250u)) {
            tradeLiveAckRetryTick350X_=now;
            CompanionTradeApplyAckPacket ack{};
            ack.size=sizeof(ack);ack.code=OPCODE_COMPANION_TRADE_APPLY_ACK;
            ack.version=COMPANION_TRADE_DELIVERY_VERSION;ack.status=1;
            ack.requestId=tradeRequestId_;ack.txnId=pendingTradeTxnId_;
            std::strncpy(ack.characterName,pendingTradeConfirm_.characterName,
                         sizeof(ack.characterName)-1);
            auto& dataSender=gameplay::network::legacy::ClientLegacyBackendChannelSender();
            if(dataSender.Available(gameplay::network::BackendChannel::Data) &&
               dataSender.Send(gameplay::network::BackendChannel::Data,
                   gameplay::network::PacketView(&ack,ack.size,ack.code)).Succeeded()) {
                tradeLiveApplyAckPending350X_=false;
                tradeLiveAwaitingServer350Q_=true;
            }
        }
    }
    if(tradeLivePostSaveScheduled350Q_) {
        const DWORD now=GetTickCount();
        if(!trade::TradeLiveRetryDue350X(now,tradeLivePostSaveAttempt350X_,1250u)) return;
        tradeLivePostSaveAttempt350X_=now;
        trade::ArmTradeClientSaveReceipt();
        // The server now verifies the FIRST complete post-trade save against
        // the journal's canonical inventory digest before accepting it.
        if(SaveGameData()!=TRUE) {
            // A transient busy save (dwRecordTime/legacy UI/connection) must
            // not abandon the live handoff permanently. No save has been
            // sent, so leave the first-save barrier armed and retry later.
            trade::SetTradeClientFrozen(true);
            summit::companion::diagnostic::Trace("CLIENT","trade-postsave-retry",
                0,static_cast<int>(tradeRequestId_),
                rsRecorder.dwRecordTime ? 1 :
                (cTrade.OpenFlag ? 2 : (cWareHouse.OpenFlag ? 3 : 4)));
            return;
        }
        tradeLivePostSaveScheduled350Q_=false;
        tradeSavingAfterApply_=true;
        tradeHandshakeTick_=GetTickCount();
        tradeWaitReported_=false;
        summit::companion::diagnostic::Trace("CLIENT","trade-postsave-sent",
            0,static_cast<int>(tradeRequestId_),1);
        return;
    }
    // A missing save result MUST NOT leave the bag frozen forever. In this
    // phase the CONFIRM has not been sent, so no pairwise commit is possible.
    if(tradeSavingBeforeConfirm_ &&
       static_cast<DWORD>(GetTickCount() - tradeHandshakeTick_) >= 12000u) {
        tradeSavingBeforeConfirm_=false;
        trade::SetTradeClientFrozen(false);
        tradeAttempt_.received=true;
        tradeAttempt_.status=-55;
        tradeAttempt_.requestId=tradeRequestId_;
        tradeAttempt_.characterName=pendingTradeConfirm_.characterName;
        summit::companion::diagnostic::Trace("CLIENT", "trade-save-timeout",
            0,static_cast<int>(tradeRequestId_),-55);
        return;
    }
    // Once the CONFIRM may have been delivered, timeout is NOT a rollback.
    // Mark the transaction UNCERTAIN and require relogin/reconciliation.
    if(tradeConfirmSent_ && !tradeWaitReported_ && !tradeAttempt_.received &&
       static_cast<DWORD>(GetTickCount() - tradeHandshakeTick_) >= 15000u) {
        tradeWaitReported_=true;
        tradeCommitted_=true;
        tradeAttempt_.received=true;
        tradeAttempt_.status=-75;
        tradeAttempt_.requestId=tradeRequestId_;
        tradeAttempt_.characterName=pendingTradeConfirm_.characterName;
        summit::companion::diagnostic::Trace("CLIENT", "trade-confirm-timeout",
            0,static_cast<int>(tradeRequestId_),-75);
        // Leave inventory FROZEN. The server may have committed to DAT.
    }
    // Never save inside Button::OnClick (recursive UI / inventory mutation).
    // A failed save is a local rejection; NO confirm packet was sent.
    if(tradeSaveScheduled_) {
        tradeSaveScheduled_=false;
        trade::ArmTradeClientSaveReceipt();
        summit::companion::diagnostic::Trace("CLIENT", "trade-save-start",
            0,static_cast<int>(tradeRequestId_),0);
        if(SaveGameData()!=TRUE) {
            summit::companion::diagnostic::Trace("CLIENT", "trade-save-send-fail",
                0,static_cast<int>(tradeRequestId_),-51);
            tradeSavingBeforeConfirm_=false;
            trade::SetTradeClientFrozen(false);
            tradeAttempt_.received=true;
            tradeAttempt_.status=-51;
            tradeAttempt_.requestId=tradeRequestId_;
            tradeAttempt_.characterName=pendingTradeConfirm_.characterName;
            return;
        }
        tradeSavingBeforeConfirm_=true;
        tradeHandshakeTick_=GetTickCount();
        summit::companion::diagnostic::Trace("CLIENT", "trade-save-sent",
            0,static_cast<int>(tradeRequestId_),1);
        return;
    }
    if(tradeSavingBeforeConfirm_) {
        if(GetSaveResult()!=TRUE) return;
        const int outcome=trade::TradeClientSaveReceiptStatus();
        if(outcome==0) return;
        summit::companion::diagnostic::Trace("CLIENT", "trade-save-ack-observed",
            0,static_cast<int>(tradeRequestId_),outcome);
        if(outcome<0) {
            tradeSavingBeforeConfirm_=false;
            trade::SetTradeClientFrozen(false);
            tradeAttempt_.received=true;
            tradeAttempt_.status=-52;
            tradeAttempt_.requestId=tradeRequestId_;
            tradeAttempt_.characterName=pendingTradeConfirm_.characterName;
            return;
        }
        tradeSavingBeforeConfirm_=false;
        // The OPCODE_RECORDDATA save was sent over BackendChannel::Data.
        // Confirm over that exact channel so the server reserves the same
        // rsPLAYINFO / SaveSessionKey that accepted the new save.
        auto& dataSender = gameplay::network::legacy::ClientLegacyBackendChannelSender();
        if(!CurrentSavedInventoryDigest(beforeInventoryDigest_) ||
           !dataSender.Available(gameplay::network::BackendChannel::Data)) {
            // Confirm not attempted: no trade can have committed.
            trade::SetTradeClientFrozen(false);
            tradeAttempt_.received=true;
            tradeAttempt_.status=-53;
            tradeAttempt_.requestId=tradeRequestId_;
            tradeAttempt_.characterName=pendingTradeConfirm_.characterName;
            return;
        }
        summit::companion::diagnostic::Trace("CLIENT", "trade-confirm-attempt",
            0,static_cast<int>(tradeRequestId_),0);
        // From the first send attempt onward, outcome is uncertain.
        // Even a transport failure cannot certify that the server saw nothing.
        tradeConfirmSent_=true;
        tradeHandshakeTick_=GetTickCount();
        if(!dataSender.Send(gameplay::network::BackendChannel::Data,
               gameplay::network::PacketView(&pendingTradeConfirm_,
                   sizeof(pendingTradeConfirm_),pendingTradeConfirm_.code)).Succeeded()) {
            summit::companion::diagnostic::Trace("CLIENT", "trade-confirm-send-error",
                0,static_cast<int>(tradeRequestId_),-74);
            // A failed transport return does NOT prove that the request was
            // not delivered. Do not unfreeze or risk a stale second save.
            tradeCommitted_=true;
            tradeAttempt_.received=true;
            tradeAttempt_.status=-74;
            tradeAttempt_.requestId=tradeRequestId_;
            tradeAttempt_.characterName=pendingTradeConfirm_.characterName;
            return;
        }
        summit::companion::diagnostic::Trace("CLIENT", "trade-confirm-sent",
            0,static_cast<int>(tradeRequestId_),1);
        // Keep the client frozen between the fresh save and the committed
        // delivery. A negative server result will release this freeze.
    }
    if(!tradeCommitted_ || tradeSavingAfterApply_) {
        if(!tradeSavingAfterApply_) return;
        if(GetSaveResult()!=TRUE) {
            if(!tradeWaitReported_ &&
               static_cast<DWORD>(GetTickCount()-tradeHandshakeTick_)>=12000u) {
                tradeWaitReported_=true;
                summit::companion::diagnostic::Trace("CLIENT","trade-postsave-wait",
                    0,static_cast<int>(tradeRequestId_),0);
            }
            return; // no ACK: NEVER thaw an inventory whose commit is durable
        }
        const int outcome=trade::TradeClientSaveReceiptStatus();
        if(outcome==0) return;
        if(outcome<0) {
            summit::companion::diagnostic::Trace("CLIENT","trade-postsave-rejected",
                0,static_cast<int>(tradeRequestId_),outcome);
            // Already committed: keep frozen and recover from disk on reconnect.
            tradeSavingAfterApply_=false;
            tradeAttempt_.received=true;
            tradeAttempt_.status=-74;
            return;
        }
        tradeSavingAfterApply_=false;
        // Do not rely on the second ACK for the server-side commit; it is
        // diagnostic only. The first post-apply save was already verified.
        if(sender.Available()) {
            CompanionTradeApplyAckPacket ack{};
            ack.size=sizeof(ack);ack.code=OPCODE_COMPANION_TRADE_APPLY_ACK;
            ack.version=COMPANION_TRADE_DELIVERY_VERSION;ack.status=2;
            ack.requestId=tradeRequestId_;ack.txnId=pendingTradeTxnId_;
            std::strncpy(ack.characterName,pendingTradeConfirm_.characterName,
                         sizeof(ack.characterName)-1);
            (void)sender.Send(gameplay::network::PacketView(&ack,ack.size,ack.code));
        }
        // A journal commit already occurred. If the server does not accept
        // the new save, it must force a reload from the durable record.
        tradeCommitted_=false;
        tradeLiveAwaitingServer350Q_=false;
        tradeLiveApplyAckPending350X_=false;
        trade::SetTradeClientFrozen(false);
        summit::companion::diagnostic::Trace("CLIENT","trade-live-unlocked",
            0,static_cast<int>(tradeRequestId_),1);
        Execute(CompanionUiAction::TradePage,pendingTradeConfirm_.characterName,0);
        Execute(CompanionUiAction::TradePage,pendingTradeConfirm_.characterName,1);
        return;
    }
    std::vector<std::uint8_t> image;
    std::uint64_t txn=0;
    trade::TradeSha256Digest expectedBefore{};
    {
        std::lock_guard<std::mutex> guard(tradeDeliveryMutex_);
        if(pendingTradePlayerImage_.empty()) return;
        // 350X: throttle expensive decode/texture refresh on a transient
        // apply refusal, but preserve the immutable committed DAT image.
        const DWORD now=GetTickCount();
        if(!trade::TradeLiveRetryDue350X(now,tradeLiveApplyLastAttempt350X_,750u)) return;
        tradeLiveApplyLastAttempt350X_=now;
        image.swap(pendingTradePlayerImage_);
        txn=pendingTradeTxnId_;
        expectedBefore=deliveredBeforeDigest_;
    }
    // 350W: identify failures before an Apply ACK. A frozen inventory after
    // live-image-sent is a recovery-required state, not a successful sync.
    if(expectedBefore!=beforeInventoryDigest_) {
        summit::companion::diagnostic::Trace("CLIENT","trade-live-digest-refused",
            0,static_cast<int>(tradeRequestId_),-1);
        return;
    }
    summit::companion::diagnostic::Trace("CLIENT","trade-live-apply-start",
        0,static_cast<int>(tradeRequestId_),static_cast<int>(image.size()));
    if(ApplyCommittedCompanionTradePlayerInventory(image.data(),
                 static_cast<int>(image.size()))!=TRUE) {
        // 350X: Apply can refuse for an in-flight cursor or legacy window.
        // The prior implementation had SWAPPED away the only committed image,
        // then returned: no next attempt was possible and the bag froze forever.
        // Restore it by move (no DAT mutation), retry on the global UI tick.
        {
            std::lock_guard<std::mutex> guard(tradeDeliveryMutex_);
            if(pendingTradePlayerImage_.empty() && txn==pendingTradeTxnId_)
                pendingTradePlayerImage_=std::move(image);
        }
        summit::companion::diagnostic::Trace("CLIENT","trade-live-apply-retry",
            0,static_cast<int>(tradeRequestId_),
            MouseItem.Flag ? 1 : (cTrade.OpenFlag ? 2 : (cWareHouse.OpenFlag ? 3 : 4)));
        // No authenticated apply ACK, no thaw, no stale save.
        return;
    }
    CompanionTradeApplyAckPacket ack{};
    ack.size=sizeof(ack);ack.code=OPCODE_COMPANION_TRADE_APPLY_ACK;
    ack.version=COMPANION_TRADE_DELIVERY_VERSION;ack.status=1;
    ack.requestId=tradeRequestId_;ack.txnId=txn;
    std::strncpy(ack.characterName,pendingTradeConfirm_.characterName,
                 sizeof(ack.characterName)-1);
    // The journal image is in the running bags, but the server session is
    // STILL SEALED. Do not send a legacy save until the authenticated server
    // rebases its ledger and authorises the first post-trade save.
    auto& dataSender=gameplay::network::legacy::ClientLegacyBackendChannelSender();
    if(dataSender.Available(gameplay::network::BackendChannel::Data) &&
       dataSender.Send(gameplay::network::BackendChannel::Data,
           gameplay::network::PacketView(&ack,ack.size,ack.code)).Succeeded()) {
        tradeLiveAwaitingServer350Q_=true;
        summit::companion::diagnostic::Trace("CLIENT","trade-live-applied",
            0,static_cast<int>(tradeRequestId_),1);
    }
    else {
        // After applying, the image must not be applied a second time.
        // Retain the exact ACK identity, not the image, for retransmission.
        tradeLiveApplyAckPending350X_=true;
        summit::companion::diagnostic::Trace("CLIENT","trade-live-ack-retry",
            0,static_cast<int>(tradeRequestId_),-1);
    }
}

void LegacyCompanionGateway::ApplyTradeResult(
    const gameplay::companions::protocol::CompanionTradeResultPacket& p)
{
    if(!gameplay::companions::v2::ValidTradeResult(p) ||
       p.requestId != tradeRequestId_) return;
    summit::companion::diagnostic::Trace("CLIENT", "trade-result-received",
        0,static_cast<int>(p.requestId),p.status);
    const std::string name(p.characterName);
    if(tradePages_[0].characterName != name ||
       tradePages_[1].characterName != name) return;
    if(p.status==1 && tradeLiveAwaitingServer350Q_) {
        // This is a second, authenticated server result for THIS request:
        // after-image has been installed in the server ledger and buffer.
        // The session remains locally frozen until our fresh save is ACKed.
        tradeLiveAwaitingServer350Q_=false;
        tradeLiveApplyAckPending350X_=false;
        tradeLivePostSaveScheduled350Q_=true;
        tradeSavingBeforeConfirm_=false;
        tradeCommitted_=true;
    } else if(p.status == 2 || p.status == 3 || p.status == 1) {
        // 2/3 are the production journal+relogin outcomes. Do not treat
        // them as an ordinary preview result: the player's in-memory bags
        // are stale until a new login loads the committed DAT. The server
        // deliberately keeps the socket open to avoid an abrupt FD_CLOSE.
        tradeSavingBeforeConfirm_=false;
        tradeCommitted_=true;
        gameplay::companions::v2::SetTradeClientFrozen(true);
    } else if(p.status == -74) {
        // Disk commit/recovery may already have modified one or both files.
        // Never unlock the old inventory on an uncertain result.
        tradeSavingBeforeConfirm_=false;
        tradeCommitted_=true;
        gameplay::companions::v2::SetTradeClientFrozen(true);
    } else if(p.status < 0) {
        // A definitive rejection is safe; uncertain (-74) is handled above.
        tradeConfirmSent_=false;
        tradeSavingBeforeConfirm_=false;
        if(!tradeCommitted_ && !tradeSavingAfterApply_)
            gameplay::companions::v2::SetTradeClientFrozen(false);
    }
    tradeAttempt_.received = true;
    tradeAttempt_.status = p.status;
    tradeAttempt_.requestId = p.requestId;
    tradeAttempt_.characterName = name;
}

void LegacyCompanionGateway::ApplyState(const gameplay::companions::protocol::CompanionUIStatePacket& packet)
{
    using namespace gameplay::companions::protocol;
    if (packet.version != COMPANION_UI_PROTOCOL_VERSION)
    {
        summit::companion::diagnostic::Trace("CLIENT","bad-state-version",0,
            packet.version,-102);
        summit::companion::diagnostic::Errors().Record(-102);
        return;
    }
    // This error cannot be cleared by a later state refresh.
    if(packet.lastResult < 0)
        summit::companion::diagnostic::Errors().Record(packet.lastResult);
    if(packet.lastResult != 0)
        summit::companion::diagnostic::Trace("CLIENT","state-result",0,
            0,packet.lastResult);

    snapshot_ = CompanionUiSnapshot{};
    snapshot_.valid = true;
    snapshot_.mode = static_cast<CompanionUiMode>(std::clamp(packet.mode, 0, 2));
    snapshot_.activeCount = packet.activeCount;
    snapshot_.maxActive = packet.maxActive;
    snapshot_.lastResult = packet.lastResult;
    snapshot_.skillLoadoutActive = packet.skillLoadoutActive != 0;
    snapshot_.buffLoadoutActive = packet.buffLoadoutActive != 0;
    snapshot_.customTacticsEnabled = packet.customTacticsEnabled != 0;
    for (int rule = 0; rule < COMPANION_UI_TACTIC_RULES; ++rule)
    {
        const auto& sourceRule = packet.tacticRules[rule];
        auto& targetRule = snapshot_.tacticRules[static_cast<std::size_t>(rule)];
        targetRule.enabled = sourceRule.enabled != 0;
        targetRule.condition = std::clamp(sourceRule.condition, 0, (int)COMPANION_TACTIC_CONDITION_PARTY_THREATENED);
        targetRule.action = std::clamp(sourceRule.action, 0, (int)COMPANION_TACTIC_ACTION_HOLD_POSITION);
        targetRule.threshold = std::clamp(sourceRule.threshold, 0, 95);
    }
    for (int slot = 0; slot < 4; ++slot)
    {
        const auto& sourceActive = packet.activeSkills[slot];
        auto& targetActive = snapshot_.activeSkills[static_cast<std::size_t>(slot)];
        targetActive.useSlot = sourceActive.useSlot;
        targetActive.skillCode = sourceActive.skillCode;
        targetActive.point = sourceActive.point;
        targetActive.intervalSeconds = sourceActive.intervalSeconds;
        targetActive.minResourcePercent = std::clamp(sourceActive.minResourcePercent, 0, 90);
        targetActive.minimumTargets = std::clamp(sourceActive.minimumTargets, 1, 16);
        targetActive.targetMode = std::clamp(sourceActive.targetMode, 0, 2);
        targetActive.name = sourceActive.skillName;

        const auto& sourceBuff = packet.buffSkills[slot];
        auto& targetBuff = snapshot_.buffSkills[static_cast<std::size_t>(slot)];
        targetBuff.useSlot = sourceBuff.useSlot;
        targetBuff.skillCode = sourceBuff.skillCode;
        targetBuff.point = sourceBuff.point;
        targetBuff.intervalSeconds = sourceBuff.intervalSeconds;
        targetBuff.name = sourceBuff.skillName;
    }
    snapshot_.entryCount = std::clamp(packet.entryCount, 0, static_cast<int>(CompanionUiMaxCharacters));

    for (int i = 0; i < snapshot_.entryCount; ++i)
    {
        const auto& source = packet.entries[i];
        auto& target = snapshot_.entries[static_cast<std::size_t>(i)];
        target.name = source.characterName;
        target.level = source.level;
        target.jobCode = source.jobCode;
        target.active = source.active != 0;
        target.selected = source.selected != 0;
        target.currentCharacter = source.currentCharacter != 0;
        target.rightItemCode = source.rightItemCode;
        target.leftItemCode = source.leftItemCode;
        target.rightItemName = source.rightItemName;
        target.leftItemName = source.leftItemName;
        target.life = source.life;
        target.maxLife = source.maxLife;
        target.mana = source.mana;
        target.maxMana = source.maxMana;
        target.dead = source.dead != 0;
        target.targetName = source.targetName;
        target.mode = static_cast<CompanionUiMode>(std::clamp(source.mode, 0, 2));
        target.tactic = static_cast<CompanionUiTactic>(std::clamp(source.tactic, 0, 3));
        target.retreatHpPercent = std::clamp(source.retreatHpPercent, 0, 90);
        target.preferredRange = std::clamp(source.preferredRange, 0, 600);
    }
}

void LegacyCompanionGateway::ApplyInventoryPage(
    const gameplay::companions::protocol::CompanionInventoryPagePacket& packet)
{
    using namespace gameplay::companions::protocol;
    if (packet.size != static_cast<std::int32_t>(sizeof(packet)) ||
        packet.code != OPCODE_COMPANION_INVENTORY_PAGE ||
        packet.version != COMPANION_INVENTORY_PROTOCOL_VERSION ||
        packet.itemCount < 0 || packet.itemCount > COMPANION_INVENTORY_PAGE_SIZE ||
        packet.page < 0 || packet.totalItems < 0 ||
        packet.page > (packet.totalItems / COMPANION_INVENTORY_PAGE_SIZE))
    {
        summit::companion::diagnostic::Trace("CLIENT","bad-inventory-packet",0,
            packet.page,-103,packet.itemCount);
        summit::companion::diagnostic::Errors().Record(-103);
        return;
    }
    // Do not render inventory pages for a companion that is no longer selected.
    const std::string name(packet.characterName,
        CountBoundedChars(packet.characterName, sizeof(packet.characterName)));
    bool selected = false;
    for (int i=0; i<snapshot_.entryCount; ++i)
        if (snapshot_.entries[static_cast<std::size_t>(i)].selected &&
            !snapshot_.entries[static_cast<std::size_t>(i)].currentCharacter &&
            snapshot_.entries[static_cast<std::size_t>(i)].name == name)
            selected = true;
    if (!selected)
    {
        summit::companion::diagnostic::Trace("CLIENT","stale-inventory-page",0,
            packet.page,0);
        return;
    }
    summit::companion::diagnostic::Trace("CLIENT","inventory-page",0,
        packet.page,packet.status,packet.itemCount);
    if(packet.status < 0) summit::companion::diagnostic::Errors().Record(packet.status);
    CompanionInventorySnapshot data{};
    data.received = true;
    data.status = packet.status;
    data.page = packet.page;
    data.totalItems = packet.totalItems;
    data.itemCount = packet.itemCount;
    data.characterName = name;
    for (int i=0; i<packet.itemCount; ++i)
    {
        const auto& source = packet.items[i];
        auto& dest = data.items[static_cast<std::size_t>(i)];
        dest.recordIndex = source.recordIndex;
        dest.position = source.itemPosition;
        dest.code = source.itemCode;
        dest.equipMask = source.equipMask;
        dest.gridX = source.gridX;
        dest.gridY = source.gridY;
        dest.bagNumber = source.bagNumber;
        dest.name.assign(source.itemName, CountBoundedChars(source.itemName, sizeof(source.itemName)));
    }
    inventorySnapshot_ = data;
}

void LegacyCompanionGateway::ApplyTradePage(
    const gameplay::companions::protocol::CompanionTradePagePacket& packet)
{
    using namespace gameplay::companions::protocol;
    if(!gameplay::companions::v2::ValidTradePreviewPage(packet))
        return;
    const std::string name(packet.characterName,
        CountBoundedChars(packet.characterName, sizeof(packet.characterName)));
    bool selected = false;
    for(int i = 0; i < snapshot_.entryCount; ++i)
        if(snapshot_.entries[static_cast<std::size_t>(i)].selected &&
           !snapshot_.entries[static_cast<std::size_t>(i)].currentCharacter &&
           snapshot_.entries[static_cast<std::size_t>(i)].name == name)
            selected = true;
    if(!selected) return;
    CompanionTradePreviewPage next{};
    next.received = true;
    next.status = packet.status;
    next.side = packet.side;
    next.page = packet.page;
    next.totalItems = packet.totalItems;
    next.itemCount = packet.itemCount;
    next.revision = packet.sourceRevision;
    next.characterName = name;
    for(int i = 0; i < packet.itemCount; ++i)
    {
        const auto& item = packet.items[i];
        if(item.index < 0 || !item.itemCode) return;
        auto& dst = next.items[static_cast<std::size_t>(i)];
        dst.index = item.index;
        dst.position = item.position;
        dst.code = item.itemCode;
        dst.key = item.itemKey;
        dst.checksum = item.itemChecksum;
        dst.name.assign(item.name, CountBoundedChars(item.name, sizeof(item.name)));
    }
    tradePages_[packet.side] = next;
}

void LegacyCompanionGateway::ApplyRuntimeState(const gameplay::companions::protocol::CompanionRuntimeStatePacket& packet)
{
    using namespace gameplay::companions::protocol;
    if (packet.version != COMPANION_RUNTIME_PROTOCOL_VERSION)
        return;

    runtimeSnapshot_ = CompanionRuntimeSnapshot{};
    runtimeSnapshot_.valid = true;
    runtimeSnapshot_.entryCount = std::clamp(packet.entryCount, 0, COMPANION_RUNTIME_MAX_ACTIVE);
    for (int i = 0; i < runtimeSnapshot_.entryCount; ++i)
    {
        const auto& source = packet.entries[i];
        auto& target = runtimeSnapshot_.entries[static_cast<std::size_t>(i)];
        target.name = source.characterName;
        target.objectSerial = source.objectSerial;
        target.level = source.level;
        target.jobCode = source.jobCode;
        target.life = source.life;
        target.maxLife = source.maxLife;
        target.mana = source.mana;
        target.maxMana = source.maxMana;
        target.dead = source.dead != 0;
        target.mode = static_cast<CompanionUiMode>(std::clamp(source.mode, 0, 2));
        target.tactic = static_cast<CompanionUiTactic>(std::clamp(source.tactic, 0, 3));
        target.casting = source.casting != 0;
        target.castingSkillCode = source.castingSkillCode;
        target.castingSkillName = source.castingSkillName;
        target.castProgressPermille = std::clamp(source.castProgressPermille, 0, 1000);
        target.cooldownMs = std::max(0, source.cooldownMs);
        target.statusFlags = source.statusFlags;
        target.effectCount = std::clamp(source.effectCount, 0, COMPANION_RUNTIME_MAX_EFFECTS);
        for(int effect = 0; effect < target.effectCount; ++effect)
        {
            target.effects[static_cast<std::size_t>(effect)].effectCode = source.effects[effect].effectCode;
            target.effects[static_cast<std::size_t>(effect)].name = StableCompanionSkillName(source.effects[effect].effectCode);
            target.effects[static_cast<std::size_t>(effect)].remainingMs = std::max(0, source.effects[effect].remainingMs);
            target.effects[static_cast<std::size_t>(effect)].positive = source.effects[effect].positive != 0;
        }
        target.targetName = source.targetName;
    }
}

void LegacyCompanionGateway::Reset()
{
    snapshot_ = CompanionUiSnapshot{};
    runtimeSnapshot_ = CompanionRuntimeSnapshot{};
    inventorySnapshot_ = CompanionInventorySnapshot{};
    tradePages_[0] = CompanionTradePreviewPage{};
    tradePages_[1] = CompanionTradePreviewPage{};
    tradeAttempt_ = CompanionTradeAttemptResult{};
    // A reset during a committed transaction must not unlock a stale client.
    // Do not thaw while an accepted fresh save / confirm is outstanding.
    if(!tradeCommitted_ && !tradeSavingAfterApply_ && !tradeConfirmSent_ &&
       !tradeSavingBeforeConfirm_ && !tradeSaveScheduled_)
        gameplay::companions::v2::SetTradeClientFrozen(false);
    tradeSaveScheduled_=false;
    tradeSavingBeforeConfirm_=false;
    pendingTradePlayerImage_.clear();
    tradeReceiver_.Reset();
    tradeLiveAwaitingServer350Q_=false;
    tradeLivePostSaveScheduled350Q_=false;
}

void ApplyLegacyCompanionStatePacket(const gameplay::companions::protocol::CompanionUIStatePacket& packet)
{
    GetLegacyCompanionGateway().ApplyState(packet);
}

void ApplyLegacyCompanionTradePagePacket(const gameplay::companions::protocol::CompanionTradePagePacket& packet)
{
    GetLegacyCompanionGateway().ApplyTradePage(packet);
}

void ApplyLegacyCompanionRuntimePacket(const gameplay::companions::protocol::CompanionRuntimeStatePacket& packet)
{
    GetLegacyCompanionGateway().ApplyRuntimeState(packet);
}

void ApplyLegacyCompanionInventoryPacket(const gameplay::companions::protocol::CompanionInventoryPagePacket& packet)
{
    GetLegacyCompanionGateway().ApplyInventoryPage(packet);
}
}

namespace ui::v2 {
void ApplyLegacyCompanionTradeResultPacket(
    const gameplay::companions::protocol::CompanionTradeResultPacket& packet)
{
    GetLegacyCompanionGateway().ApplyTradeResult(packet);
}
}
