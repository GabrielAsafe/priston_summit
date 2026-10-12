#!/usr/bin/env python3
"""Resolve Wine PE crash addresses using local debug symbols; no logs generated.

Usage: python3 tools/resolve_model_cook_crash_263.py \
          --exe ../Servidor/Server.exe 0x0076725f 0x0076826e 0x0082c9d2
If Server.exe was linked without symbols, output stays UNKNOWN rather than guessing.
"""
from __future__ import annotations
import argparse
from pathlib import Path
import re
import shutil
import subprocess
import sys


ADDRESS = re.compile(r'(?:0x)?[0-9a-fA-F]{6,16}\Z')


def resolve(exe: Path, addresses: list[str], tool: str | None = None):
    exe = exe.resolve(strict=True)
    if not exe.is_file():
        raise ValueError('executable is not a regular file')
    parsed = []
    for value in addresses:
        if not ADDRESS.fullmatch(value):
            raise ValueError('invalid hexadecimal address: ' + value)
        parsed.append('0x' + value.removeprefix('0x'))
    binary = tool or shutil.which('i686-w64-mingw32-addr2line') or shutil.which('addr2line')
    if not binary:
        raise ValueError('addr2line unavailable')
    if len(parsed) > 64:
        raise ValueError('too many addresses')
    result = subprocess.run([binary, '-f', '-C', '-e', str(exe), *parsed],
                            capture_output=True, text=True, timeout=20, check=False)
    if result.returncode:
        raise ValueError('addr2line failed: ' + result.stderr.strip()[:400])
    lines = result.stdout.splitlines()
    if len(lines) != len(parsed) * 2:
        raise ValueError('unexpected symbolizer output')
    return [(addr, lines[2*i], lines[2*i+1]) for i, addr in enumerate(parsed)]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--exe', type=Path, required=True)
    p.add_argument('--tool', help='Optional path to PE-aware addr2line')
    p.add_argument('addresses', nargs='+')
    args = p.parse_args()
    try:
        for addr, func, source in resolve(args.exe, args.addresses, args.tool):
            print(addr, 'UNKNOWN (binary without symbols)' if func == '??' and source.startswith('??')
                  else func + ' ' + source)
    except (ValueError, OSError, subprocess.TimeoutExpired) as exc:
        print('MODEL_SYMBOLIZE_263=FAIL ' + str(exc), file=sys.stderr)
        return 2
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
