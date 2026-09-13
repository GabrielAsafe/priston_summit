#!/usr/bin/env python3
from pathlib import Path
import re, sys

ROOT = Path(__file__).resolve().parent
SERVER = ROOT / 'Shared/security/integrity/isaocheck_legacy_server.inl'
text = SERVER.read_text(encoding='latin1')

# Only inspect C/C++ string literals; documentation comments may name the
# historical endpoints deliberately.
string_literals = [m.group(0)[1:-1] for m in re.finditer(r'"(?:\\.|[^"\\])*"', text)]
http_literals = [s for s in string_literals if 'http://' in s.lower() or 'https://' in s.lower()]
expected_fragment = 'clanChipChange.asp'
errors = []

if len(http_literals) != 1:
    errors.append(f'expected exactly one HTTP string literal, found {len(http_literals)}: {http_literals}')
elif expected_fragment.lower() not in http_literals[0].lower():
    errors.append(f'unexpected remaining HTTP endpoint: {http_literals[0]}')

for forbidden in [
    'login.php', 'LoginCheck.php', 'InsertUser.php', 'Login_End.php',
    'clan_readClanName.php', 'clan_WarehouseRight.php', 'updateFlag.php',
    '/NPCSound', 'clan_insertClanImage.php', 'ktj_login.php',
]:
    hits=[s for s in string_literals if forbidden.lower() in s.lower()]
    if hits:
        errors.append(f'forbidden legacy web literal {forbidden}: {hits}')

stubs = {
    'BOOL GetHttpRequest(LPSTR httpname, LPSTR Request, DWORD MaxBuffer, int *len)': 'return FALSE;',
    'int ISAOCHECK::connect_WEBdb_1(char *szWriteBuffer, char *szReadBuffer)': 'return ISAO_ERR_TCP_CONNECT;',
    'int ISAOCHECK::connect_WEBdb_BINARY(char *szWriteBuffer, char *szReadBuffer, int *dataSize)': 'return ISAO_ERR_TCP_CONNECT;',
    'int ISAOCHECK::connect_WEBdb_NOTICE(char *szWriteBuffer, char *szReadBuffer)': 'return ISAO_ERR_TCP_CONNECT;',
    'int ISAOCHECK::ReadNotice(char *userid, char *chaname, char *gserver, char *buf)': 'return ISAO_ERR_NONE_REEJCT;',
    'int ISAOCHECK::IsaoCheck(char *id, char *pass, char *ipaddr, long StartTime)': 'return ISAO_REJECT_DB_MAINTE;',
    'int ISAOCHECK::IsaoCheck_GameExit(char *id, char *pass, char *ipaddr, long usertime)': 'return ISAO_ERR_WINSOCK_NOERR;',
    'int ISAOCHECK::read_ClanName(int readClumns)': 'return ISAO_ERR_NONE_REEJCT;',
    'int ISAOCHECK::WarhouseRight(char *clname, char *right_str)': 'return ISAO_ERR_NONE_REEJCT;',
    'int ISAOCHECK::updateFlag(char *clname)': 'return ISAO_ERR_NONE_REEJCT;',
    'int ISAOCHECK::GetWavFile(char* FileName)': 'return ISAO_ERR_NONE_REEJCT;',
    'int ISAOCHECK::write_ClanImageFile(char *fname)': 'return ISAO_ERR_NONE_REEJCT;',
    'int ISAOCHECK::WEBdb_Exam()': 'return ISAO_ERR_NONE_REEJCT;',
}

def body_for(signature):
    start=text.find(signature)
    if start < 0:
        return None
    brace=text.find('{', start)
    depth=0
    i=brace
    while i < len(text):
        if text[i]=='{': depth+=1
        elif text[i]=='}':
            depth-=1
            if depth==0:
                return text[brace+1:i]
        i+=1
    return None

for sig, expected in stubs.items():
    body=body_for(sig)
    if body is None:
        errors.append(f'missing stub: {sig}')
        continue
    if expected not in body:
        errors.append(f'stub missing expected return: {sig}')
    if 'connect_WEBdb(' in body or 'connect_WEBdb_NOTICE(' in body:
        errors.append(f'stub still performs web I/O: {sig}')


# No direct HTTP/HTTPS literals should remain in SrcServer itself.  The sole
# unresolved compatibility URL lives in the server-specific isaocheck bridge.
for src in (ROOT / 'SrcServer').rglob('*'):
    if not src.is_file() or src.suffix.lower() not in {'.c','.cc','.cpp','.cxx','.h','.hpp','.inl'}:
        continue
    try:
        st = src.read_text(encoding='latin1')
    except Exception:
        continue
    lits=[m.group(0)[1:-1] for m in re.finditer(r'"(?:\\.|[^"\\])*"', st)]
    bad=[x for x in lits if 'http://' in x.lower() or 'https://' in x.lower()]
    if bad:
        errors.append(f'direct server HTTP literal(s) in {src.relative_to(ROOT)}: {bad}')

# After disabling dead connectors, WinInet primitives must exist only once each,
# inside connect_WEBdb, which is retained solely for clanChipChange.
for token in ['InternetOpen(', 'InternetOpenUrl(', 'InternetReadFile(']:
    count=text.count(token)
    if count != 1:
        errors.append(f'expected exactly one active {token} occurrence, found {count}')
main_web=body_for('int ISAOCHECK::connect_WEBdb(char *szWriteBuffer, char *szReadBuffer)')
if main_web is None or not all(t in main_web for t in ['InternetOpen(', 'InternetOpenUrl(', 'InternetReadFile(']):
    errors.append('remaining WinInet primitives are not isolated in connect_WEBdb')

# The remaining endpoint is deliberately the one unresolved active clan path.
leave_body=body_for('int ISAOCHECK::LeaveClan(char *clanname, char *chaname, char *gserver)')
if leave_body is None or 'clanChipChange.asp' not in leave_body or 'connect_WEBdb(' not in leave_body:
    errors.append('clanChipChange compatibility path is not preserved as the sole active HTTP endpoint')

if errors:
    print('Server HTTP surface FAIL')
    for e in errors: print(' -', e)
    sys.exit(1)

print('Server HTTP surface PASS:')
print('remaining_http_literals=1 (clanChipChange.asp only)')
print('disabled_dead_legacy_endpoints=login/session-web, notice-web, clan-read-name, warehouse/updateFlag, NPCSound, clan-image-upload, WEBdb_Exam')
print('remaining_wininet_implementation=connect_WEBdb only (for clanChipChange)')
