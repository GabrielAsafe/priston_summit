#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
executor_h = (root / 'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h').read_text(errors='ignore')
executor = (root / 'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
system = (root / 'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
record = (root / 'Shared/persistence/character_record/record_legacy_server.inl').read_text(errors='ignore')

checks = {
    'distinct reserve reject reasons': all(x in executor_h for x in (
        'COMPANION_SKILL_MP_RESERVE', 'COMPANION_SKILL_STAMINA_RESERVE')),
    'attempt exposes reserve telemetry': all(x in executor_h for x in ('manaReserve', 'staminaReserve')),
    'default mana reserve is configurable': '"SkillManaReservePercent", 10' in executor,
    'default stamina reserve is configurable': '"SkillStaminaReservePercent", 10' in executor,
    'reserve is checked after real legacy cost': all(x in executor for x in (
        'caster->smCharInfo.Mana[0] - mana < manaReserve',
        'caster->smCharInfo.Stamina[0] - stamina < staminaReserve')),
    'damage casts preserve reserve': 'skillCode, point, &info, &mana, &stamina, attempt, true' in executor,
    'support/heal can bypass offensive reserve': executor.count('&info, &mana, &stamina, &current, false') >= 4 and
        'SKILL_HEALING, point, &info, &mana, &stamina, attempt, false' in executor,
    'potion status names are explicit': all(x in executor for x in (
        'MONITOR_CONSUMED', 'STACK_CONSUMED', 'EMPTY', 'MONITOR_UNSYNCHRONIZED')),
    'potion diagnostics report source': 'statusName=%s source=%s' in executor,
    'empty/stale potion retries back off': 'status == -5 || status == -12 || status == -13' in executor and 'return 5000U;' in executor,
    'unchanged potion failures are rate-limited': all(x in executor_h for x in (
        'nextPotionDebugTime[3]', 'lastPotionStatus[3]', 'lastPotionTier[3]', 'lastPotionCode[3]')) and
        'runtime.nextPotionDebugTime[type] = now + 5000U' in executor,
    'skill reject logging is keyed by reason and code': all(x in executor_h for x in (
        'nextSkillRejectLogTime[3]', 'lastSkillRejectCode[3]', 'lastSkillRejectReason[3]')) and
        'ShouldLogCompanionSkillReject' in system,
    'skill reject telemetry includes reserves': 'reserveMana=%d' in system and 'reserveStamina=%d' in system,
    'basic attack fallback remains intact': 'EngageTarget(character, target, preferredRange' in system,
    'physical DAT stack remains authoritative': 'recordItem.sItemInfo.PotionCount' in record and '--recordItem.sItemInfo.PotionCount' in record,
    'synchronized PotionMonitor fallback remains available': 'potionMonitorValid' in record and 'lpStatus) *lpStatus = 2' in record,
}

failed = [name for name, ok in checks.items() if not ok]
if failed:
    print('Companion Resource Stability V8: FAIL')
    for name in failed:
        print(' -', name)
    sys.exit(1)

print('Companion Resource Stability V8: PASS (offensive reserve + potion backoff/status + reject log rate-limit)')
