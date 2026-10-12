#!/usr/bin/env python3
"""PATCH 243: staged, collision-aware, ALL-decodable missing texture cooker.

Does not replace canonical source assets, existing DDS, main cooked manifest or Git.
The separate --stage/--apply boundary makes complete preflight possible before
publishing a single DDS. Results are resumable and auditable.
"""
from __future__ import annotations

import argparse
from collections import defaultdict
import hashlib
import json
import os
from pathlib import Path
import shutil
import sys
import time

import cook_textures_dds as texture

VERSION = 'SUMMIT_COOK_ALL_MISSING_243_V1'
EXCLUDED_ROOTS = {'.git', 'cooked', 'logs', 'bugfixes', 'quarantine',
                  'backups', 'backup', 'screenshots', 'crashdumps'}
SKIP_OUTLIERS = set(texture.DOCUMENTED_TEXTURE_OUTLIERS)


def sha(path):
    return texture.sha256_file(path)


def report_write(path, obj):
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + '.tmp')
    with temporary.open('w', encoding='utf-8') as out:
        json.dump(obj, out, ensure_ascii=False, indent=2, sort_keys=True)
        out.write('\n')
    os.replace(temporary, path)


def regular(path):
    return path.is_file() and not path.is_symlink()


def discover(client):
    """Enumerate all physical image candidates, not a hot-texture whitelist."""
    result, unsafe = [], []
    for base, dirs, names in os.walk(client, topdown=True, followlinks=False):
        base = Path(base)
        relparts = base.relative_to(client).parts
        if not relparts:
            dirs[:] = [d for d in dirs if d.casefold() not in EXCLUDED_ROOTS and
                       not (base / d).is_symlink()]
        else:
            dirs[:] = [d for d in dirs if not (base / d).is_symlink()]
        for name in names:
            path = base / name
            if path.suffix.casefold() not in texture.SOURCE_EXTENSIONS:
                continue
            rel = path.relative_to(client).as_posix()
            if path.is_symlink() or not path.is_file():
                unsafe.append({'source': rel, 'reason': 'unsafe_symlink_or_nonfile'})
                continue
            try:
                texture.normalize_cooked_logical_path(rel)
            except ValueError:
                unsafe.append({'source': rel, 'reason': 'invalid_logical_path'})
                continue
            result.append(rel)
    return sorted(result, key=lambda x: (x.casefold(), x)), unsafe


def existing_dds(cooked):
    result = defaultdict(list)
    for profile in ('material', 'dib'):
        root = cooked / profile
        if not root.is_dir() or root.is_symlink():
            continue
        for base, dirs, files in os.walk(root, followlinks=False):
            base = Path(base)
            dirs[:] = [d for d in dirs if not (base / d).is_symlink()]
            for name in files:
                p = base / name
                if name.casefold().endswith('.dds'):
                    result[p.relative_to(cooked).as_posix().casefold()].append(p)
    return result


def complete_dds_header(path):
    header = texture.readable_dds_header(path)
    if not header:
        return None
    width, height, fmt = header
    minimum = (width * height * 4 if fmt == 'A8R8G8B8' else
               ((width + 3) // 4) * ((height + 3) // 4) * (8 if fmt == 'DXT1' else 16))
    return header if path.stat().st_size >= 128 + minimum else None


def prepare_plan(client):
    source_paths, unsafe = discover(client)
    by_key = defaultdict(list)
    for rel in source_paths:
        by_key[rel.casefold()].append(rel)
    duplicates = {k for k, rows in by_key.items() if len(rows) > 1}
    existing = existing_dds(client / 'cooked')
    plan = []
    exclusions = list(unsafe)
    counts = {'source_candidates': len(source_paths), 'already_present': 0,
              'missing': 0, 'ambiguous': 0, 'known_invalid': 0,
              'unsafe_or_invalid': len(unsafe), 'existing_unverifiable': 0,
              'existing_stale': 0}
    for rel in source_paths:
        src = client / rel
        if rel.casefold() in duplicates:
            counts['ambiguous'] += 1
            exclusions.append({'source': rel, 'reason': 'ambiguous_case',
                               'variants': by_key[rel.casefold()]})
            continue
        if rel.casefold() in SKIP_OUTLIERS:
            counts['known_invalid'] += 1
            exclusions.append({'source': rel, 'reason': 'documented_unsafe_outlier'})
            continue
        profile = texture.cook_profile_for(Path(rel))
        target = texture.cooked_relative_path(rel, profile).as_posix()
        current = existing.get(target.casefold(), [])
        if current:
            if (len(current) != 1 or not regular(current[0]) or
                    not complete_dds_header(current[0])):
                counts['existing_unverifiable'] += 1
                exclusions.append({'source': rel, 'reason': 'existing_dds_unverifiable',
                                   'paths': [str(p.relative_to(client)) for p in current]})
            elif current[0].stat().st_mtime_ns < src.stat().st_mtime_ns:
                counts['existing_stale'] += 1
                exclusions.append({'source': rel, 'reason': 'existing_dds_stale',
                                   'path': str(current[0].relative_to(client))})
            else:
                counts['already_present'] += 1
            continue
        counts['missing'] += 1
        plan.append({'source': rel, 'target': target, 'profile': profile})
    return plan, exclusions, counts


def ensure_safe_roots(client, staging):
    client = client.resolve(strict=True)
    if not client.is_dir():
        raise ValueError('client root missing')
    # Staging must not overlap client (never stage into the runtime tree).
    staging = staging.resolve()
    if staging == client or client in staging.parents or staging in client.parents:
        raise ValueError('staging must be outside client and not a parent of client')
    if (client / 'cooked').is_symlink():
        raise ValueError('cooked root is a symlink')
    return client, staging


def check_safe_rel(root, rel):
    if root.is_symlink():
        raise ValueError(f'symlink in output root: {root}')
    name = texture.normalize_cooked_logical_path(rel.removesuffix('.dds')
                                                  .split('/', 1)[1]) if rel.startswith(('material/', 'dib/')) else None
    if name is None or rel != rel.replace('\\', '/') or ':' in rel:
        raise ValueError(f'invalid target: {rel}')
    # Do not follow a symlink in any existing path component.
    path = root
    for segment in Path(rel).parts:
        path = path / segment
        if path.is_symlink():
            raise ValueError(f'symlink in target path: {path}')
    return path


def encode(source, rel, profile):
    authored, info = texture.open_legacy_image_with_info(source)
    image = texture.apply_cook_profile(authored, profile, source.suffix)
    # Every decodable texture, including <1 MiB, unaligned and 1x1. Keep
    # lossless UI/small-texture parity. UI still has runtime fallback policy.
    lossless = image.width * image.height * 4 < 1024 * 1024 or texture.is_login_ui(Path(rel))
    fmt = texture.select_dds_format(image, lossless=lossless)
    if fmt == 'A8R8G8B8':
        data, mips = texture.encode_argb8888_dds(image)
    else:
        data, mips = texture.encode_dds(image, fmt, False)
    return data, {'width': image.width, 'height': image.height,
                  'format': fmt, 'mips': mips, 'decode_family': info.get('decode_family', '')}


def stage(client, staging):
    staging.mkdir(parents=True, exist_ok=True)
    plan, exclusions, stats = prepare_plan(client)
    entries = []
    errors = []
    started = time.monotonic()
    for idx, item in enumerate(plan, 1):
        rel = item['source']
        src = client / rel
        target = staging / 'payload' / item['target']
        try:
            if not regular(src):
                raise ValueError('source no longer a regular file')
            digest = sha(src)
            # A previous partial stage can be reused only with a matching
            # source hash AND exact staged DDS digest + structural header.
            sidecar = staging / 'proof' / (hashlib.sha256(item['target'].encode()).hexdigest() + '.json')
            row = None
            if regular(sidecar) and regular(target):
                old = json.loads(sidecar.read_text('utf-8'))
                if (old.get('source_sha256') == digest and old.get('source') == rel and
                    old.get('target') == item['target'] and old.get('profile') == item['profile'] and
                    old.get('cooked_sha256') == sha(target) and
                    complete_dds_header(target) ==
                    (old.get('width'), old.get('height'), old.get('format'))):
                    row = old
            if row is None:
                check_safe_rel(staging / 'payload', item['target'])
                encoded, meta = encode(src, rel, item['profile'])
                target.parent.mkdir(parents=True, exist_ok=True)
                if target.is_symlink():
                    raise ValueError('staged target symlink')
                temp = target.with_name(target.name + '.tmp')
                if temp.is_symlink():
                    raise ValueError('staging temp symlink')
                temp.write_bytes(encoded)
                os.replace(temp, target)
                row = dict(item, source_sha256=digest, source_bytes=src.stat().st_size,
                           cooked_sha256=hashlib.sha256(encoded).hexdigest(),
                           cooked_bytes=len(encoded), **meta)
                report_write(sidecar, row)
            entries.append(row)
        except Exception as exc:
            errors.append({'source': rel, 'reason': type(exc).__name__, 'message': str(exc)})
        if idx % 250 == 0:
            print(f'STAGING_PROGRESS={idx}/{len(plan)} valid={len(entries)} errors={len(errors)}', flush=True)
    manifest = dict(format=VERSION, status='ready', client=str(client),
                    counts=stats, staged=len(entries), failed=len(errors),
                    exclusions=exclusions, errors=errors, entries=entries,
                    elapsed_seconds=round(time.monotonic() - started, 2))
    report_write(staging / 'stage_243.json', manifest)
    print(f'STAGING=PASS missing={len(plan)} staged={len(entries)} errors={len(errors)} '
          f'collisions={stats["ambiguous"]} already_present={stats["already_present"]}')
    print(f'REPORT={staging / "stage_243.json"}')
    return 0


def preflight_apply(client, staging, report):
    if report.get('format') != VERSION or report.get('status') != 'ready':
        raise ValueError('untrusted/incomplete stage manifest')
    if report.get('client') != str(client):
        raise ValueError('stage belongs to a different client root')
    entries = report.get('entries')
    if not isinstance(entries, list) or len(entries) != report.get('staged'):
        raise ValueError('stage entry count mismatch')
    source_paths, _ = discover(client)
    index = defaultdict(list)
    for rel in source_paths:
        index[rel.casefold()].append(rel)
    existing = existing_dds(client / 'cooked')
    seen = set()
    to_publish, already = [], []
    for row in entries:
        rel, target, profile = row['source'], row['target'], row['profile']
        if index.get(rel.casefold()) != [rel]:
            raise ValueError(f'source missing or now ambiguous: {rel}')
        if (profile != texture.cook_profile_for(Path(rel)) or
                target != texture.cooked_relative_path(rel, profile).as_posix()):
            raise ValueError(f'profile/destination altered for: {rel}')
        if target.casefold() in seen:
            raise ValueError('duplicate staged destination')
        seen.add(target.casefold())
        src = client / rel
        data = check_safe_rel(staging / 'payload', target)
        if (not regular(src) or not regular(data) or
                sha(src) != row['source_sha256'] or src.stat().st_size != row['source_bytes']):
            raise ValueError(f'source changed or payload missing: {rel}')
        if sha(data) != row['cooked_sha256'] or data.stat().st_size != row['cooked_bytes']:
            raise ValueError(f'payload tampered: {target}')
        if complete_dds_header(data) != (row['width'], row['height'], row['format']):
            raise ValueError(f'payload DDS header mismatch: {target}')
        destination = check_safe_rel(client / 'cooked', target)
        siblings = existing.get(target.casefold(), [])
        if siblings:
            if (len(siblings) != 1 or siblings[0] != destination or not regular(destination) or
                    sha(destination) != row['cooked_sha256']):
                raise ValueError(f'existing destination differs/collides: {target}')
            already.append((row, destination))
        else:
            if destination.exists() or destination.is_symlink():
                raise ValueError(f'destination occupied: {target}')
            to_publish.append((row, data, destination))
    return to_publish, already


def apply(client, staging):
    path = staging / 'stage_243.json'
    if not regular(path):
        raise ValueError('missing stage report; run --stage first')
    report = json.loads(path.read_text('utf-8'))
    to_publish, already = preflight_apply(client, staging, report)
    print(f'PREFLIGHT=PASS publish={len(to_publish)} already_identical={len(already)} '
          f'excluded={len(report.get("exclusions", []))} decode_errors={len(report.get("errors", []))}', flush=True)
    completed = 0
    for row, source, destination in to_publish:
        # Recheck immediately before publish. Never overwrite existing paths.
        check_safe_rel(client / 'cooked', row['target'])
        if sha(client / row['source']) != row['source_sha256']:
            raise ValueError(f'source changed during publish: {row["source"]}; published={completed}')
        destination.parent.mkdir(parents=True, exist_ok=True)
        check_safe_rel(client / 'cooked', row['target'])
        temp = destination.with_name('.' + destination.name + '.summit243.tmp.' + str(os.getpid()))
        try:
            with temp.open('xb') as sink, source.open('rb') as staged:
                shutil.copyfileobj(staged, sink, length=1024 * 1024)
                sink.flush()
                os.fsync(sink.fileno())
            mtime = max(time.time_ns(), (client / row['source']).stat().st_mtime_ns + 2_000_000_000)
            os.utime(temp, ns=(mtime, mtime))
            os.link(temp, destination)  # EEXIST = fail, never clobber
            completed += 1
        finally:
            temp.unlink(missing_ok=True)
    receipt = dict(format=VERSION, published_this_run=completed, already_identical=len(already),
                   staged=report['staged'], failed=report['failed'],
                   exclusions=len(report['exclusions']), client=str(client))
    report_write(staging / 'receipt_243.json', receipt)
    print(f'PUBLISH=PASS newly_published={completed} already_identical={len(already)} '
          f'decode_errors={report["failed"]} main_manifest_untouched=1 source_originals_untouched=1')
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=('scan', 'stage', 'apply'))
    parser.add_argument('--client', required=True, type=Path)
    parser.add_argument('--staging', required=True, type=Path,
                        help='external directory, e.g. ./bugfixes/SUMMIT_COOK_ALL_243_STAGE')
    args = parser.parse_args(argv)
    try:
        client, staging = ensure_safe_roots(args.client, args.staging)
        if args.action == 'scan':
            plan, exclusions, stats = prepare_plan(client)
            report_write(staging / 'scan_243.json', dict(format=VERSION, counts=stats,
                         candidates=plan, exclusions=exclusions))
            print(f'SCAN=PASS sources={stats["source_candidates"]} missing={len(plan)} '
                  f'existing={stats["already_present"]} ambiguous={stats["ambiguous"]} '
                  f'invalid={stats["known_invalid"]} unverifiable={stats["existing_unverifiable"]}')
            print(f'REPORT={staging / "scan_243.json"}')
            return 0
        if args.action == 'stage':
            return stage(client, staging)
        return apply(client, staging)
    except (ValueError, OSError, KeyError, TypeError, json.JSONDecodeError) as exc:
        print(f'FAIL_CLOSED={type(exc).__name__}: {exc}', file=sys.stderr)
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
