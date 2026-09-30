#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/player/v2/ICharacterRepository.h').read_text()
adapter=(root/'SrcServer/src/systems/gameplay/player/v2/legacy/LegacyCharacterRecordAdapter.cpp').read_text()
consumer=(root/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text()
anchor=consumer.index('if(newLevel != previousLevel)')
end=consumer.index('CompanionDebugLog(', anchor)
block=consumer[anchor:end]
checks=[
 ('green character repository exists', 'class ICharacterRepository' in contract and 'TryLoadProgression' in contract and 'TryLoadSnapshot' in contract),
 ('green record key is pointer free', 'CharacterRecordKey' in contract and 'rsPLAYINFO' not in contract and 'TRANS_RECORD_DATA' not in contract),
 ('green progression dto is legacy free', 'CharacterProgressionRecord' in contract and 'smCharInfo' not in contract),
 ('green persistent snapshot is record free', 'CharacterPersistentSnapshot' in contract and 'CharacterRecordItem' in contract and 'sRECORD_ITEM' not in contract),
 ('adapter owns legacy record load', 'rsLoadAccountCharacterRecord' in adapter and 'TRANS_RECORD_DATA' in adapter),
 ('adapter maps progression fields', all(x in adapter for x in ['StatePoint','Next_Exp','ChangeJob','JobBitMask'])),
 ('adapter owns packed item decode', 'DecodeCompress' in adapter and 'sRECORD_ITEM' in adapter and 'CharacterRecordItem' in adapter),
 ('adapter owns persisted skill translation', 'GetTier5RecordSkillPoint' in adapter and 'bSkillPoint[useSlot & 15]' in adapter),
 ('companion level-up uses repository', 'ServerLegacyCharacterRecordAdapter().TryLoadProgression' in block),
 ('companion level-up no direct record load', 'rsLoadAccountCharacterRecord' not in block and 'TRANS_RECORD_DATA' not in block),
 ('companion equipment snapshot uses repository', 'ServerLegacyCharacterRecordAdapter().TryLoadSnapshot' in consumer),
]

failed=[n for n,ok in checks if not ok]
for n,ok in checks: print(('PASS: ' if ok else 'FAIL: ')+n)
if failed: sys.exit(1)
print('character repository boundary: PASS')
