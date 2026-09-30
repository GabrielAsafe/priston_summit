#!/usr/bin/env python3
from pathlib import Path
import importlib.util
import sys

root = Path(__file__).resolve().parents[1]
renderer_h = (root / 'SrcGame/src/systems/ui/v2/rendering/Dx9UIRenderer.h').read_text(errors='replace')
renderer_cpp = (root / 'SrcGame/src/systems/ui/v2/rendering/Dx9UIRenderer.cpp').read_text(errors='replace')
guard_path = root / 'tools/architecture/validate_legacy_boundary.py'
spec = importlib.util.spec_from_file_location('legacy_boundary_guard', guard_path)
guard = importlib.util.module_from_spec(spec)
spec.loader.exec_module(guard)
metrics, violations = guard.scan(root, guard.load_allowlist(root / 'tools/architecture/legacy_boundary_allowlist.txt'))

renderer_tokens = ('IDirect3D', 'ID3DX', 'LPDIRECT3D', 'D3DCOLOR', 'D3DRS_', 'D3DFVF_', '<d3d9.h>', '<d3dx9.h>')
checks = [
    ('dx9 public header uses pimpl', 'struct Impl;' in renderer_h and 'std::unique_ptr<Impl>' in renderer_h),
    ('dx9 public header has no renderer handles', all(token not in renderer_h for token in renderer_tokens)),
    ('dx9 backend still owns d3d implementation', '<d3d9.h>' in renderer_cpp and ('IDirect3D' in renderer_cpp or 'LPDIRECT3D' in renderer_cpp)),
    ('dx9 implementation classified amber', 'SrcGame/src/systems/ui/v2/rendering/Dx9UIRenderer.cpp' in guard.AMBER_FILES),
    ('green renderer includes are zero', metrics.get('green_renderer_include_count') == 0),
    ('green renderer symbols are zero', metrics.get('green_renderer_symbol_count') == 0),
    ('no unallowlisted architecture violations', metrics.get('unallowlisted_violations') == 0 and not [v for v in violations if not v[3]]),
]
failed = [name for name, ok in checks if not ok]
for name, ok in checks:
    print(('PASS: ' if ok else 'FAIL: ') + name)
if failed:
    sys.exit(1)
print('presentation renderer boundary v1: PASS')
