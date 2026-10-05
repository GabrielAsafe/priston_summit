#!/usr/bin/env python3
from pathlib import Path
import collections
import json
import re
import sys

root = Path(__file__).resolve().parents[1]
baseline_path = root / 'tools/architecture/direct_socket_send_baseline.json'
baseline = json.loads(baseline_path.read_text(errors='replace'))
if baseline.get('schema_version') != 1:
    print('FAIL: unsupported direct socket baseline schema', file=sys.stderr)
    sys.exit(1)

suffixes = set(baseline.get('source_suffixes', []))
pattern = re.compile(rb'\b(smWsock[A-Za-z0-9_]*|lpsmSock)\s*->\s*Send\s*\(')
current = {}
for base_rel in ('SrcGame/src', 'SrcServer/src', 'Shared'):
    base = root / base_rel
    if not base.exists():
        continue
    for path in base.rglob('*'):
        if not path.is_file() or path.suffix.lower() not in suffixes:
            continue
        hits = collections.Counter(token.decode('ascii') for token in pattern.findall(path.read_bytes()))
        if hits:
            current[path.relative_to(root).as_posix()] = hits

baseline_files = baseline.get('files', {})
failures = []
reductions = 0
for rel, hits in sorted(current.items()):
    entry = baseline_files.get(rel)
    if entry is None:
        failures.append(f'new direct socket owner: {rel}')
        continue
    maxima = entry.get('max_counts', {})
    for channel, count in sorted(hits.items()):
        maximum = int(maxima.get(channel, 0))
        if count > maximum:
            failures.append(f'{rel}: {channel} grew from max {maximum} to {count}')
        elif count < maximum:
            reductions += 1
            print(f'REDUCED: {rel}: {channel}={count} baseline_max={maximum}')

# Baseline entries may disappear entirely as RED is retired; that is a reduction.
for rel, entry in sorted(baseline_files.items()):
    if rel not in current:
        reductions += len(entry.get('max_counts', {}))
        print(f'REDUCED: {rel}: all direct socket sends removed')
        continue
    for channel, maximum in sorted(entry.get('max_counts', {}).items()):
        if channel not in current[rel]:
            reductions += 1
            print(f'REDUCED: {rel}: {channel}=0 baseline_max={maximum}')

# AMBER is explicit, not inferred by a broad filename convention in the validator.
amber = sorted(rel for rel, entry in baseline_files.items() if entry.get('role') == 'AMBER')
expected_amber = {
    'SrcGame/src/systems/network/v2/legacy/LegacyClientCommandSender.cpp',
    'SrcGame/src/systems/network/v2/legacy/LegacyBackendChannelSender.cpp',
    'SrcServer/src/systems/community/chat/v2/legacy/LegacyChatDeliveryAdapter.cpp',
    'SrcServer/src/systems/gameplay/pets/v2/legacy/LegacyPetRuntimeAdapter.cpp',
    'SrcServer/src/systems/network/v2/legacy/LegacyPacketSender.cpp',
    'SrcServer/src/systems/network/v2/legacy/LegacyBackendChannelSender.cpp',
    'SrcServer/src/systems/commerce/v2/legacy/LegacyMixtureResetKernel.inl',
    'SrcServer/src/systems/commerce/v2/legacy/LegacyAgingKernel.inl',
    'SrcServer/src/systems/commerce/v2/legacy/LegacyCraftingKernel.inl',
}
if set(amber) != expected_amber:
    failures.append('AMBER direct socket ownership set changed without validator review')

kernel = sorted(rel for rel, entry in baseline_files.items() if entry.get('role') == 'KERNEL')
expected_kernel = {'Shared/network/transport/legacy/SmWSockPacketSendQueueLegacyCommon.inl'}
if set(kernel) != expected_kernel:
    failures.append('legacy transport KERNEL ownership set changed without validator review')

inactive = sorted(rel for rel, entry in baseline_files.items() if entry.get('role') == 'INACTIVE_TEMPLATE')
expected_inactive = {
    'Shared/gameplay/combat/LegacyDamageCommon.inl',
    'Shared/gameplay/combat/LegacyDamagePacket.inl',
    'Shared/gameplay/combat/LegacyDamagePolicy.inl',
    'Shared/gameplay/skills/LegacySkillSubCommon.inl',
}
if set(inactive) != expected_inactive:
    failures.append('inactive legacy template ownership set changed without validator review')

oracle = sorted(rel for rel, entry in baseline_files.items() if entry.get('role') == 'ORACLE')
expected_oracle = {rel for rel in baseline_files if 'legacy_oracle/' in rel}
if set(oracle) != expected_oracle:
    failures.append('legacy oracle ownership classification drifted')

if failures:
    for failure in failures:
        print('FAIL: ' + failure, file=sys.stderr)
    sys.exit(1)

red_files = sum(1 for entry in baseline_files.values() if entry.get('role') == 'RED')
print(f'direct socket send budget v1: PASS (tracked_files={len(baseline_files)}, red_files={red_files}, amber_files={len(amber)}, kernel_files={len(kernel)}, inactive_templates={len(inactive)}, oracle_files={len(oracle)}, reductions={reductions})')
