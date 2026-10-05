#!/usr/bin/env python3
from __future__ import annotations
import subprocess, sys
from pathlib import Path

root=Path(__file__).resolve().parents[2]
checks=[
    [sys.executable, str(root/'tools/content_port/validate_canonical_text.py'), str(root/'Content/canonical/text')],
    [sys.executable, str(root/'tools/content_port/sync_linked_text_resources_pt.py'), '--check'],
]
for cmd in checks:
    proc=subprocess.run(cmd,cwd=root,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    if proc.returncode != 0:
        print('ERROR: '+proc.stdout.strip())
        raise SystemExit(proc.returncode)
print('PATCH 203 gate: OK — canonical text payload SHA/size metadata and PT linked resources are consistent')
