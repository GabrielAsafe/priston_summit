#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[5]
text = (root / 'Shared/gameplay/character/character_legacy_client.inl').read_text(encoding='utf-8', errors='replace')
start = text.index('\t\tif (PlaySlowCount > 0)')
end = text.index('\n\n\n\n\t\tif (TalkFrame)', start)
block = text[start:end]

# The current main still owns this lifecycle inline. Patch 160 is validation
# repair only, so the guard records the current contract rather than importing
# the donor branch's later StatusEffectLifecycleService/Concealment slices.
for token in (
    'PlaySlowCount--;',
    'if (PlayDistortion > 0) PlayDistortion--;',
    'PlayVanish--;',
    'PlayStunCount--;',
    'if (AttackIce > 0 && AttackIce < 160) AttackIce++;',
    'if (PlayInvincible > 0)\tPlayInvincible--;',
    'PlayPoison[1]--;',
    'if (DispPoison > 0 && DispPoison < 100) DispPoison++;',
):
    assert token in block, f'current client status lifecycle contract changed: {token}'

print('smCHAR client player status lifecycle guard: PASS')
