from pathlib import Path

ROOT = Path(__file__).resolve().parent
CPP = ROOT / 'SrcServer/src/systems/social/clan/customization/ClanCustom.cpp'
README = ROOT / 'Database/clan_tag_assets/README.txt'
s = CPP.read_text(encoding='utf-8', errors='replace')

required = [
    'Database\\\\clan_tag_assets\\\\%s.bmp',
    'Database\\\\clan_marks\\\\%s.bmp',
    'CopyFileA(sourceMarkPath, destinationMarkPath, FALSE);',
]
missing = [x for x in required if x not in s]
if missing:
    raise SystemExit('Clan tag local storage FAIL: missing ' + repr(missing))

for forbidden in [
    'C:\\\\inetpub\\\\wwwroot\\\\brnxcontenttags',
    'C:\\\\inetpub\\\\wwwroot\\\\ClanContent',
    'system(comando)',
]:
    if forbidden in s:
        raise SystemExit('Clan tag local storage FAIL: legacy webroot dependency remains: ' + forbidden)

if not README.exists():
    raise SystemExit('Clan tag local storage FAIL: README missing')

print('Clan tag local storage PASS: Database/clan_tag_assets -> Database/clan_marks, IIS/webroot-free')
