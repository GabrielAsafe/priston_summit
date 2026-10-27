#!/usr/bin/env python3
from pathlib import Path
import sys
p=Path(sys.argv[1] if len(sys.argv)>1 else '.')/'SourceCode'
trade=(p/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
snap=(p/'Shared/gameplay/companions/v2/CompanionTradeInventorySnapshot.h').read_text()
move=(p/'Shared/gameplay/companions/v2/CompanionTradeItemMove.h').read_text()
revision=(p/'Shared/gameplay/companions/v2/CompanionTradeRevision.h').read_text()
stage=(p/'Shared/gameplay/companions/v2/CompanionTradeLegacyInventoryStage.h').read_text()
concrete=(p/'Shared/gameplay/companions/v2/CompanionTradeConcreteTransfer.h').read_text()
comp=(p/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text()
checks={
 'legacy P2P distinguishes potion-stack quantity': 'exactTradeRequest.potionStacks[cnt]' in trade and '(sinPM1&sinITEM_MASK1)' in trade,
 'exact item transfer remains separate in P2P': 'exactTradeRequest.items[cnt]' in trade,
 'read-only decoder tolerates potion instance absence': '!IsLegacyPotionStack350N(decoded.code)' in snap,
 'uniqueness enforced for nonpotion instances': '!IsLegacyPotionStack350N(item.itemCode)' in snap,
 'exact transfer refuses selecting a potion': 'IsLegacyPotionStack350N(r.itemCode)' in concrete,
 'journal move checks passive stack conservation': 'TradePassiveStacksUnchanged350N(player,next.playerAfter)' in move and 'TradePassiveStacksUnchanged350N(companion,next.companionAfter)' in move,
 'preview hashes complete encoded potion records': 'IsLegacyPotionStack350N(r.itemCode)' in revision and 'r.encodedRecord.size()' in revision,
 'client stage validates slots while accepting potion records': 'IsLegacyPotionStack350N(record.itemCode)' in stage and 'DuplicateSlot' in stage,
 'per-failure reason logged without private details': 'unsafeFlags=%u' in comp and 'selectionReason=%d' in comp,
 'selection reason split without relaxing rules': 'TradeSelectionReject350N::UnsafeOwner' in concrete and 'TradeSelectionReject350N::WrongIdentity' in concrete,
 'UI disallows equipped and potion selection': 'if(item.position!=0 || (item.code&0xff000000u)==0x04000000u)' in (p/'SrcGame/src/systems/ui/v2/pages/CompanionsPage.cpp').read_text(),
 'worker/journal active': 'stage=queued' in comp and 'stage=worker-result status=%d' in comp,
}
for name,ok in checks.items():print(f'SUMMIT350N_AUDIT {"PASS" if ok else "FAIL"} {name}')
if not all(checks.values()):sys.exit(1)
print('SUMMIT350N_AUDIT=PASS (structural; MinGW/Wine not verified)')
