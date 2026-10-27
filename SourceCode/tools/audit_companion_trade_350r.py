#!/usr/bin/env python3
from pathlib import Path
from zipfile import ZipFile
import glob, json
root=Path(__file__).resolve().parents[1]
page=(root/'SrcGame/src/systems/ui/v2/pages/CompanionsPage.cpp').read_text()
button=(root/'SrcGame/src/systems/ui/v2/widgets/Button.cpp').read_text()
start=page.index('TextureHandle CompanionItemIcon(')
end=page.index('bool CompanionWarmIcon(',start)
assert 'LoadDibSurfaceOffscreen' not in page[start:end], 'Texture IO in render lookup'
assert 'LoadDibSurfaceOffscreen' in page[end:page.index('const char* TacticName',end)]
assert 'CompanionWarmIcon(' in page[page.index('void CompanionsPage::Update('):page.index('void CompanionsPage::RebuildInventoryRects(')]
assert 'LoadDibSurfaceOffscreen' not in page[page.index('void CompanionsPage::RenderTradePreview'):page.index('void CompanionsPage::ExecuteForSelected')]
assert 'LoadDibSurfaceOffscreen' not in page[page.index('void CompanionsPage::RenderInventoryManager'):page.index('void CompanionsPage::OnRender')]
assert 'button->SetIcon(icon);' in page
# 352A adds an explicit block reason without changing real icon drawing.
assert ('button->SetText(icon.IsValid()?"":"...");' in page or
        ('TradePreviewBlockLabel352A(reason)' in page and 'icon.IsValid()?"":"..."' in page))
assert 'DrawImage(icon_' in button
assert 'selected.index,selected.code,selected.key,selected.checksum' in page
assert 'RequestTradeAttempt(tradePreviewName_' in page
bridge=(root/'SrcGame/src/systems/ui/v2/legacy/LegacyUIV2Bridge.cpp').read_text()
assert 'companionGateway.PumpTradeExecution();' in bridge
assert 'gateway_.PumpTradeExecution()' not in page
assert bridge.count('companionGateway.PumpTradeExecution();')==1
assert 'std::to_string(item.index)' not in page[page.index('void CompanionsPage::RefreshControls'):page.index('void CompanionsPage::Update')]
print('SUMMIT350R_AUDIT=PASS (no image IO in draw, icon grid, unchanged transactional selection)')
