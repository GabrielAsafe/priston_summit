from pathlib import Path
import csv,re,sys
ROOT=Path(__file__).resolve().parent
rows=list(csv.DictReader(open(ROOT/'STRUCTURAL_EXACT_MOVES.tsv',encoding='utf-8'),delimiter='\t'))
inc_re=re.compile(r'^\s*#\s*include\s*"([^"]+)"',re.M)
CLIENT=[ROOT,ROOT/'Shared',ROOT/'Shared2',ROOT/'SrcGame/src',ROOT/'SrcGame/src/systems/rendering/legacy_3d',ROOT/'SrcGame/src/third_party/lua/legacy_src',ROOT/'SrcGame/src/third_party/lua/legacy_src/Lua',ROOT/'modules',ROOT/'modules/File',ROOT/'modules/MD5',ROOT/'modules/Helper',ROOT/'Dependency/DirectX9/Include']
SERVER=[ROOT,ROOT/'SrcServer/src',ROOT/'SrcGame/src',ROOT/'Shared',ROOT/'Shared/Lua',ROOT/'Shared2',ROOT/'modules',ROOT/'modules/File',ROOT/'Dependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Include']
def resolve(base,name,incs):
    for p in [base/name]+[d/name for d in incs]:
        if p.is_file(): return p.resolve()
    return None
out=[]; bad=[]
for r in rows:
    orig=ROOT/r['path']; impl=ROOT/r['implementation']; incs=CLIENT if r['path'].startswith('SrcGame/') else SERVER
    text=impl.read_text(encoding='utf-8',errors='ignore')
    for name in inc_re.findall(text):
        a=resolve(orig.parent,name,incs); b=resolve(impl.parent,name,incs)
        ar=a.relative_to(ROOT).as_posix() if a and str(a).startswith(str(ROOT)) else str(a) if a else ''
        br=b.relative_to(ROOT).as_posix() if b and str(b).startswith(str(ROOT)) else str(b) if b else ''
        status='SAME' if ar==br else ('BOTH_UNRESOLVED' if not ar and not br else 'CHANGED')
        out.append((r['path'],r['implementation'],name,ar,br,status))
        if status=='CHANGED':bad.append(out[-1])
with open(ROOT/'STRUCTURAL_EXACT_INCLUDE_RESOLUTION.tsv','w',newline='',encoding='utf-8') as f:
    w=csv.writer(f,delimiter='\t');w.writerow(['original','implementation','include','from_original','from_moved','status']);w.writerows(out)
print('includes checked',len(out),'changed',len(bad))
for x in bad[:30]:print(x)
if bad: sys.exit(1)
