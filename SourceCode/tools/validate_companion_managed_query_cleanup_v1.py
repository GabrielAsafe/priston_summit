#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
H=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.h').read_text(errors='replace')
C=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='replace')
def fail(m): print(f'companion managed query cleanup v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for text,name in ((H,'header'),(C,'implementation')):
  if 'IsManagedCompanion(' in text: fail(f'dead IsManagedCompanion API remains in {name}')
 print('companion managed query cleanup v1: PASS (unused smCHAR pointer query removed)')
if __name__=='__main__': main()
