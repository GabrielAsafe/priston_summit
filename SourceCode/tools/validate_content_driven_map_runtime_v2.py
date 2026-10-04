#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]


def text(rel: str) -> str:
    return (ROOT / rel).read_text(encoding="utf-8", errors="ignore")


def require(cond: bool, message: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {message}")

adapter = text("Shared/content/v1/map/legacy/LegacyMapAdapter.h")
compiler = text("tools/content_port/compile_maps.py")
schema = text("tools/content_port/schemas/map_catalog.schema.json")
server_field = text("Shared/world/field/field_legacy_server.inl")
client_field = text("Shared/world/field/field_legacy_client.inl")
onserver = text("Shared/server/legacy_runtime/OnSever_legacy_server.inl")
onserver_h = text("SrcServer/src/core/server/legacy_runtime/onserver.h")
client_onserver = text("Shared/server/legacy_runtime/OnSever_legacy_client.inl")
client_onserver_h = text("Shared/server/legacy_runtime/onserver_legacy_client.h")

require("expectedDynamicFieldCode" not in adapter and "expectedStaticFieldCount" not in adapter,
        "LegacyMapAdapter still requires caller-owned field counts")
require("outStaticFieldCount" in adapter and "outDynamicFieldCode" in adapter,
        "LegacyMapAdapter does not return pack-owned field counts")
require("dynamicFieldCode!=static_cast<std::int32_t>(catalog.staticFieldCount)" in adapter,
        "runtime does not enforce dynamic field == first field after static range")
require("expected_codes = set(range(static_field_count))" in compiler,
        "map compiler does not enforce contiguous field codes")
require("dynamic_field_code = static_field_count" in compiler,
        "map compiler does not derive the custom field from static count")
require('"maximum": 255' in schema and '"maximum": 254' in schema,
        "map schema does not reserve one FIELD_MAX slot for the custom field")

for side, src in (("client", client_field), ("server", server_field)):
    require("paths, 53, 53" not in src, f"{side} runtime still hardcodes 53/53 loader contract")
    require("FinalizeLegacyFieldCatalog(53)" not in src, f"{side} runtime still finalizes at field 53")
    require("&staticFieldCount, &dynamicFieldCode" in src, f"{side} runtime does not consume pack counts")
    require("FinalizeLegacyFieldCatalog(static_cast<int>(staticFieldCount))" in src,
            f"{side} custom field is not appended after static content")
    require("cnt >= FIELD_MAX" in src, f"{side} custom field lacks FIELD_MAX bounds protection")

require("STAGE_AREA_MAX" not in onserver_h and "STAGE_AREA_MAX" not in client_onserver_h,
        "fixed 64-stage capacity define still exists in active runtime headers")
require("extern int StageAreaCount;" in onserver_h, "server StageAreaCount is not exported")
require("int StageAreaCount = 0;" in onserver, "server StageAreaCount storage is missing")
require("StageAreaCount = FieldCount;" in onserver, "server StageAreaCount is not derived from loaded fields")
require("StageArea = new STG_AREA[StageAreaCount];" in onserver,
        "server StageArea is not dynamically sized from FieldCount")
require("extern int StageAreaCount;" in client_onserver_h, "client compatibility StageAreaCount is not exported")
require("int StageAreaCount = 0;" in client_onserver, "client compatibility StageAreaCount storage is missing")
require("StageAreaCount = FieldCount;" in client_onserver, "client compatibility StageAreaCount is not derived from loaded fields")
require("StageArea = new STG_AREA[StageAreaCount];" in client_onserver,
        "client compatibility StageArea is not dynamically sized from FieldCount")
require("StageArea = new STG_AREA[STAGE_AREA_MAX]" not in "\n".join(
            line for line in onserver.splitlines() if not line.lstrip().startswith("//")),
        "active server runtime still allocates StageArea[STAGE_AREA_MAX]")
require("for(int cnt = 0; cnt < StageAreaCount; ++cnt)" in onserver,
        "srReadStage does not iterate exact loaded field count")

live_stage_files = [
    "Shared/server/legacy_runtime/OnSever_legacy_server.inl",
    "Shared/gameplay/combat/Svr_Damge_legacy_server.inl",
    "SrcServer/src/systems/gameplay/world/v2/legacy/LegacyStageAdapter.cpp",
    "SrcServer/src/systems/gameplay/world/v2/legacy/LegacyWorldAdapter.cpp",
    "SrcServer/src/systems/gameplay/loot/v2/legacy/LegacyLootAdapter.cpp",
    "SrcServer/src/systems/gameplay/character/core/character.cpp",
    "SrcServer/src/systems/gameplay/events/server_legacy/SvrCaravan.cpp",
    "SrcServer/src/systems/gameplay/events/server_legacy/svrEventSpawn.cpp",
    "Shared/server/legacy_runtime/OnSever_legacy_client.inl",
    "Shared/gameplay/combat/Svr_Damge_legacy_client.inl",
    "SrcGame/src/compat/client_server/server_legacy/svrEventSpawn.cpp",
]
for rel in live_stage_files:
    src = text(rel)
    active = "\n".join(line for line in src.splitlines() if not line.lstrip().startswith("//"))
    require("< STAGE_AREA_MAX" not in active and "> STAGE_AREA_MAX" not in active and ">= STAGE_AREA_MAX" not in active,
            f"{rel} still uses STAGE_AREA_MAX as a live StageArea bound")

spawn_h = text("SrcServer/src/systems/gameplay/events/server_legacy/svrEventSpawn.h")
spawn_cpp = text("SrcServer/src/systems/gameplay/events/server_legacy/svrEventSpawn.cpp")
client_spawn_h = text("SrcGame/src/compat/client_server/server_legacy/svrEventSpawn.h")
client_spawn_cpp = text("SrcGame/src/compat/client_server/server_legacy/svrEventSpawn.cpp")
require("rsEventSpawn[FIELD_MAX]" in spawn_h and "rsEventSpawn[FIELD_MAX]" in spawn_cpp,
        "server event-spawn auxiliary table is still capped at 64")
require("rsEventSpawn[FIELD_MAX]" in client_spawn_h and "rsEventSpawn[FIELD_MAX]" in client_spawn_cpp,
        "client compatibility event-spawn auxiliary table is still capped at 64")

print("content-driven map runtime validator: PASS")
