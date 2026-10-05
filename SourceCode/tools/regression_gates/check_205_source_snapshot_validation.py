#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[2]
checks={
 'tools/validate_asset_load_system_v2.py': ('link_script.is_file()', 'omitted from source snapshot'),
 'tools/validate_async_streaming_v3.py': ('def require_if_present', 'omitted from source snapshot'),
 'tools/validate_media_runtime_inventory_v2.py': ('doc_path.is_file()', 'docs/ omitted from source snapshot'),
 'tools/content_port/validate_internal_runtime_contract.py': ('LOCALIZATION_OVERRIDES_ONLY', 'EXECUTABLE_FIRST_WITH_SOURCE_TREE_DEV_FALLBACK'),
 'tools/content_port/validate_runtime_pack_resolution.py': ('DEPLOYMENT_CHECK=SKIP', 'EXECUTABLE_FIRST_WITH_SOURCE_TREE_DEV_FALLBACK'),
 'tools/regression_gates/check_188_merchant_text_wrap.py': ('StatusStartY()', 'DetailStartY()'),
 'tests/assets/asset_load_registry_v2_smoke.cpp': ('AssetLoadRegistry<4>', 'MarkTransientFailure'),
}
errors=[]
for rel,tokens in checks.items():
 p=root/rel
 if not p.is_file(): errors.append('missing validation target: '+rel); continue
 text=p.read_text(encoding='utf-8',errors='replace')
 for token in tokens:
  if token not in text: errors.append(f'{rel}: missing {token}')
if errors:
 print('\n'.join('ERROR: '+e for e in errors)); sys.exit(1)
print('PATCH 205 gate: OK — source-only snapshots retain meaningful validation without requiring omitted deployment artifacts')
