#!/usr/bin/env python3
from pathlib import Path
import re, sys
root=Path(__file__).resolve().parents[1]
ids=(root/'Shared/presentation/v2/PresentationIds.h').read_text()
contract=(root/'Shared/presentation/v2/TextureService.h').read_text()
adapter=(root/'SrcGame/src/systems/presentation/v2/legacy/LegacyTextureAdapter.cpp').read_text()
consumer=(root/'SrcGame/src/systems/gameplay/teleport/ui/TCore.cpp').read_text()
legacy_type=(root/'SrcGame/src/systems/rendering/legacy_3d/smType.h').read_text()
keys=set(re.findall(r'LoadTeleportTexture\("([^"]+)"\)', consumer))
adapter_keys=set(re.findall(r'\{ "(ui\.teleport\.[^"]+)", "game//teleport//', adapter))
checks=[
 ('texture id is logical key', 'struct TextureAssetId' in ids and 'std::string key' in ids),
 ('green texture port exists', 'class ITextureService' in contract and 'TextureHandle Load' in contract),
 ('green handle is renderer opaque', 'struct TextureHandle' in contract and 'IDirect3D' not in contract and 'smMATERIAL' not in contract),
 ('adapter owns material creation', 'CreateTextureMaterial(' in adapter and 'kBlendAlpha = 0x01ul' in adapter),
 ('blend constant matches historical catalog', '#define SMMAT_BLEND_ALPHA' in legacy_type and '0x01' in legacy_type),
 ('teleport uses logical texture loader', 'LoadTeleportTexture("ui.teleport.window")' in consumer),
 ('teleport ui has no direct material creation', 'CreateTextureMaterial(' not in consumer),
 ('teleport catalog has all referenced keys', keys and keys.issubset(adapter_keys)),
 ('teleport catalog migrated broad asset set', len(keys) >= 58 and len(adapter_keys) >= 58),
]
failed=[name for name,ok in checks if not ok]
for name,ok in checks: print(('PASS: ' if ok else 'FAIL: ')+name)
print(f'teleport texture keys: consumer={len(keys)} adapter={len(adapter_keys)}')
if failed: sys.exit(1)
print('presentation texture boundary: PASS')
