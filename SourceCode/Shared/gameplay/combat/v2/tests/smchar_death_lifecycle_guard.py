#!/usr/bin/env python3
from pathlib import Path
root = Path(__file__).resolve().parents[5]
paths = [
    root / 'SrcServer/src/systems/gameplay/character/core/character.cpp',
    root / 'Shared/gameplay/character/character_legacy_client.inl',
]
for path in paths:
    text = path.read_text(encoding='utf-8', errors='replace')
    attack_start = text.find('int smCHAR::PlayAttack(int EventFlag)')
    assert attack_start >= 0, f'missing PlayAttack: {path}'
    death_start = text.find('if(sinGetLife() <= 0)', attack_start)
    if death_start < 0:
        death_start = text.find('if (sinGetLife() <= 0)', attack_start)
    assert death_start >= 0, f'missing player death branch: {path}'
    death_end = text.find('ReformCharForm();', death_start)
    block = text[death_start:death_end]
    assert 'DeathLifecyclePolicy::ShouldResolveChangeJobArenaDeath(' in block, f'missing ChangeJob arena death policy: {path}'
    assert 'sinBattleNpcResult(0);' in block, f'missing ChangeJob arena side effect: {path}'
print('smCHAR death lifecycle guard: PASS')
