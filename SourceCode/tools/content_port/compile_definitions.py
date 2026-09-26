#!/usr/bin/env python3
from __future__ import annotations
import argparse,base64,hashlib,json,struct
from pathlib import Path, PurePosixPath
from legacy_text import lex_bytes

MAGIC=b'SCFG'; VERSION=1
HEADER=struct.Struct('<4sHH10I32s')
RECORD=struct.Struct('<5I') # path,id,kind,directive_start,directive_count
DIRECTIVE=struct.Struct('<3I') # command bytes string, values bytes string, flags
KIND={'monster_definition':1,'npc_definition':2,'open_item_definition':3}

def norm(s:str)->str:
 p=[]
 for x in s.replace('\\','/').split('/'):
  if not x or x=='.':continue
  if x=='..':
   if p:p.pop()
  else:p.append(x)
 return '/'.join(p)

def runtime_alias(doc:dict)->str:
 candidates=doc.get('runtime_paths',[])
 for x in candidates:
  n=norm(x)
  if not n.casefold().startswith(('servidor/','cliente/')):return n
 return norm(candidates[-1] if candidates else doc['source']['legacy_path'])

def render_values(d:dict)->bytes:
 raws=d.get('raw_values_hex') or []
 quoted=d.get('quoted') or []
 out=[]
 for i,h in enumerate(raws):
  b=bytes.fromhex(h)
  q=(i<len(quoted) and quoted[i]) or any(c in b' \t\r\n' for c in b)
  if q: out.append(b'"'+b+b'"')
  else: out.append(b)
 return b' '.join(out)

def linked_lines(payload:bytes):
 result=[]
 for line in payload.splitlines():
  toks=lex_bytes(line)
  if not toks or not toks[0].raw.startswith(b'*'):continue
  vals=[]
  for t in toks[1:]: vals.append((b'"'+t.raw+b'"') if t.quoted or any(c in b' \t\r\n' for c in t.raw) else t.raw)
  result.append((toks[0].raw,b' '.join(vals),0))
 return result

def main():
 ap=argparse.ArgumentParser();ap.add_argument('canonical',type=Path);ap.add_argument('output',type=Path);a=ap.parse_args()
 root=a.canonical.resolve(); idx=json.loads((root/'index.json').read_text(encoding='utf-8'))['documents']
 docs={}; alias_map={}
 for e in idx:
  d=json.loads((root/e['json_path']).read_text(encoding='utf-8')); docs[d['id']]=d
  for alias in d.get('runtime_paths',[]):alias_map[norm(alias).casefold()]=d
 strings=bytearray();soff={}
 def intern_bytes(b:bytes)->int:
  if b'\0' in b:raise ValueError('NUL in SCFG string')
  if b not in soff:soff[b]=len(strings);strings.extend(b+b'\0')
  return soff[b]
 def intern_text(s:str)->int:return intern_bytes(s.encode('utf-8'))
 directives=[];records=[];flattened_links=0;unresolved_links=[]
 defs=[d for d in docs.values() if d['kind'] in KIND]
 defs.sort(key=lambda d:runtime_alias(d).casefold())
 for d in defs:
  start=len(directives); rpath=runtime_alias(d)
  for directive in d.get('semantic',{}).get('directives',[]):
   key=directive.get('key')
   if key=='linked_file':
    raw=directive.get('raw_values_hex') or []
    if raw:
     rel=bytes.fromhex(raw[0]).decode('latin1')
     # path bytes in this corpus are ASCII-compatible; latin1 preserves bytes.
     parent=PurePosixPath(rpath.replace('\\','/')).parent.as_posix()
     target=norm(parent+'/'+rel).casefold()
     linked=alias_map.get(target)
     if linked:
      payload=base64.b64decode(linked['compatibility']['legacy_payload_base64'])
      for command,values,flags in linked_lines(payload):
       directives.append((intern_bytes(command),intern_bytes(values),flags))
      flattened_links+=1
     else: unresolved_links.append({'id':d['id'],'path':rpath,'target':target})
    break # exact legacy behavior switches file and never returns
   command=bytes.fromhex(directive['legacy_token_hex'])
   directives.append((intern_bytes(command),intern_bytes(render_values(directive)),0))
  records.append((intern_text(rpath),intern_text(d['id']),KIND[d['kind']],start,len(directives)-start))
 header_size=HEADER.size;ro=header_size;rs=RECORD.size;do=ro+len(records)*rs;ds=DIRECTIVE.size;so=do+len(directives)*ds
 body=bytearray()
 for r in records:body.extend(RECORD.pack(*r))
 for r in directives:body.extend(DIRECTIVE.pack(*r))
 body.extend(strings);digest=hashlib.sha256(body).digest()
 header=HEADER.pack(MAGIC,VERSION,0,len(records),ro,rs,len(directives),do,ds,so,len(strings),0,0,digest)
 a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_bytes(header+body)
 report={'format':'SCFG','version':VERSION,'definitions':len(records),'directives':len(directives),'flattened_links':flattened_links,'legacy_missing_links':unresolved_links,'bytes':a.output.stat().st_size,'sha256':hashlib.sha256(a.output.read_bytes()).hexdigest()}
 a.output.with_suffix(a.output.suffix+'.report.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print(json.dumps({k:(len(v) if k=='legacy_missing_links' else v) for k,v in report.items()},ensure_ascii=False))
if __name__=='__main__':main()
