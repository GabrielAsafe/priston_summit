#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
DAMAGE = ROOT / "Shared/gameplay/combat/Svr_Damge_legacy_server.inl"
ONSERVER = ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl"
CHARACTER = ROOT / "SrcServer/src/systems/gameplay/character/core/character.cpp"
MONSTER_ADAPTER = ROOT / "SrcServer/src/systems/gameplay/monsters/v2/legacy/LegacyMonsterAdapter.cpp"
TARGETING_ADAPTER = ROOT / "SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyTargetingAdapter.cpp"
MONSTER_CONTRACT = ROOT / "Shared/gameplay/monsters/v2/MonsterCommands.h"
MONSTER_QUERY = ROOT / "Shared/gameplay/monsters/v2/MonsterQuery.h"
SUMMON_CONTRACT = ROOT / "Shared/gameplay/monsters/v2/SummonedActorCommands.h"


def read(path: Path) -> str:
    if not path.is_file():
        print(f"monster/npc authority v1: FAIL: missing {path.relative_to(ROOT)}", file=sys.stderr)
        raise SystemExit(1)
    return path.read_text(encoding="utf-8", errors="replace")


def section(text: str, start_token: str, end_token: str) -> str:
    start = text.find(start_token)
    if start < 0:
        raise AssertionError(f"missing section start: {start_token}")
    end = text.find(end_token, start + len(start_token))
    if end < 0:
        raise AssertionError(f"missing section end: {end_token}")
    return text[start:end]


def fail(errors):
    if errors:
        for error in errors:
            print(f"monster/npc authority v1: FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)


def main() -> None:
    damage = read(DAMAGE)
    onserver = read(ONSERVER)
    character = read(CHARACTER)
    monster_adapter = read(MONSTER_ADAPTER)
    targeting_adapter = read(TARGETING_ADAPTER)
    contracts = [read(MONSTER_CONTRACT), read(MONSTER_QUERY), read(SUMMON_CONTRACT)]
    errors = []

    # GREEN contracts stay free of legacy runtime types.
    joined_contracts = "\n".join(contracts)
    for symbol in ("rsPLAYINFO", "smCHAR", "STG_AREA", "smWINSOCK", "windows.h", "onserver.h", "character.h"):
        if symbol in joined_contracts:
            errors.append(f"GREEN monster contract leaks legacy symbol/include: {symbol}")

    # Holy Incantation: success/rng remains in legacy skill policy, but the state transition
    # (owner, brood, life, expiry bookkeeping and target clearing) belongs to IMonsterCommands.
    try:
        holy = section(damage, "case SKILL_PLAY_HOLY_INCANTATION:", "case SKILL_PLAY_GRAND_CROSS:")
    except AssertionError as exc:
        errors.append(str(exc))
        holy = ""
    required_holy = (
        "ServerLegacyMonsterAdapter().ConvertToFollower(",
        "FollowerConversionRequest(",
        "ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo)",
        "if(conversion.Ok())",
        "smCOMMNAD_USER_HOLY_INCANTATION",
    )
    for token in required_holy:
        if token not in holy:
            errors.append(f"Holy Incantation routed token missing: {token}")
    for token in (
        "lpChar->lpMasterPlayInfo = lpPlayInfo",
        "lpChar->smMonsterInfo.dwAutoCharCode = 0",
        "lpChar->PlayHolyIncantation[0] =",
        "lpChar->PlayHolyIncantation[1] =",
        "lpChar->smCharInfo.Life[0] = lpChar->smCharInfo.Life[1]",
        "lpChar->smCharInfo.Next_Exp = lpPlayInfo->dwObjectSerial",
        "lpChar->smCharInfo.Brood = smCHAR_MONSTER_USER",
        "lpChar->lpTargetChar = 0",
        "lpChar->lpTargetPlayInfo = 0",
    ):
        if token in holy:
            errors.append(f"Holy Incantation still mutates follower state directly: {token}")

    for token in (
        "target->lpMasterPlayInfo = owner",
        "target->smMonsterInfo.dwAutoCharCode = 0",
        "target->PlayHolyIncantation[0]",
        "target->PlayHolyIncantation[1]",
        "target->smCharInfo.Life[0] = target->smCharInfo.Life[1]",
        "target->smCharInfo.Next_Exp = owner->dwObjectSerial",
        "target->smCharInfo.Brood = smCHAR_MONSTER_USER",
        "target->lpTargetChar = 0",
        "target->lpTargetPlayInfo = 0",
        "LegacyMonsterAdapter::ReleaseFollower",
    ):
        if token not in monster_adapter:
            errors.append(f"monster adapter parity token missing: {token}")

    # Expiry must also remain delegated to the same owner boundary.
    try:
        expiry = section(character, "if(PlayHolyIncantation[1] > 0)", "if(UseObject_VirtualLife)")
    except AssertionError as exc:
        errors.append(str(exc))
        expiry = ""
    if "ServerLegacyMonsterAdapter().ReleaseFollower(" not in expiry:
        errors.append("Holy Incantation expiry does not delegate to ReleaseFollower")
    for token in ("smCharInfo.Brood = PlayHolyIncantation[0]", "lpMasterPlayInfo = 0"):
        if token in expiry:
            errors.append(f"Holy Incantation expiry still mutates ownership directly: {token}")

    # Two exact legacy target clears have an existing V2 equivalent. Keep them routed instead
    # of leaving special-case monster/NPC target mutation in RED.
    try:
        range_attack = section(damage, "int RecvRangeAttack(", "int RecvRangeAttackUser(")
    except AssertionError as exc:
        errors.append(str(exc))
        range_attack = ""
    if "ServerLegacyTargetingAdapter().ClearPlayerTarget(" not in range_attack:
        errors.append("range-attack monster player-target clear bypasses TargetingService")
    direct_range_player_clears = re.findall(r"lpChar->lpTargetPlayInfo\s*=\s*(?:0|NULL)", range_attack)
    # One clear intentionally remains inside the Compulsion forced-player-target state machine;
    # ClearPlayerTarget alone is not equivalent because that branch also rewires forced-target state.
    if len(direct_range_player_clears) != 1 or "lpChar->lpCompulsionPlayInfo = lpPlayInfo" not in range_attack:
        errors.append(
            f"unexpected range-attack player-target mutation surface: direct_clears={len(direct_range_player_clears)}"
        )

    try:
        vanish = section(onserver, "static void SummitReleaseTargetsForVanish(", "int rsRecvProcessSkill(")
    except AssertionError as exc:
        errors.append(str(exc))
        vanish = ""
    if "ServerLegacyTargetingAdapter().ClearPlayerTarget(" not in vanish:
        errors.append("vanish monster player-target clear bypasses TargetingService")
    if re.search(r"lpMonster->lpTargetPlayInfo\s*=\s*(?:0|NULL)", vanish):
        errors.append("direct vanish lpTargetPlayInfo clear remains")

    for token in ("LegacyTargetingAdapter::ClearPlayerTarget", "resolved->lpTargetPlayInfo = 0"):
        if token not in targeting_adapter:
            errors.append(f"targeting adapter parity token missing: {token}")

    # Audit the remaining owner/target mutation surfaces. These are intentionally legacy-shell
    # construction/AI or companion-domain state for which no equivalent generic V2 command exists.
    # Unexpected files fail this gate so new gameplay authority cannot silently spread.
    audited_files = {
        "SrcServer/src/systems/gameplay/monsters/v2/legacy/LegacyMonsterAdapter.cpp",
        "SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyTargetingAdapter.cpp",
        "SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp",
        "SrcServer/src/systems/gameplay/events/server_legacy/SvrCaravan.cpp",
        "SrcServer/src/systems/gameplay/character/core/character.cpp",
        "Shared/server/legacy_runtime/OnSever_legacy_server.inl",
        "Shared/gameplay/combat/Svr_Damge_legacy_server.inl",
    }
    mutation_re = re.compile(r"\b(?:lpMasterPlayInfo|lpTargetChar|lpTargetPlayInfo)\s*=\s*(?!=)|\bsmCharInfo\.Brood\s*=\s*(?!=)")
    unexpected = []
    total = 0
    for base in (ROOT / "SrcServer/src",):
        for path in base.rglob("*"):
            if not path.is_file() or path.suffix.lower() not in {".cpp", ".h", ".hpp", ".inl"}:
                continue
            if ".before" in path.name:
                continue
            text = path.read_text(encoding="utf-8", errors="replace")
            if not mutation_re.search(text):
                continue
            rel = path.relative_to(ROOT).as_posix()
            total += len(mutation_re.findall(text))
            if rel not in audited_files:
                unexpected.append(rel)
    for rel in ("Shared/server/legacy_runtime/OnSever_legacy_server.inl", "Shared/gameplay/combat/Svr_Damge_legacy_server.inl"):
        text = read(ROOT / rel)
        total += len(mutation_re.findall(text))
    if unexpected:
        errors.append("unaudited monster/NPC owner-target mutation files: " + ", ".join(sorted(set(unexpected))))

    fail(errors)
    print(
        "monster/npc authority v1: PASS "
        f"(Holy Incantation delegated, exact target clears delegated, audited mutation tokens={total})"
    )


if __name__ == "__main__":
    main()
