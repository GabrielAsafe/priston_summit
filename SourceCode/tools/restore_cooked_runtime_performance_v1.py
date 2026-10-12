#!/usr/bin/env python3
"""Read-only audit or strict offline recovery of unambiguous *existing* DDS.

PATCH 237: the runtime no longer probes old cooked/foo.dds after the semantic
identity migration. This tool validates legacy artifacts against original
sources before COPYING them to cooked/material/foo.bmp.dds (etc.). It never
cooks, touches sources, overwrites cooked files, or visits quarantine paths.
"""
from __future__ import annotations

import argparse
from collections import Counter, defaultdict
from dataclasses import dataclass
import hashlib
import io
import json
import os
from pathlib import Path
import shutil
import sys
import tempfile
import time

from PIL import Image
import cook_textures_dds as cooker

EXCLUDED_FAMILIES = (
    "image/gui/", "image/clan/", "image/clanimage/", "image/login/",
    "startimage/login/", "image/sky/", "effect/sky/", "startimage/sky/",
    "image/sinimage/items/",  # Includes as-yet-unproven DropItem/ASE textures.
)


@dataclass(frozen=True)
class Migration:
    source: Path
    old: Path
    new: Path
    relative: str
    old_sha: str
    source_sha: str
    width: int
    height: int
    fmt: str


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def legacy_relative(relative: Path) -> Path:
    return relative.with_suffix(".dds")


def verify_pixel_semantics(source: Path, old: Path, fmt: str, width: int, height: int) -> bool:
    """Reject old last-writer/mis-profile DDS by sampling decoded visible pixels.

    This is intentionally more restrictive than a basic header check. It may
    reject some usable lossy DDS; those must be recooked offline, not guessed.
    """
    try:
        authored, _ = cooker.open_legacy_image_with_info(source)
        semantic = cooker.apply_cook_profile(authored, cooker.COOK_PROFILE_MATERIAL, source.suffix)
        if semantic.size != (width, height):
            return False
        if cooker.has_semantic_alpha(semantic) and fmt == "DXT1":
            return False
        with Image.open(old) as stored:
            stored.load()
            if stored.size != semantic.size:
                return False
            actual = stored.convert("RGBA")

        xgrid = sorted({0, width - 1, width // 2} | {int((width - 1) * x / 15) for x in range(16)})
        ygrid = sorted({0, height - 1, height // 2} | {int((height - 1) * y / 15) for y in range(16)})
        rgb_total = 0
        alpha_total = 0
        visible_samples = 0
        samples = 0
        outliers = 0
        for y in ygrid:
            for x in xgrid:
                expected = semantic.getpixel((x, y))
                got = actual.getpixel((x, y))
                alpha_delta = abs(expected[3] - got[3])
                alpha_total += alpha_delta
                samples += 1
                if alpha_delta > 96:
                    outliers += 1
                if expected[3] <= 8 and got[3] <= 8:
                    continue  # Unused RGB in transparent pixels need not match.
                delta = sum(abs(expected[i] - got[i]) for i in (0, 1, 2)) / 3
                rgb_total += delta
                visible_samples += 1
                if delta > 96:
                    outliers += 1
        return (samples > 0 and alpha_total / samples <= 12 and
                rgb_total / max(1, visible_samples) <= 30 and
                outliers / samples <= 0.06)
    except Exception:
        # Unknown legacy encoding, unsupported DDS, or protected-source
        # decoder errors fail closed. This is an offline-only validator.
        return False


def audit(client: Path, limit: int = 0) -> tuple[list[Migration], Counter]:
    cooked_root = client / "cooked"
    stats: Counter = Counter()
    old_groups: dict[str, list[tuple[Path, Path, str]]] = defaultdict(list)
    for source in client.rglob("*"):
        if not source.is_file() or source.is_symlink():
            continue
        relative = source.relative_to(client)
        if not relative.parts or relative.parts[0].casefold() == "cooked":
            continue
        if source.suffix.casefold() not in cooker.SOURCE_EXTENSIONS:
            continue
        stats["source_images"] += 1
        logical = relative.as_posix()
        if logical.casefold().startswith(EXCLUDED_FAMILIES) or cooker.cook_profile_for(relative) != "material":
            stats["excluded_unproven_family"] += 1
            continue
        old_rel = legacy_relative(relative)
        old_groups[old_rel.as_posix().casefold()].append((source, relative, logical))

    existing_material = {}
    material_root = cooked_root / "material"
    if material_root.exists():
        for output in material_root.rglob("*"):
            if output.is_file():
                rel = output.relative_to(cooked_root).as_posix()
                existing_material.setdefault(rel.casefold(), []).append(rel)

    ready = []
    for group in old_groups.values():
        if len(group) != 1:
            stats["legacy_collision_sources"] += len(group)
            continue
        if limit and len(ready) >= limit:
            stats["limit_reached"] += 1
            continue
        source, relative, logical = group[0]
        old = cooked_root / legacy_relative(relative)
        new = cooked_root / cooker.cooked_relative_path(relative, cooker.COOK_PROFILE_MATERIAL)
        try:
            if not old.is_file() or old.is_symlink():
                stats["legacy_dds_missing"] += 1
                continue
            if old.stat().st_mtime < source.stat().st_mtime:
                stats["legacy_dds_stale"] += 1
                continue
            header = cooker.readable_dds_header(old)
            if not header:
                stats["legacy_dds_invalid_header"] += 1
                continue
            width, height, fmt = header
            key = new.relative_to(cooked_root).as_posix().casefold()
            if any(v != new.relative_to(cooked_root).as_posix() for v in existing_material.get(key, [])):
                stats["casefold_target_collision"] += 1
                continue
            old_hash = sha256(old)
            if new.exists():
                stats["already_present" if new.is_file() and sha256(new) == old_hash else "different_new_output_exists"] += 1
                continue
            if not verify_pixel_semantics(source, old, fmt, width, height):
                stats["semantic_mismatch_or_unproven"] += 1
                continue
            ready.append(Migration(source, old, new, logical, old_hash, sha256(source), width, height, fmt))
            stats["verified_reusable"] += 1
        except (OSError, ValueError):
            stats["unreadable_or_unverifiable"] += 1
    return ready, stats


def publish(client: Path, ready: list[Migration], stats: Counter) -> None:
    if not ready:
        print("[INFO] no safe reusable legacy DDS; nothing published")
        return
    cooked = client / "cooked"
    cooked.mkdir(exist_ok=True)
    created: list[Path] = []
    record = []
    with tempfile.TemporaryDirectory(prefix=".summit_restore_", dir=cooked) as work:
        stage = Path(work)
        try:
            # Stage *all* bytes and prove the source and DDS stayed unchanged.
            for idx, entry in enumerate(ready):
                if sha256(entry.old) != entry.old_sha or sha256(entry.source) != entry.source_sha:
                    raise RuntimeError("asset changed during recovery preflight: " + entry.relative)
                staged = stage / f"{idx:06d}.dds"
                shutil.copy2(entry.old, staged)
                if sha256(staged) != entry.old_sha:
                    raise RuntimeError("staging checksum mismatch: " + entry.relative)

            for idx, entry in enumerate(ready):
                entry.new.parent.mkdir(parents=True, exist_ok=True)
                if entry.new.exists():
                    raise RuntimeError("target appeared during publication: " + str(entry.new))
                # Link is atomic, never overwrites an existing target. Staging
                # and cooked roots are on the same filesystem by construction.
                os.link(stage / f"{idx:06d}.dds", entry.new)
                created.append(entry.new)
                record.append({
                    "source": entry.relative,
                    "from_legacy_dds": entry.old.relative_to(cooked).as_posix(),
                    "to_material_dds": entry.new.relative_to(cooked).as_posix(),
                    "source_sha256": entry.source_sha,
                    "cooked_sha256": entry.old_sha,
                    "format": entry.fmt,
                    "width": entry.width, "height": entry.height,
                    "proof": "unique_casefold_source+mtime+decoded_dimensions+alpha+color_samples",
                })
            output = cooked / ("SUMMIT_COOKED_RESTORE_" + time.strftime("%Y%m%d_%H%M%S") + f"_{os.getpid()}.json")
            payload = {
                "format": "SUMMIT_OFFLINE_LEGACY_DDS_RESTORE_V1",
                "timestamp": time.time(),
                "copy_not_cook": True,
                "new_artifacts": record,
                "stats": dict(stats),
            }
            with output.open("x", encoding="utf-8") as handle:
                json.dump(payload, handle, indent=2, ensure_ascii=False)
                handle.write("\n")
            print(f"[OK] published {len(created)} previously cooked, visually verified MATERIAL DDS files")
            print(f"[OK] recovery manifest: {output}")
        except BaseException:
            for path in reversed(created):
                try:
                    path.unlink()
                except OSError:
                    pass
            raise


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--client", type=Path, required=True,
                        help="offline client asset root (contains image/ and cooked/)")
    parser.add_argument("--apply", action="store_true",
                        help="copy verified DDS to new material paths (game must be closed)")
    parser.add_argument("--limit", type=int, default=0,
                        help="max verified DDS to restore; 0 means all verified")
    args = parser.parse_args()
    if args.limit < 0:
        parser.error("--limit cannot be negative")
    client = args.client.resolve()
    if not (client / "image").is_dir() or not (client / "cooked").is_dir():
        parser.error("client must already contain image/ and cooked/ (refusing to create runtime roots)")
    selected, counts = audit(client, args.limit)
    print("SUMMIT_COOKED_PERFORMANCE_RECOVERY_MODE=" + ("APPLY" if args.apply else "AUDIT_ONLY"))
    for key, value in sorted(counts.items()):
        print(f"{key}={value}")
    print(f"safe_material_dds_to_restore={len(selected)}")
    print(f"verified_dds_bytes={sum(item.old.stat().st_size for item in selected)}")
    if args.apply:
        publish(client, selected, counts)
    else:
        print("[SAFE] no files written; run with --apply only after reviewing counts and closing Game.exe")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, RuntimeError) as exc:
        print("[FAIL] offline DDS recovery: " + str(exc), file=sys.stderr)
        sys.exit(1)
