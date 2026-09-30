#!/usr/bin/env python3
from __future__ import annotations
import json
import subprocess
import sys
import tempfile
from pathlib import Path

root=Path(__file__).resolve().parents[2]
allow=root/'tools/architecture/legacy_boundary_allowlist.txt'
entries=[line for line in allow.read_text(errors='ignore').splitlines() if line.strip() and not line.lstrip().startswith('#')]
if entries:
    print(f'FAIL: legacy allowlist contains {len(entries)} debt entries')
    sys.exit(1)
with tempfile.TemporaryDirectory() as tmp:
    metrics=Path(tmp)/'metrics.json'
    proc=subprocess.run([sys.executable, str(root/'tools/architecture/validate_legacy_boundary.py'), '--root', str(root), '--metrics-out', str(metrics)], text=True)
    if proc.returncode:
        sys.exit(proc.returncode)
    data=json.loads(metrics.read_text())
for key in ('allowlist_entries','green_legacy_include_count','green_legacy_symbol_count','green_renderer_include_count','green_renderer_symbol_count','green_direct_socket_send_count','unallowlisted_violations'):
    if data.get(key) != 0:
        print(f'FAIL: {key}={data.get(key)}')
        sys.exit(1)
print('zero legacy debt guard: PASS (allowlist/legacy/renderer/direct-socket debt/violations all zero)')
