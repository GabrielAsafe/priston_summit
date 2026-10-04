#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
required=[
 'Shared/presentation/v2/AudioService.h',
 'SrcGame/src/systems/audio/playback/legacy/Wave.cpp',
 'SrcGame/src/systems/audio/effects/legacy/effectsnd.cpp',
 'SrcGame/src/systems/audio/voice/legacy_client/Process.cpp',
 'SrcGame/src/systems/audio/voice/ipc/memmap.cpp',
 'SrcGame/src/systems/media/video/ParkDXPlayer.cpp',
 'Shared/presentation/v2/MediaRuntimeInventory.h',
 'docs/architecture/MEDIA_RUNTIME_INVENTORY_V2.md',
]
missing=[p for p in required if not (root/p).exists()]
if missing:
    raise SystemExit('missing media inventory targets: '+', '.join(missing))
doc=(root/'docs/architecture/MEDIA_RUNTIME_INVENTORY_V2.md').read_text(errors='replace')
for token in ('Audio playback','Audio effects','Voice','Video','CProcess::Clear','CParkPlayer::LoadPlayerFile'):
    if token not in doc: raise SystemExit(f'media inventory missing: {token}')
print('media runtime inventory validator: PASS')
