#!/usr/bin/env python3
from __future__ import annotations
import argparse, fnmatch, json, re, sys
from pathlib import Path

FORBIDDEN_SYMBOLS = [
    'rsPLAYINFO','smCHAR','STG_AREA','sFIELD','smWINSOCK','TRANS_RECORD_DATA','cQuestHandler','OpenFlag'
]
FORBIDDEN_INCLUDE_BASENAMES = {
    'smwsock.h','character.h','sininventory.h','sinskill.h','field.h','record.h','onserver.h','smd3d.h'
}
FORBIDDEN_RENDERER_INCLUDE_BASENAMES = {'d3d9.h','d3dx9.h'}
GREEN_GLOBS = [
    'Shared/gameplay/**/v2/**',
    'Shared/gameplay/core/v2/**',
    'Shared/gameplay/world/v2/**',
    'Shared/gameplay/player/v2/**',
    'Shared/network/v2/**',
    'Shared/presentation/v2/**',
    'SrcServer/src/systems/gameplay/**/v2/**',
    'SrcServer/src/systems/network/v2/**',
    'SrcGame/src/systems/ui/v2/**',
    'SrcGame/src/systems/presentation/v2/**',
]
AMBER_MARKERS = ['/legacy/', '/legacy_support/']
# Explicit platform/render backends may live beside a GREEN interface for build/layout
# compatibility while remaining AMBER implementation details. Their public headers
# are still scanned as GREEN and must stay renderer-handle free.
AMBER_FILES = {
    'SrcGame/src/systems/ui/v2/rendering/Dx9UIRenderer.cpp',
}
SOURCE_SUFFIXES = {'.h','.hpp','.cpp','.cc','.cxx'}
HEADER_SUFFIXES = {'.h','.hpp'}

INC_RE = re.compile(r'^\s*#\s*include\s*[<\"]([^>\"]+)[>\"]')
SYM_RE = re.compile(r'\b(' + '|'.join(map(re.escape, FORBIDDEN_SYMBOLS)) + r')\b|\b(SIN_[A-Za-z0-9_]+)\b')
DIRECT_SOCKET_SEND_RE = re.compile(r'\b(?:lpsmSock|smWsock[A-Za-z0-9_]*)\s*->\s*Send\s*\(')
RENDERER_SYM_RE = re.compile(
    r'\b(?:IDirect3D[A-Za-z0-9_]*|ID3DX[A-Za-z0-9_]*|LPDIRECT3D[A-Za-z0-9_]*|'
    r'D3D(?:COLOR|FVF|RS|BLEND|CULL|PT|SBT|SURFACE|FORMAT|POOL|USAGE|LOCK|PRESENT|DEVTYPE)[A-Z0-9_]*)\b')

def norm(p: Path) -> str:
    return p.as_posix()

def is_green(rel: str) -> bool:
    if rel in AMBER_FILES:
        return False
    padded='/' + rel
    if any(m in padded for m in AMBER_MARKERS):
        return False
    # Existing adapters named Legacy* are AMBER even when historical layout
    # placed them beside V2 runtime files instead of under a /legacy/ folder.
    if Path(rel).name.startswith('Legacy'):
        return False
    return any(fnmatch.fnmatch(rel, g) for g in GREEN_GLOBS)

def is_amber(rel: str) -> bool:
    if rel in AMBER_FILES:
        return True
    if not any(fnmatch.fnmatch(rel, g) for g in GREEN_GLOBS):
        return False
    padded='/' + rel
    return any(m in padded for m in AMBER_MARKERS) or Path(rel).name.startswith('Legacy')

def strip_comments(line: str) -> str:
    # Conservative single-line stripping; block comments are intentionally not parsed.
    return line.split('//',1)[0]

def load_allowlist(path: Path):
    allowed=set()
    if not path.exists(): return allowed
    for raw in path.read_text(errors='ignore').splitlines():
        line=raw.strip()
        if not line or line.startswith('#'): continue
        parts=[p.strip() for p in line.split('|')]
        if len(parts) >= 2:
            allowed.add((parts[0], parts[1]))
    return allowed

def scan(root: Path, allowlist):
    violations=[]
    metrics={'green_files':0,'green_header_files':0,'amber_files':0,'green_legacy_include_count':0,'green_legacy_symbol_count':0,
             'green_renderer_include_count':0,'green_renderer_symbol_count':0,
             'green_direct_socket_send_count':0,
             'allowlist_entries':len(allowlist),'unallowlisted_violations':0}
    for p in root.rglob('*'):
        if not p.is_file() or p.suffix.lower() not in SOURCE_SUFFIXES: continue
        rel=norm(p.relative_to(root))
        if is_amber(rel):
            metrics['amber_files'] += 1
            continue
        if not is_green(rel): continue
        metrics['green_files'] += 1
        if p.suffix.lower() in HEADER_SUFFIXES: metrics['green_header_files'] += 1
        seen=set()
        for lineno, raw in enumerate(p.read_text(errors='ignore').splitlines(),1):
            code=strip_comments(raw)
            inc=INC_RE.match(code)
            if inc:
                base=Path(inc.group(1)).name.lower()
                if base in FORBIDDEN_INCLUDE_BASENAMES:
                    key='include:'+base
                    if (rel,key) not in seen:
                        seen.add((rel,key)); metrics['green_legacy_include_count'] += 1
                        allowed=(rel,key) in allowlist
                        violations.append((rel,lineno,key,allowed,raw.strip()))
                if base in FORBIDDEN_RENDERER_INCLUDE_BASENAMES:
                    key='include:'+base
                    if (rel,key) not in seen:
                        seen.add((rel,key)); metrics['green_renderer_include_count'] += 1
                        allowed=(rel,key) in allowlist
                        violations.append((rel,lineno,key,allowed,raw.strip()))
            for match in SYM_RE.finditer(code):
                token=match.group(1) or match.group(2)
                key='symbol:'+token
                if (rel,key) in seen: continue
                seen.add((rel,key)); metrics['green_legacy_symbol_count'] += 1
                allowed=(rel,key) in allowlist
                violations.append((rel,lineno,key,allowed,raw.strip()))
            for match in RENDERER_SYM_RE.finditer(code):
                token=match.group(0)
                key='renderer:'+token
                if (rel,key) in seen: continue
                seen.add((rel,key)); metrics['green_renderer_symbol_count'] += 1
                allowed=(rel,key) in allowlist
                violations.append((rel,lineno,key,allowed,raw.strip()))
            if DIRECT_SOCKET_SEND_RE.search(code):
                key='transport:direct_socket_send'
                if (rel,key) not in seen:
                    seen.add((rel,key)); metrics['green_direct_socket_send_count'] += 1
                    allowed=(rel,key) in allowlist
                    violations.append((rel,lineno,key,allowed,raw.strip()))
    metrics['unallowlisted_violations']=sum(1 for v in violations if not v[3])
    return metrics, violations

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--root', default=str(Path(__file__).resolve().parents[2]))
    ap.add_argument('--allowlist', default=None)
    ap.add_argument('--metrics-out', default=None)
    ap.add_argument('--report-only', action='store_true')
    ap.add_argument('--allow-legacy-debt', action='store_true',
                    help='explicitly permit non-empty legacy allowlist entries for historical diagnostics')
    args=ap.parse_args()
    root=Path(args.root).resolve()
    allow=Path(args.allowlist).resolve() if args.allowlist else root/'tools/architecture/legacy_boundary_allowlist.txt'
    allowed=load_allowlist(allow)
    metrics, violations=scan(root, allowed)
    for rel,line,key,ok,text in violations:
        status='ALLOW' if ok else 'ERROR'
        print(f'{status} {rel}:{line} {key} :: {text}')
    print(json.dumps(metrics, sort_keys=True))
    if args.metrics_out:
        Path(args.metrics_out).write_text(json.dumps(metrics, indent=2, sort_keys=True)+'\n')
    if not args.report_only:
        if metrics['unallowlisted_violations']:
            return 1
        if metrics['allowlist_entries'] and not args.allow_legacy_debt:
            print('ERROR legacy debt allowlist must remain empty; use --allow-legacy-debt only for historical diagnostics')
            return 1
    return 0
if __name__=='__main__': sys.exit(main())
