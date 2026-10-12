#!/usr/bin/env python3
"""PATCH 239: source gates and read-only analysis of opt-in runtime diagnostics."""
from __future__ import annotations

import argparse
from collections import Counter
from pathlib import Path
import statistics

ROOT = Path(__file__).resolve().parents[1]


def parse_log(path: Path, prefix: str) -> list[dict[str, str]]:
    entries: list[dict[str, str]] = []
    if not path.exists():
        raise FileNotFoundError(path)
    for raw in path.read_text(encoding="utf-8", errors="replace").splitlines():
        columns = raw.strip().split("\t")
        if not columns or not columns[0].startswith(prefix):
            continue
        row = {"event": columns[0]}
        for token in columns[1:]:
            if "=" in token:
                key, value = token.split("=", 1)
                row[key] = value
        entries.append(row)
    return entries


def integer(record: dict[str, str], key: str) -> int:
    try:
        return int(record.get(key, "0"))
    except ValueError:
        return 0


def source_gates(root: Path) -> None:
    win = (root / "SrcGame/src/platform/windows/application/Winmain.cpp").read_text(encoding="utf-8")
    texture = (root / "Shared/rendering/legacy_3d/material/smTexture_legacy_client.inl").read_text(encoding="utf-8")
    for snippet in (
        'std::getenv("SUMMIT_FOG_FORCE_OFF")',
        'std::getenv("SUMMIT_FRAME_DIAG_239")',
        'Summit239ForceD3DFogOff()',
        'D3DRS_FOGENABLE, FALSE',
        'stage_pump_us=',
        'upload_pump_us=',
        'frame_stage_239_diag.tsv',
        'Summit239FrameSample(',
    ):
        assert snippet in win, f"Winmain missing: {snippet}"
    for snippet in (
        'SUMMIT_TEXTURE_SLOW_DIAG_239',
        'SUMMIT_TEXTURE_SLOW_DIAG_US',
        'texture_slow_upload_239.tsv',
        'gpuQueuedAt',
        'wait_gpu_us=',
        'Summit239LogSlowUpload(',
    ):
        assert snippet in texture, f"texture loader missing: {snippet}"
    assert 'if (Summit239FrameDiagnosticEnabled())' in win
    assert 'else if (smRender.m_FogIsRend && smRender.m_FogMode)' in win
    assert 'if (generationMatches && Summit239SlowUploadEnabled()' in texture
    assert 'static const bool enabled' in texture
    print('SUMMIT_239_SOURCE_GATES=PASS')


def summarise(stage: list[dict[str, str]], upload: list[dict[str, str]]) -> None:
    print(f"FRAME_DIAG_EVENTS={len(stage)}")
    if stage:
        for key in ('gap_us', 'stage_pump_us', 'upload_pump_us', 'model_us', 'sky_us', 'prewarm_us', 'effect_us'):
            print(f"max_{key}={max(integer(row, key) for row in stage)}")
        print("slow_frame_events=" + str(sum(row['event'] == 'FRAME_SLOW' for row in stage)))
        print("fields_sampled=" + ",".join(sorted({row.get('field', '?') for row in stage})))
    print(f"SLOW_UPLOAD_EVENTS={len(upload)}")
    if upload:
        commits = [integer(row, "commit_us") for row in upload]
        print(f"max_commit_us={max(commits)}")
        print(f"max_wait_gpu_us={max(integer(row, 'wait_gpu_us') for row in upload)}")
        print(f"p50_logged_commit_us={int(statistics.median(commits))}")
        for item in sorted(upload, key=lambda row: integer(row, 'commit_us'), reverse=True)[:5]:
            print('SLOWEST', item.get('commit_us', '?'), 'us',
                  'gpu_wait=', item.get('wait_gpu_us', '?'),
                  'cooked=', item.get('cooked', '?'),
                  'texture=', item.get('logical', '?'))
        print('cooked_slow_events=' + str(Counter(row.get('cooked', '?') for row in upload)))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--frame-log', type=Path)
    parser.add_argument('--upload-log', type=Path)
    args = parser.parse_args()
    source_gates(ROOT)
    stage = parse_log(args.frame_log, 'FRAME_') if args.frame_log else []
    upload = parse_log(args.upload_log, 'UPLOAD_SLOW') if args.upload_log else []
    if args.frame_log or args.upload_log:
        summarise(stage, upload)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
