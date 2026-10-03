#!/usr/bin/env python3
from pathlib import Path
import re
import sqlite3
import sys

ROOT = Path(__file__).resolve().parents[1]

def fail(message):
    print(f"Covenant V1 complete validation: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)

def read(rel):
    p = ROOT / rel
    if not p.exists():
        fail(f"missing required file: {rel}")
    return p.read_text(encoding="utf-8", errors="replace")

def require(rel, *needles):
    text = read(rel)
    for needle in needles:
        if needle not in text:
            fail(f"{rel} is missing contract: {needle}")
    return text

# Documentation is part of the contract: do not regress the user-approved
# chat-first/no-Sigil-activation semantics while adding future UI/world surfaces.
require(
    "Shared/gameplay/covenants/v2/README.md",
    "Sigils **do not activate Covenant effects**",
    "Option A", "ICovenantGameplayBridge",
    "Presentation/content extension points",
)

# Product/content contract: five Covenants, cumulative ranks, no Sigil activation gate.
content = require(
    "Database/migrations/040_covenant_default_content.sql",
    "(1,'hunters'", "(2,'gatherers'", "(3,'artisans'", "(4,'fortune'", "(5,'ascendants'",
    "rank benefits are cumulative in content (Option A)",
    "Sigils/items are rewards/collection content and DO NOT activate effects",
)
if content.count("INSERT OR IGNORE INTO ServerDB__ProgressionCovenantRank\n") != 1:
    fail("default Covenant rank seed contract changed unexpectedly")

runtime_sql = require(
    "Database/migrations/041_covenant_runtime_complete.sql",
    "ServerDB__CovenantPlayerState", "ServerDB__CovenantMastery", "ServerDB__CovenantCollection",
    "ServerDB__CovenantChallengeDefinition", "ServerDB__CovenantChallengeProgress",
    "ServerDB__CovenantPowerDefinition", "ServerDB__CovenantPowerState",
    "hunters_mark", "tempt_fate", "focused_training",
)
inventory_sql = require(
    "Database/migrations/042_covenant_inventory.sql",
    "ServerDB__CovenantInventory", "ServerDB__CovenantInventory_ByPlayer",
)
artisan_sql = require(
    "Database/migrations/043_covenant_artisan_mode.sql",
    "ServerDB__CovenantArtisanState", "Mode INTEGER",
)

# Migration is part of the runtime bootstrap, not a manual-only SQL file.
require(
    "SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp",
    "041_covenant_runtime_complete.sql", "042_covenant_inventory.sql",
    "043_covenant_artisan_mode.sql",
)

# Core lifecycle and runtime invariants.
require(
    "Shared/gameplay/covenants/v2/CovenantService.cpp",
    "CovenantService::Activate", "AddRepeatableReputation", "SwitchActive",
)
service = read("Shared/gameplay/covenants/v2/CovenantService.cpp").lower()
if "sigil" in service:
    fail("CovenantService must not use Sigils to activate Covenant effects")

runtime = require(
    "SrcServer/src/systems/gameplay/progression/v2/ProgressionRuntime.cpp",
    "SUMMIT_COVENANT_SWITCH_COOLDOWN_SECONDS",
    "GrantCollectible", "sigil:", "RecordServerCovenantActivity",
    "TryGrantCovenantMastery", "UseServerCovenantPower",
    "GetServerCovenantModifierAggregate", "ApplyServerCovenantModifierValue",
    "GetServerCovenantPowerState",
    "AddServerCovenantInventoryItem", "ConsumeServerCovenantInventoryItem",
    "GetServerCovenantInventory", "RefineServerCovenantTrophy",
    "GetServerCovenantArtisanMode", "SetServerCovenantArtisanMode",
    "DeactivatePowers", "covenantService.Activate(playerKey, previousActive.covenantId)",
)

# Chat is the first shipping player surface.
require(
    "SrcServer/src/systems/gameplay/covenants/v2/CovenantChatCommand.cpp",
    "/covenant list", "/covenant join <name>", "/covenant activate <name>",
    "/covenant challenges [name]", "/covenant mastery", "/covenant sigils", "/covenant power",
    "/covenant survey", "/covenant artisan [off|reforge|forgemaster|master]",
    "kGatheringSurveyorEnabled", "GetServerCovenantArtisanMode", "SetServerCovenantArtisanMode",
    "GetServerCovenantSwitchCooldown", "GetServerCovenantPowerState",
)
require(
    "SrcServer/src/systems/network/v2/legacy/LegacyModernChatIngressAdapter.inl",
    "HandleServerCovenantChatCommand", 'rsCompString("/covenant"',
)
require(
    "Shared/server/legacy_runtime/OnSever_legacy_server.inl",
    "RouteModernChatIngress",
)

# Authoritative gameplay hooks. Keep these very explicit: losing any of these makes
# a Covenant appear configured while silently doing nothing in gameplay.
onserver = require(
    "Shared/server/legacy_runtime/OnSever_legacy_server.inl",
    '"xp.monster_gain"', '"monster.drop_chance"', '"hunt.drop_chance"',
    '"loot.extra_roll_chance"', '"combat.damage_taken"', '"crafting.success"',
    '"vendor.blacksmith_sell_value"',
    "SummitApplyArtisanCraftingOutput", "crafting.reforge_enabled",
    "crafting.forgemaster_enabled", "crafting.master_artisan_enabled",
    "CreateDefItemPerf", "special_material:", "refined_trophy:", "refined_material:",
)
require("SrcServer/src/core/server/legacy_runtime/PartySever.cpp", '"xp.monster_gain"')
require(
    "SrcServer/src/systems/gameplay/events/v2/GameplayEventRuntime.cpp",
    '"monster.kill"', '"hunt.kill"', "event.monsterBoss",
    '"hunt.trophy_chance"', '"trophy:"', "AddServerCovenantInventoryItem",
)
require(
    "SrcServer/src/systems/gameplay/events/v2/legacy/LegacyGameplayEventAdapter.cpp",
    "MONSTER_CLASS_BOSS", "const bool isBoss", "MonsterKilledEvent event",
)
require(
    "Shared/gameplay/covenants/v2/ICovenantGameplayBridge.h",
    "ApplyModifierValue", "GetModifierAggregate", "AddInventoryItem", "RecordActivity",
)
require(
    "SrcServer/src/systems/gameplay/mining/v2/MiningRuntime.cpp",
    "ServerMiningCovenantBridge", "ApplyServerCovenantModifierValue",
    "RecordServerCovenantActivity",
)
require(
    "SrcServer/src/systems/gameplay/mining/v2/MiningService.cpp",
    '"gathering.yield"', '"gathering.extra_yield_chance"', '"gathering.harvest"',
    '"gathering.special_material_chance"', '"gathering.refined_material_chance"',
    '"special_material:"', '"refined_material:"',
)
require(
    "SrcServer/src/systems/gameplay/mining/v2/MiningService.cpp",
    "ListResources", "ListNodes",
)
require(
    "SrcServer/src/systems/gameplay/gathering/v2/GatheringService.cpp",
    "GatheringService::ListResources", "GatheringService::ListNodes",
)

# Tier III effects are content-driven, not if(covenant==X) balance constants in hot paths.
for hotpath in [
    "Shared/server/legacy_runtime/OnSever_legacy_server.inl",
    "SrcServer/src/core/server/legacy_runtime/PartySever.cpp",
    "SrcServer/src/systems/gameplay/mining/v2/MiningService.cpp",
]:
    text = read(hotpath)
    for forbidden in ["hunters_mark", "tempt_fate", "focused_training"]:
        if forbidden in text:
            fail(f"{hotpath} hardcodes Tier III power {forbidden}")

# Lightweight SQL checks over the two Covenant content migrations themselves.
db = sqlite3.connect(":memory:")
db.executescript("""
CREATE TABLE ServerDB__ProgressionCovenantDefinition(CovenantId INTEGER PRIMARY KEY,Key TEXT,DisplayNameKey TEXT,ExclusiveMembership INTEGER,HasReputationBounds INTEGER,MinReputation INTEGER,MaxReputation INTEGER,Enabled INTEGER);
CREATE TABLE ServerDB__ProgressionCovenantJoinCondition(CovenantId INTEGER,SortOrder INTEGER,ConditionType INTEGER,Key TEXT,Id INTEGER,Value INTEGER,AuxiliaryId INTEGER,PRIMARY KEY(CovenantId,SortOrder));
CREATE TABLE ServerDB__ProgressionCovenantRank(CovenantId INTEGER,RankId INTEGER,Key TEXT,DisplayNameKey TEXT,MinReputation INTEGER,PRIMARY KEY(CovenantId,RankId));
CREATE TABLE ServerDB__ProgressionCovenantRankBenefit(CovenantId INTEGER,RankId INTEGER,SortOrder INTEGER,StatKey TEXT,Operation INTEGER,Value REAL,ScopeKey TEXT,PRIMARY KEY(CovenantId,RankId,SortOrder));
CREATE TABLE ServerDB__CovenantMembership(PlayerKey TEXT,CovenantId INTEGER,Reputation INTEGER,RankId INTEGER,Active INTEGER,PRIMARY KEY(PlayerKey,CovenantId));
CREATE TABLE ServerDB__CovenantCollection(PlayerKey TEXT,CovenantId INTEGER,CollectibleKey TEXT,AcquiredAt INTEGER,PRIMARY KEY(PlayerKey,CovenantId,CollectibleKey));
""")
db.executescript(content)
db.executescript(runtime_sql)
db.executescript(inventory_sql)
db.executescript(artisan_sql)
counts = {
    "covenants": db.execute("SELECT COUNT(*) FROM ServerDB__ProgressionCovenantDefinition").fetchone()[0],
    "ranks": db.execute("SELECT COUNT(*) FROM ServerDB__ProgressionCovenantRank").fetchone()[0],
    "benefits": db.execute("SELECT COUNT(*) FROM ServerDB__ProgressionCovenantRankBenefit").fetchone()[0],
    "activities": db.execute("SELECT COUNT(*) FROM ServerDB__CovenantActivityDefinition WHERE Enabled=1").fetchone()[0],
    "challenges": db.execute("SELECT COUNT(*) FROM ServerDB__CovenantChallengeDefinition WHERE Enabled=1").fetchone()[0],
    "powers": db.execute("SELECT COUNT(*) FROM ServerDB__CovenantPowerDefinition WHERE Enabled=1").fetchone()[0],
}
expected = {"covenants":5,"ranks":15,"benefits":52,"activities":5,"challenges":15,"powers":3}
if counts != expected:
    fail(f"unexpected content counts: got {counts}, expected {expected}")

# Option A: each later rank must contain the full semantic set of earlier ranks.
for covenant_id in range(1, 6):
    prior = set()
    for rank_id in range(1, 4):
        current = set(db.execute(
            "SELECT StatKey,ScopeKey,Operation FROM ServerDB__ProgressionCovenantRankBenefit WHERE CovenantId=? AND RankId=?",
            (covenant_id, rank_id),
        ).fetchall())
        if not prior.issubset(current):
            fail(f"Covenant {covenant_id} rank {rank_id} violates cumulative Option A")
        prior = current

db.close()
print("Covenant V1 complete integration validation: PASS")
