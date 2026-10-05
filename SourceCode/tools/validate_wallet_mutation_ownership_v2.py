#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
ADAPTER = ROOT / "SrcServer/src/systems/commerce/v2/legacy/LegacyWalletAdapter.cpp"
KERNEL = ROOT / "SrcServer/src/core/server/legacy_runtime/PartySever.cpp"
ONSERVER = ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl"
RECORD = ROOT / "Shared/persistence/character_record/record_legacy_server.inl"

CALL_RE = re.compile(r"->(?:AddServerMoney|SubServerMoney|SetServerMoney)\s*\(")
FIELD_WRITE_RE = re.compile(r"(?:->|\.)ServerMoney\s*(?:[+\-*/]?=|\+\+|--)")
ALLOWED_SLOT_INIT = "rsPlayInfo[cnt].ServerMoney = 0;"


def fail(message: str) -> None:
    print(f"wallet mutation ownership v2: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def candidate_files():
    seen = set()
    for base in (ROOT / "SrcServer/src",):
        for path in base.rglob("*"):
            if not path.is_file() or path.suffix not in {".cpp", ".h", ".hpp", ".inl"}:
                continue
            rel = path.relative_to(ROOT).as_posix()
            if "/legacy_oracle/" in f"/{rel}/" or ".before" in path.name or "_legacy_client" in path.name:
                continue
            if path in (ADAPTER, KERNEL):
                continue
            if path not in seen:
                seen.add(path)
                yield path
    for path in (ONSERVER, RECORD):
        if path not in seen:
            yield path


def main() -> None:
    for required in (ADAPTER, KERNEL, ONSERVER, RECORD):
        if not required.is_file():
            fail(f"missing {required.relative_to(ROOT)}")

    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")
    for token in (
        "legacyPlayer->AddServerMoney",
        "legacyPlayer->SubServerMoney",
        "legacyPlayer->SetServerMoney",
        "LegacyWalletAdapter::SetBalance",
    ):
        if token not in adapter:
            fail(f"wallet adapter missing kernel boundary token: {token}")

    kernel = KERNEL.read_text(encoding="utf-8", errors="replace")
    for token in (
        "rsPLAYINFO::AddServerMoney",
        "rsPLAYINFO::SubServerMoney",
        "rsPLAYINFO::SetServerMoney",
    ):
        if token not in kernel:
            fail(f"legacy wallet kernel missing: {token}")

    unauthorized = []
    shell_state_writes = 0
    for path in candidate_files():
        text = path.read_text(encoding="utf-8", errors="replace")
        rel = path.relative_to(ROOT).as_posix()
        for lineno, line in enumerate(text.splitlines(), 1):
            if CALL_RE.search(line):
                unauthorized.append(f"{rel}:{lineno}: direct wallet kernel call: {line.strip()}")
            if FIELD_WRITE_RE.search(line):
                if path == ONSERVER and ALLOWED_SLOT_INIT in line:
                    shell_state_writes += 1
                    continue
                unauthorized.append(f"{rel}:{lineno}: direct ServerMoney write: {line.strip()}")

    if unauthorized:
        fail("\n" + "\n".join(unauthorized))

    print(
        "wallet mutation ownership v2: PASS "
        f"(adapter_kernel_calls=3, shell_state_writes={shell_state_writes}, unauthorized=0)"
    )


if __name__ == "__main__":
    main()
