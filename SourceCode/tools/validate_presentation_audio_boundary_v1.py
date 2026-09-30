#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
ids=(root/'Shared/presentation/v2/PresentationIds.h').read_text()
contract=(root/'Shared/presentation/v2/AudioService.h').read_text()
adapter=(root/'SrcGame/src/systems/presentation/v2/legacy/LegacyAudioAdapter.cpp').read_text()
consumer=(root/'SrcGame/src/systems/gameplay/character/status_ui/sinCharStatus.cpp').read_text()
legacy_catalog=(root/'Shared/inventory/legacy/sinItem_legacy_client.h').read_text()
checks=[
 ('sound id is logical key', 'struct SoundId' in ids and 'std::string key' in ids),
 ('green audio port exists', 'class IAudioService' in contract and 'PlayOneShot' in contract),
 ('adapter maps reset key', 'ui.stat_reset.consume' in adapter and 'kEatPotion2SoundIndex = 25' in adapter),
 ('adapter index matches historical catalog', '#define\t\tSIN_SOUND_EAT_POTION2\t\t25' in legacy_catalog or '#define  SIN_SOUND_EAT_POTION2' in legacy_catalog),
 ('adapter owns legacy sound call', 'sinPlaySound(soundIndex, volume)' in adapter),
 ('consumer uses audio service', 'ClientLegacyAudioAdapter().PlayOneShot' in consumer and 'SoundId("ui.stat_reset.consume")' in consumer),
 ('migrated consumer no direct potion sound call', 'sinPlaySound(SIN_SOUND_EAT_POTION2);\n\tStartEffect' not in consumer),
]
failed=[name for name,ok in checks if not ok]
for name,ok in checks: print(('PASS: ' if ok else 'FAIL: ')+name)
if failed: sys.exit(1)
print('presentation audio boundary: PASS')
