#!/usr/bin/env python3
"""Enforce the V2 boundary for mutable server clan membership state."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
contract = (ROOT / "Shared/gameplay/social/clan/v2/ClanMembershipService.h").read_text()
adapter = (ROOT / "SrcServer/src/systems/social/clan/v2/legacy/LegacyClanMembershipAdapter.cpp").read_text()
onserver = (ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl").read_text(encoding="utf-8")

checks = [
    ("green clan membership service", "class ClanMembershipService" in contract and "IClanMembershipMutationPort" in contract),
    ("green contract has no legacy player type", "rsPLAYINFO" not in contract and "smCHAR_INFO" not in contract),
    ("adapter resolves PlayerId", "players_.Resolve(request.player)" in adapter),
    ("adapter owns clan mark write", "player->smCharInfo.ClassClan =" in adapter),
    ("adapter owns runtime clan-code write", "player->dwClanCode =" in adapter and "GetClanCode" in adapter),
    ("OnServer delegates clan synchronization", onserver.count("SummitOnServerSynchronizeClanMark(") >= 6),
    ("OnServer has no direct live-player clan-code writes", "lpPlayInfo->dwClanCode =" not in onserver),
    ("OnServer has no direct live-player clan-mark writes", "lpPlayInfo->smCharInfo.ClassClan =" not in onserver),
    ("OnServer preserves empty-slot clan initialization", "rsPlayInfo[cnt].dwClanCode = 0;" in onserver),
]
failed = []
for name, ok in checks:
    print(("PASS" if ok else "FAIL") + ": " + name)
    if not ok:
        failed.append(name)
if failed:
    sys.exit(1)
print("clan membership mutation ownership v1: PASS")
