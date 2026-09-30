#!/usr/bin/env python3
"""Boundary guard for the playable-class V2 migration.

This test does not try to outlaw job-specific gameplay. Skill handlers and quest
content may legitimately target a specific class. It protects the *generic*
character/class integration surfaces from regressing to numeric JOB_CODE ranges
or per-class switch tables.
"""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[5]

FAILURES = []

# Numeric ranges are a legacy encoding of brood/faction. A new class must get
# this behavior from ClassDefinition::brood, never from its numeric JOB_CODE.
NUMERIC_BROOD_PATTERNS = [
    re.compile(r"\b(?:JOB_CODE|jobCode|JobCode)\s*<\s*5"),
    re.compile(r"\b(?:JOB_CODE|jobCode|JobCode)\s*<=\s*4"),
    re.compile(r"\b(?:JOB_CODE|jobCode|JobCode)\s*>\s*4"),
    re.compile(r"\b(?:JOB_CODE|jobCode|JobCode)\s*==\s*(?:9|10)\b"),
    re.compile(r"\b(?:JOB_CODE|jobCode|JobCode)\s*!=\s*9\b"),
]

SCAN_ROOTS = [ROOT / "Shared", ROOT / "SrcGame", ROOT / "SrcServer"]
SKIP_PARTS = {"legacy_oracle", ".git"}
SKIP_SUFFIXES = {".orig", ".before", ".bak"}

for scan_root in SCAN_ROOTS:
    for path in scan_root.rglob("*"):
        if not path.is_file() or any(part in SKIP_PARTS for part in path.parts):
            continue
        if any(path.name.endswith(suffix) for suffix in SKIP_SUFFIXES):
            continue
        if path.suffix.lower() not in {".cpp", ".h", ".hpp", ".inl"}:
            continue
        try:
            text = path.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            continue
        for lineno, line in enumerate(text.splitlines(), 1):
            stripped = line.lstrip()
            if stripped.startswith("//"):
                continue
            for pattern in NUMERIC_BROOD_PATTERNS:
                if pattern.search(line):
                    FAILURES.append(
                        f"{path.relative_to(ROOT)}:{lineno}: numeric brood inference: {line.strip()}"
                    )

# These files are generic class/character infrastructure. Concrete class names,
# JobDataBase and ArmorJobNum must remain behind adapters/content registries.
CLASS_NEUTRAL_FILES = [
    "Shared/authentication/character_selection/HoLogin_legacy_client.inl",
    "SrcServer/src/systems/authentication/character_selection/HoLogin.cpp",
    "Shared/authentication/character_selection/ClassSelectionAdapter.h",
    "Shared/gameplay/legacy_support/PlaySubHairSkinLegacyCommon.inl",
    "Shared/gameplay/legacy_support/PlaySubHairModelLegacyCommon.inl",
    "Shared/world/field/field_legacy_client.inl",
    "Shared/world/field/field_legacy_server.inl",
    "Shared/persistence/character_record/record_legacy_client.inl",
    "Shared/persistence/character_record/record_legacy_server.inl",
    "SrcGame/src/systems/ui/help/legacy_sin/sinHelp.cpp",
    "SrcServer/src/systems/ui/help/legacy_sin/sinHelp.cpp",
    "SrcGame/src/systems/ui/help/legacy_sin/sinHelpPet.cpp",
    "SrcServer/src/systems/ui/help/legacy_sin/sinHelpPet.cpp",
    "SrcGame/src/systems/ui/messages/legacy_sin/sinMessageBox.cpp",
    "SrcServer/src/systems/ui/messages/legacy_sin/sinMessageBox.cpp",
    "SrcGame/src/systems/gameplay/loot/filter/CLootHandler.cpp",
    "SrcGame/src/systems/inventory/legacy/sinItem.cpp",
    "Shared/inventory/legacy/sinItem_legacy_server.inl",
    "SrcGame/src/core/data/legacy_loader/fileread.cpp",
    "SrcServer/src/core/data/legacy_loader/fileread.cpp",
    "Shared/formats/item_config/ItemDefinitionReader.inl",
    "Shared/formats/model_config/LegacyModelConfigExact_client.inl",
    "Shared/formats/model_config/LegacyModelConfigExact_server.inl",
    "SrcGame/src/systems/social/party/ui/cPartyWindow.cpp",
    "SrcGame/src/systems/progression/rankings/general/Ranking.cpp",
    "SrcGame/src/systems/combat/pvp/kill_notifications/KillHandler.cpp",
]

CONCRETE_CLASS = re.compile(
    r"\bJOBCODE_(?:FIGHTER|MECHANICIAN|ARCHER|PIKEMAN|ATALANTA|KNIGHT|MAGICIAN|PRIESTESS|ASSASSIN|SHAMAN)\b"
)
FORBIDDEN_NEUTRAL = [
    (CONCRETE_CLASS, "concrete class symbol"),
    (re.compile(r"\bJobDataBase\b"), "legacy JobDataBase dependency"),
    (re.compile(r"\bArmorJobNum\b"), "legacy ArmorJobNum dependency"),
    (re.compile(r"\bBitMaskJobNames\b"), "legacy motion-job table dependency"),
    (re.compile(r"\bszClasseList\b"), "legacy fixed class-title table dependency"),
    (re.compile(r"\bimgClasse\b"), "legacy fixed ranking class-icon table dependency"),
]


# Historical random-box class kits are content. The server runtime may invoke
# the adapter, but CaixaItensRandom itself must not enumerate concrete classes.
RANDOM_BOX_RUNTIME = ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl"
if RANDOM_BOX_RUNTIME.exists():
    text = RANDOM_BOX_RUNTIME.read_text(encoding="utf-8", errors="ignore")
    start = text.find("int CaixaItensRandom")
    end = text.find("DWORD rsLowLevelItem", start) if start >= 0 else -1
    if start < 0 or end < 0:
        FAILURES.append("OnSever_legacy_server.inl: unable to locate CaixaItensRandom guard region")
    else:
        section = text[start:end]
        for offset, line in enumerate(section.splitlines(), 1):
            if CONCRETE_CLASS.search(line) and not line.lstrip().startswith("//"):
                FAILURES.append(
                    f"OnSever_legacy_server.inl:CaixaItensRandom+{offset}: class-specific event kit escaped adapter"
                )


# Concrete JOBCODE_* symbols are now confined to protocol declarations, the
# canonical class-content catalog and legacy skill/effect implementation files.
# A new occurrence elsewhere is almost certainly a class-system regression and
# must be routed through ClassService or an explicit content adapter instead.
CONCRETE_JOB_ALLOWED_FILES = {
    "Shared/smPacket.h",
    "Shared/gameplay/character/character_legacy_client.inl",
    "Shared/gameplay/combat/LegacyDamagePolicy.inl",
    "Shared/gameplay/combat/Svr_Damge_legacy_client.inl",
    "Shared/gameplay/combat/Svr_Damge_legacy_server.inl",
    "Shared/gameplay/skills/LegacySkillCommon.inl",
    "Shared/gameplay/skills/Tier5SinSkillClient.inl",
    "Shared/gameplay/skills/sinSkill_legacy_client.inl",
    "Shared/gameplay/skills/sinSkill_legacy_server.inl",
    "Shared/server/legacy_runtime/OnSever_legacy_client.inl",
    "Shared/server/legacy_runtime/OnSever_legacy_server.inl",
    "SrcGame/src/systems/combat/damage/Damage.cpp",
    "SrcGame/src/systems/gameplay/character/status_ui/sinCharStatus.cpp",
    "SrcGame/src/systems/inventory/legacy/sinInvenTory.cpp",
    "SrcGame/src/systems/network/client/netplay.cpp",
    "SrcServer/src/systems/gameplay/character/core/character.cpp",
    # Canonical extended skill metadata is itself a legacy skill-content boundary;
    # concrete owner-class identities are the payload it translates.
    "Shared/gameplay/skills/v2/legacy/LegacyExtendedSkillMetadata.inl",
}
for scan_root in SCAN_ROOTS:
    for path in scan_root.rglob("*"):
        if not path.is_file() or path.suffix.lower() not in {".cpp", ".h", ".hpp", ".inl"}:
            continue
        relative = str(path.relative_to(ROOT)).replace("\\", "/")
        if any(part in SKIP_PARTS for part in path.parts):
            continue
        if relative.startswith("Shared/gameplay/classes/v2/content/"):
            continue
        if "/legacy/" in relative and relative.endswith("Adapter.h"):
            continue
        if relative in CONCRETE_JOB_ALLOWED_FILES:
            continue
        text = path.read_text(encoding="utf-8", errors="ignore")
        for lineno, line in enumerate(text.splitlines(), 1):
            if CONCRETE_CLASS.search(line) and not line.lstrip().startswith("//"):
                FAILURES.append(
                    f"{relative}:{lineno}: new concrete JOBCODE symbol outside class content/skill boundary"
                )

# Runtime composition must go through the validated source boundary. The
# compatibility aggregate may exist, but generic runtime code must not start
# constructing it or manually registering source payloads.
DIRECT_COMPAT_SOURCE = re.compile(r"\bLegacyClassContentSource\b")
COMPAT_SOURCE_ALLOWED = {
    "Shared/gameplay/classes/v2/content/LegacyClassCatalog.h",
    "Shared/gameplay/classes/v2/content/LegacyClassCatalog.cpp",
}
for scan_root in SCAN_ROOTS:
    for path in scan_root.rglob("*"):
        if not path.is_file() or path.suffix.lower() not in {".cpp", ".h", ".hpp", ".inl"}:
            continue
        relative = str(path.relative_to(ROOT)).replace("\\", "/")
        if relative in COMPAT_SOURCE_ALLOWED or any(part in SKIP_PARTS for part in path.parts):
            continue
        text = path.read_text(encoding="utf-8", errors="ignore")
        for lineno, line in enumerate(text.splitlines(), 1):
            if DIRECT_COMPAT_SOURCE.search(line) and not line.lstrip().startswith("//"):
                FAILURES.append(f"{relative}:{lineno}: direct compatibility class source dependency")

for relative in CLASS_NEUTRAL_FILES:
    path = ROOT / relative
    if not path.exists():
        FAILURES.append(f"missing guarded file: {relative}")
        continue
    text = path.read_text(encoding="utf-8", errors="ignore")
    for lineno, line in enumerate(text.splitlines(), 1):
        stripped = line.lstrip()
        if stripped.startswith("//"):
            continue
        for pattern, label in FORBIDDEN_NEUTRAL:
            if pattern.search(line):
                FAILURES.append(f"{relative}:{lineno}: {label}: {line.strip()}")


# Presentation-only consumers must not depend on the full class-definition
# object graph just to enumerate enabled jobs. This keeps UI surfaces coupled
# to narrow ClassService queries rather than domain storage details.
PRESENTATION_JOB_ENUMERATION_FILES = [
    "SrcGame/src/systems/combat/pvp/kill_notifications/KillHandler.cpp",
    "SrcGame/src/systems/social/party/ui/cPartyWindow.cpp",
    "SrcGame/src/systems/ui/help/legacy_sin/sinHelp.cpp",
    "SrcServer/src/systems/ui/help/legacy_sin/sinHelp.cpp",
    "SrcGame/src/systems/progression/rankings/general/Ranking.cpp",
]
for relative in PRESENTATION_JOB_ENUMERATION_FILES:
    path = ROOT / relative
    if not path.exists():
        FAILURES.append(f"missing presentation guard file: {relative}")
        continue
    text = path.read_text(encoding="utf-8", errors="ignore")
    for lineno, line in enumerate(text.splitlines(), 1):
        if "EnabledClasses()" in line and not line.lstrip().startswith("//"):
            FAILURES.append(
                f"{relative}:{lineno}: presentation consumer leaks full class definitions; use EnabledJobCodes()"
            )

# JOB_CODE is a protocol identity, not an array index. The only allowed job-1
# derivations live in the legacy content catalog where old fixed-table row IDs
# are explicitly captured as compatibility metadata.
JOB_MINUS_ONE = re.compile(r"\b(?:JOB_CODE|JobCode|jobCode)\s*-\s*1\b")
for scan_root in SCAN_ROOTS:
    for path in scan_root.rglob("*"):
        if not path.is_file() or path.suffix.lower() not in {".cpp", ".h", ".hpp", ".inl"}:
            continue
        relative = str(path.relative_to(ROOT)).replace("\\", "/")
        if relative == "Shared/gameplay/classes/v2/content/LegacyClassCatalog.cpp" or \
           any(part in SKIP_PARTS for part in path.parts) or \
           any(path.name.endswith(suffix) for suffix in SKIP_SUFFIXES):
            continue
        text = path.read_text(encoding="utf-8", errors="ignore")
        for lineno, line in enumerate(text.splitlines(), 1):
            if JOB_MINUS_ONE.search(line) and not line.lstrip().startswith("//"):
                FAILURES.append(f"{relative}:{lineno}: JOB_CODE used as zero-based array index")

# Motion selection has its own legacy mask protocol. It must never reuse the
# item-specialization JobBitCode simply because both values are bitmasks.
MOTION_RUNTIME_FILES = [
    "Shared/gameplay/character/character_legacy_client.inl",
    "SrcServer/src/systems/gameplay/character/core/character.cpp",
]
WRONG_MOTION_MASK = re.compile(r"TryJobBitCode\s*\([^\n]*JOB_CODE[^\n]*dwJobCodeMask")
for relative in MOTION_RUNTIME_FILES:
    path = ROOT / relative
    text = path.read_text(encoding="utf-8", errors="ignore")
    for lineno, line in enumerate(text.splitlines(), 1):
        if WRONG_MOTION_MASK.search(line):
            FAILURES.append(f"{relative}:{lineno}: item specialization bit reused as motion mask")

# MAX_JOBNUM is retained only as a compatibility alias for the historical
# eight-class skill table. Generic systems must not use it as the registry size.
MAX_JOBNUM_ALLOWED = {
    "Shared/smPacket.h",
}
for scan_root in SCAN_ROOTS:
    for path in scan_root.rglob("*"):
        if not path.is_file() or path.suffix.lower() not in {".cpp", ".h", ".hpp", ".inl"}:
            continue
        relative = str(path.relative_to(ROOT)).replace("\\", "/")
        if relative in MAX_JOBNUM_ALLOWED or any(part in SKIP_PARTS for part in path.parts):
            continue
        text = path.read_text(encoding="utf-8", errors="ignore")
        for lineno, line in enumerate(text.splitlines(), 1):
            if "MAX_JOBNUM" in line and not line.lstrip().startswith("//"):
                FAILURES.append(f"{relative}:{lineno}: MAX_JOBNUM used as runtime class count")

# HoLogin capacity is a UI/storage limit, not the number of classes in the game.
# Keep the historical aliases declared for compatibility, but new/active code
# must use the semantic capacity names.
SELECTION_ALIAS_ALLOWED = {
    "Shared/authentication/character_selection/HoLoginLegacyShared.h",
}
for scan_root in SCAN_ROOTS:
    for path in scan_root.rglob("*"):
        if not path.is_file() or path.suffix.lower() not in {".cpp", ".h", ".hpp", ".inl"}:
            continue
        relative = str(path.relative_to(ROOT)).replace("\\", "/")
        if relative in SELECTION_ALIAS_ALLOWED or any(part in SKIP_PARTS for part in path.parts):
            continue
        text = path.read_text(encoding="utf-8", errors="ignore")
        for lineno, line in enumerate(text.splitlines(), 1):
            if ("CHARACTER_JOB_MAX" in line or "CHARACTER_FACE_MAX" in line) and not line.lstrip().startswith("//"):
                FAILURES.append(f"{relative}:{lineno}: deprecated character-selection capacity alias used")

# The V2 class domain may depend on itself and the C++ standard library, but it
# must not start reaching into inventory/UI/quest/render legacy systems.
V2_ROOT = ROOT / "Shared/gameplay/classes/v2"
BAD_INCLUDE = re.compile(r'^\s*#include\s+["<]([^">]+)[">]')
for path in V2_ROOT.rglob("*"):
    if not path.is_file() or path.suffix.lower() not in {".cpp", ".h", ".hpp", ".inl"}:
        continue
    if "tests" in path.parts:
        continue
    text = path.read_text(encoding="utf-8", errors="ignore")
    for lineno, line in enumerate(text.splitlines(), 1):
        match = BAD_INCLUDE.match(line)
        if not match:
            continue
        header = match.group(1)
        if header.startswith("gameplay/classes/v2/"):
            continue
        if "/" not in header:  # standard library / C library
            continue
        FAILURES.append(
            f"{path.relative_to(ROOT)}:{lineno}: class-domain dependency points outward: {header}"
        )

# Concrete content composition is an application concern. Registry/service
# implementation must stay reusable with arbitrary sources.
for relative in [
    "Shared/gameplay/classes/v2/PlayableClassRegistry.cpp",
    "Shared/gameplay/classes/v2/services/ClassService.cpp",
]:
    path = ROOT / relative
    text = path.read_text(encoding="utf-8", errors="ignore")
    for lineno, line in enumerate(text.splitlines(), 1):
        if "LegacyClassCatalog.h" in line or "ReservedClassCatalog.h" in line:
            FAILURES.append(f"{relative}:{lineno}: registry/service depends on concrete class content")

# Generic engine consumers query ClassService. Direct access to the default
# registry is reserved for the composition root, ClassService construction and
# tests that validate the registry contract.
DEFAULT_REGISTRY_ALLOWED = {
    "Shared/gameplay/classes/v2/content/DefaultClassContent.cpp",
    "Shared/gameplay/classes/v2/content/DefaultClassContent.h",
    "Shared/gameplay/classes/v2/services/ClassService.cpp",
    "Shared/gameplay/classes/v2/tests/playable_class_registry_smoke.cpp",
}
for scan_root in SCAN_ROOTS:
    for path in scan_root.rglob("*"):
        if not path.is_file() or path.suffix.lower() not in {".cpp", ".h", ".hpp", ".inl"}:
            continue
        relative = str(path.relative_to(ROOT)).replace("\\", "/")
        if relative in DEFAULT_REGISTRY_ALLOWED or any(part in SKIP_PARTS for part in path.parts):
            continue
        text = path.read_text(encoding="utf-8", errors="ignore")
        for lineno, line in enumerate(text.splitlines(), 1):
            if "DefaultPlayableClassRegistry" in line and not line.lstrip().startswith("//"):
                FAILURES.append(f"{relative}:{lineno}: generic consumer bypasses ClassService")


# Every compiled implementation unit in the class subsystem must be linked by
# both client and server manifests. This prevents portable tests from passing
# while the native MinGW link silently omits a new Shared .cpp.
CLASS_CPP_FILES = sorted(
    str(path.relative_to(ROOT)).replace("\\", "/")
    for path in V2_ROOT.rglob("*.cpp")
    if "tests" not in path.parts
)
for manifest_relative in [
    "build_client_mingw_i686_multicore.sh",
    "build_server_mingw_i686_j8.sh",
]:
    manifest = ROOT / manifest_relative
    if not manifest.exists():
        FAILURES.append(f"missing build manifest: {manifest_relative}")
        continue
    manifest_text = manifest.read_text(encoding="utf-8", errors="ignore")
    for implementation in CLASS_CPP_FILES:
        if implementation not in manifest_text:
            FAILURES.append(
                f"{manifest_relative}: missing playable-class implementation: {implementation}"
            )

# OpenItem parser identity must resolve through ClassService. The historical
# JobDataBase rows may still back localized presentation bridges, but they must
# never become parser identity again.
ITEM_IDENTITY_FILES = [
    "Shared/formats/item_config/LegacyItemDefinitionExact_client.inl",
    "Shared/formats/item_config/LegacyItemDefinitionExact_server.inl",
    "Shared/formats/legacy_loader/fileread_legacy_client.inl",
    "Shared/formats/legacy_loader/fileread_legacy_server.inl",
]
for relative in ITEM_IDENTITY_FILES:
    path = ROOT / relative
    text = path.read_text(encoding="utf-8", errors="ignore")
    # Only the item-definition parser section is relevant in the legacy-loader
    # snapshots; GetJobDataCode farther below may still use localized rows.
    parser_text = text[: text.find("JOB_DATA_CODE *GetJobDataCode") if "JOB_DATA_CODE *GetJobDataCode" in text else len(text)]
    for lineno, line in enumerate(parser_text.splitlines(), 1):
        if "JobDataBase" in line and not line.lstrip().startswith("//"):
            FAILURES.append(f"{relative}:{lineno}: JobDataBase used as item parser identity")

if FAILURES:
    print("Class architecture guard: FAIL", file=sys.stderr)
    for failure in FAILURES:
        print("  - " + failure, file=sys.stderr)
    sys.exit(1)

print("Class architecture guard: PASS")
