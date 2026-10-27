#!/usr/bin/env python3
"""350J source checks; successful static checks do not prove Wine runtime."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
s=(r/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text()
b=(r/'Shared/gameplay/companions/v2/CompanionTradeBaseline350J.h').read_text()
a=(r/'Shared/gameplay/companions/v2/CompanionTradeAcceptedRecord350E.h').read_text()
checks={
    'enabled with kill switch': 'SUMMIT_DISABLE_COMPANION_TRADE' in s and 'return !enabled || std::strcmp(enabled,"0")!=0;' in s,
    'accepted save normalization': 'BuildNormalizedRecord350D(' in s and 'rsSaveServerPotion(owner,&save)' in s and 'rsSaveServerForce(owner,&save)' in s,
    'receipt not confused with disk': 'evidence.persistedSequence!=sequence' not in s,
    'no IO before packet admission': s.index('BuildNormalizedRecord350D(')>s.index('void HandleTradeConfirm(rsPLAYINFO*'),
    'complete item images before write': s.index('BuildTradePairedRecordImages(job_.acceptedOwnerRecord')<s.index('MaterializeAcceptedOwner350J('),
    'disk-backed preflight': 'CompanionTradeExactRecordStore::Replace(' in b and 'Read(ownerPath,check)' in b,
    'uncertain preflight holds gate': 'preflightUncertain_=true;' in s and '!host_.PreflightUncertain()' in s,
    'protected two DAT journal worker': 'CompanionTradeDiskJournal journal_;' in s and 'ServerTradeParticipantGate350B().TryReserve(' in s,
    'real persisted legacy witness preserved': 'PublishPersisted(' in a,
    'result delivered after timer': 'DecideReply350C(job,status,live)' in s and 'PumpDetachedTrade350H()' in s,
    'short request handler': 'rsRecordMemoryBuffToFile(' not in s[s.index('void HandleTradeConfirm(rsPLAYINFO*'):s.index('void HandleUIAction',s.index('void HandleTradeConfirm(rsPLAYINFO*'))],
}
for name,good in checks.items():
    print(f'SUMMIT350J_AUDIT {"PASS" if good else "FAIL"} {name}')
if not all(checks.values()):raise SystemExit(1)
print('SUMMIT350J_AUDIT=PASS (structural only)')
