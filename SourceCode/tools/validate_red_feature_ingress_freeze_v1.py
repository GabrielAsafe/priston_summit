#!/usr/bin/env python3
from pathlib import Path
import json
import re
import sys

root = Path(__file__).resolve().parents[1]
baseline_path = root / 'tools/architecture/red_feature_ingress_baseline.json'
baseline = json.loads(baseline_path.read_text(errors='replace'))

if baseline.get('schema_version') != 1:
    print('FAIL: unsupported red feature ingress baseline schema', file=sys.stderr)
    sys.exit(1)

tokens = baseline.get('tokens', [])
files = baseline.get('files', {})
required_tokens = {'companion', 'covenant', 'story', 'hunting', 'mining', 'gathering', 'futurefeature'}
if not required_tokens.issubset(set(tokens)):
    print('FAIL: ingress baseline is missing required feature tokens', file=sys.stderr)
    sys.exit(1)

def strip_non_code(text: str) -> str:
    """Replace C/C++ comments and string/character literals with whitespace.

    The ingress freeze is intended to budget feature identifiers in executable
    RED code, not words that appear in documentation or diagnostic strings.
    Newlines are preserved so future diagnostics can still be line-oriented.
    """
    out = []
    i = 0
    state = 'code'
    quote = ''
    while i < len(text):
        current = text[i]
        following = text[i + 1] if i + 1 < len(text) else ''
        if state == 'code':
            if current == '/' and following == '/':
                out.extend('  ')
                i += 2
                state = 'line_comment'
                continue
            if current == '/' and following == '*':
                out.extend('  ')
                i += 2
                state = 'block_comment'
                continue
            if current in ('\"', "'"):
                quote = current
                out.append(' ')
                i += 1
                state = 'literal'
                continue
            out.append(current)
            i += 1
            continue
        if state == 'line_comment':
            if current == '\n':
                out.append('\n')
                state = 'code'
            else:
                out.append(' ')
            i += 1
            continue
        if state == 'block_comment':
            if current == '*' and following == '/':
                out.extend('  ')
                i += 2
                state = 'code'
                continue
            out.append('\n' if current == '\n' else ' ')
            i += 1
            continue
        if current == '\\' and i + 1 < len(text):
            out.extend('  ')
            i += 2
            continue
        if current == quote:
            out.append(' ')
            i += 1
            state = 'code'
            continue
        out.append('\n' if current == '\n' else ' ')
        i += 1
    return ''.join(out)


failures = []
reductions = []
for rel, maximums in sorted(files.items()):
    path = root / rel
    if not path.exists():
        failures.append(f'monitored RED ingress file is missing: {rel}')
        continue
    code = strip_non_code(path.read_text(errors='replace'))
    for token in tokens:
        pattern = re.compile(r'\b' + re.escape(token) + r'[A-Za-z0-9_]*', re.IGNORECASE)
        count = len(pattern.findall(code))
        maximum = int(maximums.get(token, 0))
        status = 'PASS'
        if count > maximum:
            status = 'FAIL'
            failures.append(f'{rel}: {token} grew from max {maximum} to {count}')
        elif count < maximum:
            status = 'REDUCED'
            reductions.append((rel, token, maximum, count))
        print(f'{status}: {rel}: {token}={count} baseline_max={maximum}')

if failures:
    for failure in failures:
        print('FAIL: ' + failure, file=sys.stderr)
    sys.exit(1)

print(f'red feature ingress freeze v1: PASS (files={len(files)}, tokens={len(tokens)}, reductions={len(reductions)})')
