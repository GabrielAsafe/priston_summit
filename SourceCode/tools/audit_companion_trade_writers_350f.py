#!/usr/bin/env python3
"""Summit 350F: source-level guard check, NOT a proof of full DAT exclusivity.

Exit nonzero when one of the six instrumented server functions loses the
admission marker. Lists known uncovered legacy writers as explicit blockers.
Never reads DATs, accounts, journals, or external files.
"""
from pathlib import Path
import argparse
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'Shared/persistence/character_record/record_legacy_server.inl'
REQUIRED = (
    'int rsRECORD_DBASE::RecvRecordDataFromClient(',
    'int rsRecordMemoryBuffToFile(',
    'int rsSetOfflineCompanionEquipmentSlot(',
    'int rsMutateOfflineCompanionBag(',
    'int rsAddOfflineCharacterExp(',
    'int rsConsumeOfflineCharacterPotion(',
)
# All of these need a future complete migration or a proven transaction gate.
UNCOVERED = (
    'rsRECORD_DBASE::SaveThrowData (legacy direct DAT writer without account-scoped gate)',
    'rsRECORD_DBASE::InsertCharData / DeleteCharData (character lifecycle)',
    'ImportTTServerUser (cross-server record importer)',
    'Any uninstrumented record writer in other translation units',
    'Durable worker commit, session result pump, journal recovery integration',
)

def check(source: str) -> bool:
    lines = source.splitlines()
    found = []
    for head in REQUIRED:
        hits = [i for i, line in enumerate(lines) if head in line]
        if len(hits) != 1:
            print('SUMMIT350F_GUARD=MISSING_FUNCTION ' + head)
            return False
        begin = hits[0]
        # The guard must occur immediately after admission/ownership checks,
        # before any read/write branch. The first 45 lines cover each function.
        snippet = '\n'.join(lines[begin:begin+45])
        if ('TradeParticipantGate350B::WriteGuard' not in snippet or
                'summit350f' not in snippet or
                'Allowed()' not in snippet):
            print('SUMMIT350F_GUARD=MISSING ' + head)
            return False
        found.append(head)
    # Deferred writes do not use an RAII guard on the stack. Instead they
    # acquire a queue counter at admission and release it only after flush.
    for marker in ('BeginQueuedWrite(participant)',
                   'FinishQueuedWrite(recDataBuff[cnt].summit350fParticipantId,',
                   'entry.summit350fParticipantId=participant;'):
        if marker not in source:
            print('SUMMIT350F_QUEUE_GUARD=MISSING '+marker)
            return False
    print('SUMMIT350F_GUARDS=PASS ('+str(len(found))+' synchronous paths + deferred queue)')
    return True

if __name__ == '__main__':
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--source', type=Path, default=SOURCE)
    opts = ap.parse_args()
    data = opts.source.read_bytes().decode('latin1')
    if not check(data):
        sys.exit(2)
    for item in UNCOVERED:
        print('SUMMIT350F_PENDING=' + item)
    print('SUMMIT350F_REAL_TRADE=DISABLED (-76)')
