#!/usr/bin/env python3
"""Summarize Summit runtime asset probe logs from client/server deployments."""
from __future__ import annotations
import argparse
from collections import Counter, defaultdict
from pathlib import Path


def parse_attempt(path: Path):
    if not path.is_file():
        return []
    out=[]
    for raw in path.read_text(encoding='utf-8', errors='replace').splitlines():
        if not raw or raw.startswith('SESSION_BEGIN'):
            continue
        parts=raw.split('\t',5)
        if len(parts)!=6:
            continue
        tick,tid,role,category,operation,asset=parts
        out.append((role,category,operation,asset,tick,tid))
    return out


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--repo-root', default='..', help='Summit repo root; default assumes execution from SourceCode/')
    ap.add_argument('--contains', default='', help='case-insensitive path substring filter')
    ap.add_argument('--category', default='', help='exact category filter, e.g. STAGE or TEXTURE')
    ap.add_argument('--role', default='', choices=('', 'CLIENT', 'SERVER'), help='role filter')
    ap.add_argument('--show', type=int, default=80, help='max unique paths/failures to print')
    ap.add_argument('--timeline', type=int, default=0, help='print the last N matching raw events in time order')
    args=ap.parse_args()
    root=Path(args.repo_root).resolve()
    logs=[root/'cliente/LOGS/runtime_asset_attempts.log', root/'Servidor/LOGS/runtime_asset_attempts.log']
    rows=[]
    for p in logs:
        found=parse_attempt(p)
        print(f'LOG={p} EXISTS={int(p.is_file())} EVENTS={len(found)}')
        rows.extend(found)
    needle=args.contains.lower()
    if needle:
        rows=[r for r in rows if needle in r[3].lower()]
    if args.category:
        wanted=args.category.upper()
        rows=[r for r in rows if r[1].upper()==wanted]
    if args.role:
        rows=[r for r in rows if r[0]==args.role]
    attempts=[r for r in rows if not r[2].endswith('_OK') and not r[2].endswith('_FAIL')]
    results=[r for r in rows if r[2].endswith('_OK') or r[2].endswith('_FAIL')]
    failures=[r for r in results if r[2].endswith('_FAIL')]
    cats=Counter((r[0],r[1]) for r in attempts)
    unique=defaultdict(int)
    for role,cat,op,asset,_,_ in attempts:
        unique[(role,cat,asset)] += 1
    print(f'ATTEMPTS={len(attempts)} RESULTS={len(results)} FAIL_RESULTS={len(failures)} UNIQUE_ASSETS={len(unique)}')
    for (role,cat),count in sorted(cats.items()):
        print(f'CATEGORY role={role} category={cat} attempts={count}')
    print('=== UNIQUE ASSETS ===')
    for i,((role,cat,asset),count) in enumerate(sorted(unique.items(), key=lambda x:(x[0][0],x[0][1],x[0][2].lower()))):
        if i>=args.show:
            print(f'... truncated; use --show {len(unique)} to print all')
            break
        print(f'{role}\t{cat}\t{count}\t{asset}')
    if failures:
        print('=== FAILED RESULTS ===')
        for role,cat,op,asset,tick,tid in failures[-args.show:]:
            print(f'{role}\t{cat}\t{op}\t{asset}\ttick={tick}\ttid={tid}')
    if args.timeline:
        print('=== TIMELINE ===')
        for role,cat,op,asset,tick,tid in rows[-args.timeline:]:
            print(f'{tick}\t{tid}\t{role}\t{cat}\t{op}\t{asset}')

if __name__=='__main__':
    main()
