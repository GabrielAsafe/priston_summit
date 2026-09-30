#!/usr/bin/env python3
"""Guard the read/write Story quest adapter against legacy cQUESTHANDLER drift."""

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT / "SrcServer/src/systems/gameplay/progression/v2/LegacyQuestContract.h"
HANDLER = ROOT / "SrcServer/src/systems/gameplay/quests/server/cQuestHandler.cpp"
REPOSITORY = ROOT / "SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestRepository.cpp"

EXPECTED = {
    "kRepeatedQuestKind": 21,
    "kDailyQuestKind": 7,
    "kSingleQuestKind": 9,
}


def fail(message: str) -> None:
    print(f"legacy quest contract validation: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if not CONTRACT.is_file() or not HANDLER.is_file() or not REPOSITORY.is_file():
        fail("contract, cQuestHandler source, or quest repository source is missing")

    contract = CONTRACT.read_text(encoding="utf-8")
    resolved = {}
    for name in EXPECTED:
        match = re.search(rf"\b{name}\s*=\s*(-?\d+)\s*;", contract)
        if not match:
            fail(f"missing {name} in LegacyQuestContract.h")
        resolved[name] = int(match.group(1))
    if resolved != EXPECTED:
        fail(f"unexpected QuestKind constants: {resolved}")

    # Restrict the proof to StartQuest rather than accepting unrelated quest-kind checks.
    raw = HANDLER.read_bytes()
    text = raw.decode("latin-1")
    start = text.find("void cQUESTHANDLER::StartQuest")
    if start < 0:
        fail("cQUESTHANDLER::StartQuest not found")
    end = text.find("void cQUESTHANDLER::", start + 1)
    body = text[start:] if end < 0 else text[start:end]

    # The original implementation used switch cases. The modernized handler delegates
    # persistence to LegacyQuestRepository::StartProgress, so accept either representation
    # while still proving that exactly the canonical kinds remain wired at the handler edge.
    handled = {int(value) for value in re.findall(r"\bcase\s+(\d+)\s*:", body)}
    handled.update(int(value) for value in re.findall(r"\bQuestKind\s*==\s*(\d+)", body))
    missing = sorted(set(EXPECTED.values()) - handled)
    if missing:
        fail(f"StartQuest no longer handles QuestKind values {missing}")

    if "ServerLegacyQuestRepository().StartProgress" not in body:
        fail("StartQuest no longer delegates quest start persistence to the repository")

    repository = REPOSITORY.read_text(encoding="utf-8", errors="ignore")
    repo_start = repository.find("bool LegacyQuestRepository::StartProgress")
    if repo_start < 0:
        fail("LegacyQuestRepository::StartProgress not found")
    repo_end = repository.find("bool LegacyQuestRepository::", repo_start + 1)
    repo_body = repository[repo_start:] if repo_end < 0 else repository[repo_start:repo_end]
    repository_kinds = {int(value) for value in re.findall(r"\bquestKind\s*(?:==|!=)\s*(\d+)", repo_body)}
    missing_repo = sorted(set(EXPECTED.values()) - repository_kinds)
    if missing_repo:
        fail(f"StartProgress no longer preserves QuestKind values {missing_repo}")

    print("legacy quest contract validation: PASS")


if __name__ == "__main__":
    main()
