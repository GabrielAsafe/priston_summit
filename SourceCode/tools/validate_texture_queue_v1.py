#!/usr/bin/env python3
"""Offline regression + optional queue log summary; never launches the UI."""
from __future__ import annotations
import argparse
from pathlib import Path
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
SOURCE = HERE.parent / 'Shared/rendering/legacy_3d/material/smTexture_legacy_client.inl'

def test_source() -> None:
    text = SOURCE.read_text(encoding='utf-8')
    assert text.count('SummitTexturePreparingCount.fetch_add(1u') == 1
    assert text.count('SummitTexturePreparingCount.fetch_sub(1u') == 3
    assert text.count('SummitTextureGPUReadyCount.fetch_add(1u') == 1
    assert text.count('SummitTextureGPUReadyCount.fetch_sub(1u') == 1
    assert 'TextureQueueCanAdmit(pressure, normalLimit, priority)' in text
    assert 'SUMMIT_TEXTURE_QUEUE_FAIR' in text
    queue_begin = text.index('static void SummitQueuePreparedTexture(')
    queue_end = text.index('static bool SummitCookedTexturesEnabled()', queue_begin)
    queue = text[queue_begin:queue_end]
    insert = queue.index('SummitPreparedTextureQueue.insert(position, prepared);')
    ready = queue.index('SummitTextureAsyncStates[prepared->index].store(SUMMIT_TEX_ASYNC_READY')
    preparing = queue.index('SummitTexturePreparingCount.fetch_sub(1u', insert)
    unlock = queue.index('\n    }', insert)
    assert insert < ready < preparing < unlock, 'READY must publish under queue lock before render thread pop'
    assert 'SummitTextureQueueDiagFrame(processed, maxUploadMicros, budgetMicros, stoppedByBudget)' in text
    assert 'SummitTextureLoadingBoost.load' in text
    assert 'SUMMIT_TEXTURE_UPLOADS_PER_FRAME' in text
    assert 'SUMMIT_TEXTURE_UPLOAD_BUDGET_US' in text

def test_cpp() -> None:
    with tempfile.TemporaryDirectory(prefix='summit_texture_queue_') as temp:
        executable = Path(temp) / 'test_queue'
        subprocess.run(['g++','-std=c++11','-Wall','-Wextra','-Werror','-pthread',
                        '-I', str(HERE.parent / 'Shared'), str(HERE / 'test_texture_queue_admission_v1.cpp'),
                        '-o',str(executable)], check=True)
        subprocess.run([str(executable)], check=True)

def summarize_log(path: Path) -> None:
    if not path.is_file():
        print(f'QUEUE_LOG_NOT_FOUND={path}')
        return
    records = []
    for line in path.read_text(encoding='utf-8', errors='replace').splitlines():
        if not line.startswith('QUEUE_60F\t'):
            continue
        parts = {}
        for field in line.split('\t')[1:]:
            key,sep,value = field.partition('=')
            if sep:
                try: parts[key] = int(value)
                except ValueError: pass
        records.append(parts)
    print('QUEUE_SAMPLES=',len(records))
    if not records: return
    for key in ('uploaded', 'blocked_cpu', 'blocked_gpu', 'blocked_total','budget_hits','admitted','prepared'):
        print(f'{key}_total={sum(row.get(key,0) for row in records)}')
    for key in ('ready_peak','max_upload_us','staging_mb'):
        print(f'{key}_max={max(row.get(key,0) for row in records)}')
    print('last_sample=',records[-1])

def main() -> None:
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--log',type=Path,help='LOGS/texture_queue_v3_diag.tsv from a separate Wine run')
    args=parser.parse_args()
    test_source()
    test_cpp()
    print('SUMMIT_TEXTURE_QUEUE_SOURCE_GATE=PASS')
    if args.log: summarize_log(args.log)

if __name__=='__main__': main()
