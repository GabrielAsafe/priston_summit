#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
cpp=(root/'SrcServer/src/systems/community/news/News.cpp').read_text(errors='replace')
repo=(root/'SrcServer/src/systems/community/news/v2/legacy/LegacyNewsNoticeRepository.h').read_text(errors='replace')
service=(root/'Shared/community/v2/NewsFeedService.h').read_text(errors='replace')
start=cpp.index('void News::msgNotice()')
end=cpp.index('void News::msgPlayerLogado', start)
body=cpp[start:end]
checks={
 'news handler delegates feed service': 'NewsFeedService::LoadActiveMessages' in body,
 'news handler no direct SQL': 'SQLConnection::GetConnection' not in body,
 'repository owns NoticeSystem query': 'SELECT * FROM NoticeSystem WHERE Activated=1' in repo,
 'green service owns formatting': 'NewsContentService::FormatNotice' in service,
 'packet send remains legacy adapter path': 'SummitNewsSend(player, &Login, Login.size)' in body,
}
failed=[n for n,v in checks.items() if not v]
if failed:
 for n in failed: print('FAIL:',n,file=sys.stderr)
 raise SystemExit(1)
print('news feed service v2: PASS')
