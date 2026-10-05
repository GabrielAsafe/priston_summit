#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[2]
errors=[]
for rel in ('Shared/world/field/field_legacy_client.inl','Shared/world/field/field_legacy_server.inl'):
    text=(root/rel).read_text(encoding='utf-8',errors='replace')
    required=['ResetWarpInteractionLock(bool applyCooldown)','if (!lpLastWarpField || dwFieldCode < 0)','ResetWarpInteractionLock(true);','if (dist > DIST_TRANSLEVEL_LOW)']
    for token in required:
        if token not in text: errors.append(f'{rel}: missing {token}')
    if 'if (dwFieldCode < 0) return FALSE;' in text: errors.append(f'{rel}: stale cancel path can retain lock')
if errors: print('\n'.join(errors),file=sys.stderr); sys.exit(1)
print('PATCH 179 gate: OK')
