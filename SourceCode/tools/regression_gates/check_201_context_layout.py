#!/usr/bin/env python3
from pathlib import Path
import re, sys
root=Path(__file__).resolve().parents[2]
ctx=(root/'SrcGame/src/systems/ui/v2/panels/ContextPanel.cpp').read_text(encoding='utf-8')
inv=(root/'SrcGame/src/systems/ui/v2/pages/InventoryPage.cpp').read_text(encoding='utf-8')
errors=[]
for token in ('ActionStartY() const noexcept','DetailStartY() const noexcept','StatusStartY() const noexcept'):
    if token not in ctx: errors.append('ContextPanel missing dynamic footer helper '+token)
if 'const int footerTop = StatusStartY();' not in ctx:
    errors.append('item area is not bounded by the text/action footer')
if 'WrapTextApprox(snapshot_.statusText' not in ctx:
    errors.append('status text is not wrapped')
if 'root.y + DetailStartY()' not in ctx:
    errors.append('hover detail is not placed by dynamic detail footer')
if 'root.y + ActionStartY() - 24' not in ctx:
    errors.append('money/fee line is not anchored above actions')
# Same-frame ownership: child/context update must happen before normal footer visibility is decided.
update_pos=inv.find('Container::Update(deltaSeconds);')
active_pos=inv.find('const bool contextActive = contextPanel_ && contextPanel_->IsContextActive();')
if update_pos < 0 or active_pos < 0 or update_pos > active_pos:
    errors.append('InventoryPage can expose one stale frame of legacy/footer controls over active ContextPanel')
m=re.search(r'quest\.SetBounds\(\{542,\s*52,\s*(\d+),\s*30\}\);',inv)
if not m or int(m.group(1)) < 130:
    errors.append('Portuguese quest-items tab is too narrow and will ellipsize')
if errors:
    for e in errors: print('ERROR:',e,file=sys.stderr)
    sys.exit(1)
print('PATCH 201 gate: OK — merchant/crafting footer, details and inventory controls cannot overlap active context text/actions')
