#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[1]
cpp = (root / 'SrcServer/src/systems/community/news/News.cpp').read_text(errors='replace')
feed = (root / 'Shared/community/v2/NewsFeedService.h').read_text(errors='replace')

for req in ('NewsContentService::FormatName', 'NewsContentService::FormatNumber'):
    if req not in cpp:
        raise SystemExit(f'missing V2 news content path: {req}')
if 'NewsFeedService::LoadActiveMessages' not in cpp:
    raise SystemExit('News handler is not delegating notice read model to NewsFeedService')
if 'NewsContentService::FormatNotice' not in feed:
    raise SystemExit('NewsFeedService is not delegating notice formatting to NewsContentService')
if 'wsprintf(Login.szMessage, szMessageLogin' in cpp:
    raise SystemExit('legacy DB notice formatting authority remains')
if 'SummitNewsSend' not in cpp:
    raise SystemExit('existing News transport adapter unexpectedly removed')
print('news content service validator: PASS')
