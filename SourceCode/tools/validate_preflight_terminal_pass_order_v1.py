#!/usr/bin/env python3
from pathlib import Path
import sys

preflight = Path(__file__).with_name("run_frozen_legacy_preflight.sh")
text = preflight.read_text(encoding="utf-8")
marker = 'echo "FROZEN_LEGACY_${MODE^^}_PREFLIGHT: PASS"'
pos = text.find(marker)
if pos < 0:
    print("FAIL: final preflight PASS marker not found")
    sys.exit(1)

tail = text[pos + len(marker):]
late_gates = [line.strip() for line in tail.splitlines() if line.strip().startswith("run ")]
if late_gates:
    print("FAIL: executable gates exist after final PASS marker:")
    for gate in late_gates:
        print(f"  {gate}")
    sys.exit(1)

print("preflight terminal PASS order v1: PASS (no executable gate after final PASS)")
