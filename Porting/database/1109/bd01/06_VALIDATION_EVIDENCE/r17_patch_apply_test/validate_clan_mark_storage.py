#!/usr/bin/env python3
from pathlib import Path
import re

ROOT=Path(__file__).resolve().parent
SRC=ROOT/'Shared/security/integrity/isaocheck_legacy_server.inl'
text=SRC.read_text(errors='replace')

def body(signature):
    start=text.index(signature)
    b=text.index('{',start)
    depth=0
    for i in range(b,len(text)):
        if text[i]=='{': depth+=1
        elif text[i]=='}':
            depth-=1
            if depth==0: return text[start:i+1]
    raise RuntimeError(signature)

helper=body('static BOOL ReadLocalClanMarkBmp')
read1=body('int ISAOCHECK::read_ClanImage(char *clanname, char *filename)')
read2=body('int ISAOCHECK::read_ClanImage2(char *clanname, char *filename, CLANINFO *ci)')

checks={
    'local path': 'Database\\\\clan_marks\\\\%s.bmp' in helper,
    'missing does not create': 'OPEN_EXISTING' in helper and 'OPEN_ALWAYS' not in helper,
    'bounded to legacy buffer': 'fileSize > maxBuffer' in helper,
    'bmp signature': "buffer[0] == 'B' && buffer[1] == 'M'" in helper,
    'path traversal guard': "*p == '\\\\'" in helper and "*p == '/'" in helper and "*p == ':'" in helper,
    'read_ClanImage local': 'ReadLocalClanMarkBmp' in read1 and 'http://' not in read1 and 'GetHttpRequest' not in read1,
    'read_ClanImage2 local': 'ReadLocalClanMarkBmp' in read2 and 'http://' not in read2 and 'GetHttpRequest' not in read2,
    'legacy result 2 on missing': re.search(r'if\(!ReadLocalClanMarkBmp[^\n]*\n\s*return 2;',read1) is not None and re.search(r'if\(!ReadLocalClanMarkBmp[^\n]*\n\s*return 2;',read2) is not None,
    'storage README': (ROOT/'Database/clan_marks/README.txt').exists(),
}
failed=[k for k,v in checks.items() if not v]
if failed:
    print('Clan mark local storage FAIL: '+', '.join(failed))
    raise SystemExit(1)
print('Clan mark local storage PASS: Database/clan_marks, bounded BMP reads, HTTP-free active mark reads')
