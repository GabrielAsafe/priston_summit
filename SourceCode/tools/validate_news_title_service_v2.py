#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
cpp=(root/'SrcServer/src/systems/community/news/News.cpp').read_text(errors='replace')
repo=(root/'SrcServer/src/systems/community/news/v2/legacy/LegacyNewsTitleRepository.h').read_text(errors='replace')
service=(root/'Shared/community/v2/NewsTitleService.h').read_text(errors='replace')
start=cpp.index('void News::title()')
end=cpp.index('void News::msgCoinShop()', start)
body=cpp[start:end]
checks={
 'title handler delegates service': 'NewsTitleService::Load' in body,
 'title handler no direct file access': 'File file(' not in body and 'readString(' not in body,
 'legacy repository owns Sistemas.ini': 'Configs//Sistemas.ini' in repo and 'readString("Title", "message")' in repo,
 'green service has repository port': 'class INewsTitleRepository' in service,
 'packet send remains adapter path': 'SummitNewsSend(player, &packet, packet.size)' in body,
}
failed=[name for name,value in checks.items() if not value]
if failed:
    for name in failed: print('FAIL:',name,file=sys.stderr)
    raise SystemExit(1)
print('news title service v2: PASS')
