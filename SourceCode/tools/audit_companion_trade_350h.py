#!/usr/bin/env python3
"""SUMMIT350H static safety gates. Does not establish runtime correctness."""
from pathlib import Path
import sys
r=Path(__file__).resolve().parents[1]
c=(r/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text()
on=(r/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
j=(r/'Shared/gameplay/companions/v2/CompanionTradeDetachedJournalWorker350B.h').read_text()
g=(r/'Shared/gameplay/companions/v2/CompanionTradeParticipantGate350B.h').read_text()
q=(r/'Shared/gameplay/companions/v2/CompanionTradeDetachedJobQueue.h').read_text()
checks={
 '350H env compatibility present': 'SUMMIT_350H_DISPOSABLE_DAT_LAB' in c and 'Trade350HLabMode() && TradeWorkerStarted350H().load()' in c,
 'no leaked player pointer inside worker host': 'class DetachedPairHost350H' in c and 'rsPLAYINFO' not in c[c.index('class DetachedPairHost350H'):c.index('class TradeFactory350H')],
 'trusted paths stored by value': 'trustedOwnerPath, trustedCompanionPath' in q and 'job.trustedOwnerPath=a;' in c,
 'prior DAT match before commit': 'if(ownerRaw!=job_.acceptedOwnerRecord) return false;' in c,
 'companion decoded without gated load': 'TryDecodeTradeBufferedPayloadCandidate(companionRaw.data()' in c,
 'backend journal in worker': 'CompanionTradeExactPairBackend backend_;' in c and 'CompanionTradeDiskJournal journal_;' in c,
 'real queue used': 'ServerTradeWorker350H().Enqueue(job)' in c and 'DetachedTradeWorker worker(executor,4)' in c,
 'late completion pump timer': 'PumpDetachedTrade350H();' in on and 'PumpDetachedTrade350H()' in c,
 'join on destroy': 'StopDetachedTrade350H();' in on,
 'pair gate': 'ServerTradeParticipantGate350B().TryReserve(' in c and 'IsReserved(owner)' in c,
 'uncertain seals old session': 'SealCommittedForReconnect(key,job.leaseToken)' in c,
 'nonce mismatch no success': 'sameNonce ? DetachedTradeOutcome::Committed(true)' in j,
 'unknown pending never refusal': 'return DetachedTradeOutcome::Unknown();\n        } else {' in j,
 'external writer accounting': 'failedUnknownWrite_' in g,
 'session validation before result': 'DecideReply350C(job,status,live)' in c,
 'never shuts down connection in ingress': 'shutdown(' not in c[c.index('void HandleTradeConfirm(rsPLAYINFO*'):c.index('void HandleUIAction',c.index('void HandleTradeConfirm(rsPLAYINFO*'))]
}
for name,ok in checks.items():print(f'SUMMIT350H_AUDIT {"PASS" if ok else "FAIL"} {name}')
if not all(checks.values()):sys.exit(1)
print('SUMMIT350H_AUDIT=PASS (structural only; MinGW/Wine and writer coverage NOT proven)')
