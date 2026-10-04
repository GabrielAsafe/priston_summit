#!/usr/bin/env python3
from pathlib import Path
root = Path(__file__).resolve().parents[5]
path = root / 'SrcServer/src/systems/commerce/v2/legacy/LegacyAgingKernel.inl'
text = path.read_text(encoding='latin1', errors='replace')
for required in (
    'AgingResultLogPolicy::Plan',
    'resultLogPlan.logCode',
    'resultLogPlan.announcement',
    'pedraDireto > 0, resultLogConfig',
):
    if required not in text:
        raise SystemExit(f'legacy aging result-log guard: missing {required}')
for stale in (
    'int AgeNum = 0;',
    'AgingLog = ITEMLOG_AGING_1DOWN;',
    'AgingLog = ITEMLOG_AGING_2DOWN;',
    'AgingLog = ITEMLOG_AGING_2UP;',
    'if (AgeNum >= 10)',
):
    if stale in text:
        raise SystemExit(f'legacy aging result-log guard: inline decision remains: {stale}')
print('legacy aging result-log V2 guard: PASS')
