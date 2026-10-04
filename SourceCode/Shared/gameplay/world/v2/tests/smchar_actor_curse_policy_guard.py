#!/usr/bin/env python3
from pathlib import Path
import re

root = Path(__file__).resolve().parents[5]
for rel, marker in (
    ('SrcServer/src/systems/gameplay/character/core/character.cpp', 'if(!lpTargetChar && lpTargetPlayInfo && cnt)'),
    ('Shared/gameplay/character/character_legacy_client.inl', 'if (!lpTargetChar && lpTargetPlayInfo && cnt)'),
):
    text = (root / rel).read_text(encoding='utf-8', errors='replace')
    if marker not in text:
        raise SystemExit(f'actor curse guard: target block missing in {rel}')
    block = text.split(marker, 1)[1].split(
        'if(smMonsterInfo.SkillRange > 0)' if 'SrcServer' in rel else 'if (smMonsterInfo.SkillRange > 0)', 1
    )[0]

    # The evolved main did not receive the donor branch's ActorCursePolicy.
    # Preserve the observable legacy mapping without pretending that policy
    # already exists; later integration may migrate this independently.
    for required in (
        'switch(smMonsterInfo.SkillCurse)' if 'SrcServer' in rel else 'switch (smMonsterInfo.SkillCurse)',
        'dwCurse_Attack_Param = GetRandomPos(10, 15);',
        'dwCurse_Defence_Param = GetRandomPos(2, 6);',
        'dwCurse_Defence_Param = GetRandomPos(10, 16);',
    ):
        if required not in block:
            raise SystemExit(f'actor curse guard: current curse contract missing in {rel}: {required}')

    # Keep literal ranges attached to their assignments; unrelated visual
    # GetRandomPos calls elsewhere in EventSkill are intentionally ignored.
    for pattern in (
        r'dwCurse_Attack_Param\s*=\s*GetRandomPos\(\s*10\s*,\s*15\s*\)',
        r'dwCurse_Defence_Param\s*=\s*GetRandomPos\(\s*2\s*,\s*6\s*\)',
        r'dwCurse_Defence_Param\s*=\s*GetRandomPos\(\s*10\s*,\s*16\s*\)',
    ):
        if not re.search(pattern, block):
            raise SystemExit(f'actor curse guard: range assignment changed in {rel}: {pattern}')

print('smCHAR actor curse policy guard: PASS (legacy authority on current main)')
