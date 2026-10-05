#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
P=ROOT/'tools/run_frozen_legacy_preflight.sh'
C=ROOT/'tools/check_native_build_prerequisites_v1.sh'
def fail(m): print('native preflight failfast v1: FAIL: '+m,file=sys.stderr); raise SystemExit(1)
if not C.is_file(): fail('missing prerequisite checker')
check=C.read_text(encoding='utf-8')
for token in ('i686-w64-mingw32-g++','i686-w64-mingw32-windres','parallel','Shared2','modules','Dependency/DirectX9/Include','Microsoft DirectX SDK (June 2010)/Include'):
    if token not in check: fail('prerequisite checker missing '+token)
pre=P.read_text(encoding='utf-8')
call='run bash tools/check_native_build_prerequisites_v1.sh'
first_static='run python3 tools/architecture/validate_zero_legacy_debt.py'
if call not in pre: fail('native checker is not wired')
if pre.index(call) > pre.index(first_static): fail('native prerequisite check is not fail-fast before static suite')
if 'required=(i686-w64-mingw32-g++ i686-w64-mingw32-windres)' in pre: fail('stale partial late prerequisite check remains')
print('native preflight failfast v1: PASS (toolchain+parallel+dependency roots checked before static suite)')
