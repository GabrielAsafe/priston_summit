#!/usr/bin/env python3
"""
PT SMD Diffuse Color Path Audit V2

Read-only source audit. Focuses only on the proven runtime read:

    SetBlinkColor(sColors, smRendMaterial[MatNum].Diffuse, ...)

and follows:
  smMATERIAL::Diffuse
    -> SetBlinkColor
    -> sColors
    -> render vertex color / D3D diffuse color

No source/client modifications.
"""

from __future__ import annotations

import argparse
import json
import pathlib
import re

TEXT_EXTS={".h",".hpp",".hh",".c",".cc",".cpp",".cxx",".inc",".inl"}
SKIP={"dependency","dependencies","thirdparty","third_party","vendor","vendors",
      "build","builds","bin","obj","node_modules",".git",".svn",".hg"}

PATTERNS=[
    "SetBlinkColor",
    "sColors",
    "smRendMaterial[MatNum].Diffuse",
    "WindMeshBottom",
    "SetD3DRendBuff",
    "SetD3DRendBuff2",
    "SetD3DRendBuff3",
    "bCol",
    "sDef_Color",
    "d3dColor",
    "D3DCOLOR",
    "D3DTA_DIFFUSE",
]

def read_text(p):
    raw=p.read_bytes()
    for enc in ("utf-8","cp1252","latin1"):
        try: return raw.decode(enc)
        except UnicodeDecodeError: pass
    return raw.decode("latin1","replace")

def skip(rel):
    return any(x.lower() in SKIP for x in rel.parts[:-1])

def context(lines, idx, before=24, after=40):
    a=max(0,idx-before); b=min(len(lines),idx+after+1)
    return {
        "match_line":idx+1,
        "start_line":a+1,
        "end_line":b,
        "text":"".join(f"{j+1:6d}: {lines[j]}" for j in range(a,b))
    }

def function_block(lines, idx):
    # Conservative brace-balanced extraction around a line containing a function name.
    start=max(0,idx-8)
    # find probable signature start
    for j in range(idx, max(-1,idx-12), -1):
        s=lines[j]
        if "(" in s and not s.lstrip().startswith(("//","/*","*")):
            start=j
            break
    depth=0; seen=False; end=min(len(lines)-1, idx+180)
    for j in range(start, min(len(lines), idx+220)):
        for c in lines[j]:
            if c=="{":
                depth+=1; seen=True
            elif c=="}" and seen:
                depth-=1
                if depth==0:
                    end=j
                    return {
                        "start_line":start+1,
                        "end_line":end+1,
                        "text":"".join(f"{k+1:6d}: {lines[k]}" for k in range(start,end+1))
                    }
    return context(lines, idx, before=12, after=100)

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--source-root",required=True)
    ap.add_argument("--report",default="logs/pt_smd_diffuse_color_path_audit_v2.json")
    args=ap.parse_args()

    root=pathlib.Path(args.source_root).expanduser().resolve()
    if not root.is_dir(): raise FileNotFoundError(root)

    files_scanned=0
    hits=[]
    setblink_defs=[]
    setblink_calls=[]
    smrend_calls=[]
    color_path_hits=[]

    for p in sorted(root.rglob("*")):
        if not p.is_file() or p.suffix.lower() not in TEXT_EXTS: continue
        rel=p.relative_to(root)
        if skip(rel): continue
        files_scanned+=1
        try: text=read_text(p)
        except OSError: continue
        if not any(x.lower() in text.lower() for x in PATTERNS): continue

        lines=text.splitlines(keepends=True)
        for i,line in enumerate(lines):
            lo=line.lower()
            if "setblinkcolor" in lo:
                rec={"path":str(rel).replace("\\","/"),"line":i+1,
                     "source_line":line.rstrip("\r\n")}
                # definition heuristic: function name and "(" but no ";" before body,
                # and not obviously a call with indentation/assignment.
                window="".join(lines[i:min(len(lines),i+5)])
                if re.search(r"\b(?:void|int|bool|BOOL|DWORD|float|double)\s+SetBlinkColor\s*\(", window, re.I):
                    rec["block"]=function_block(lines,i)
                    setblink_defs.append(rec)
                else:
                    rec["context"]=context(lines,i,30,55)
                    setblink_calls.append(rec)

            if "smrendmaterial[matnum].diffuse" in lo.replace(" ",""):
                smrend_calls.append({
                    "path":str(rel).replace("\\","/"),
                    **context(lines,i,45,70)
                })

            if any(k in lo for k in (
                "scolors","bcol","sdef_color","d3dta_diffuse","d3dcolor",
                "setd3drendbuff"
            )):
                color_path_hits.append({
                    "path":str(rel).replace("\\","/"),
                    "line":i+1,
                    "source_line":line.rstrip("\r\n"),
                    **context(lines,i,8,14)
                })

    # Deduplicate exact function-def duplicates by path/line.
    def uniq(rows,key):
        out=[]; seen=set()
        for r in rows:
            k=key(r)
            if k not in seen:
                seen.add(k); out.append(r)
        return out

    setblink_defs=uniq(setblink_defs,lambda r:(r["path"],r["line"]))
    setblink_calls=uniq(setblink_calls,lambda r:(r["path"],r["line"]))
    smrend_calls=uniq(smrend_calls,lambda r:(r["path"],r["match_line"]))
    color_path_hits=uniq(color_path_hits,lambda r:(r["path"],r["line"]))

    result={
        "tool":"PT_SMD_DIFFUSE_COLOR_PATH_AUDIT_V2",
        "source_root":str(root),
        "text_files_scanned":files_scanned,
        "setblink_definition_count":len(setblink_defs),
        "setblink_call_count":len(setblink_calls),
        "smrend_diffuse_call_context_count":len(smrend_calls),
        "color_path_hit_count":len(color_path_hits),
        "setblink_definitions":setblink_defs,
        "setblink_calls":setblink_calls,
        "smrend_diffuse_call_contexts":smrend_calls,
        "color_path_hits":color_path_hits,
        "source_modified":False,
        "client_modified":False,
        "diffuse_color_path_audit_check":"PASS",
    }

    rp=pathlib.Path(args.report).expanduser()
    if not rp.is_absolute(): rp=pathlib.Path.cwd()/rp
    rp=rp.resolve(); rp.parent.mkdir(parents=True,exist_ok=True)
    rp.write_text(json.dumps(result,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")

    print("PT_SMD_DIFFUSE_COLOR_PATH_AUDIT_V2")
    print(f"source_root={root}")
    print(f"text_files_scanned={files_scanned}")
    print(f"setblink_definition_count={len(setblink_defs)}")
    print(f"setblink_call_count={len(setblink_calls)}")
    print(f"smrend_diffuse_call_context_count={len(smrend_calls)}")
    print(f"color_path_hit_count={len(color_path_hits)}")

    print("\n=== SETBLINKCOLOR DEFINITIONS ===")
    if not setblink_defs: print("NONE")
    for r in setblink_defs[:8]:
        print(f"SOURCE_FILE {r['path']} line={r['line']}")
        print(r["block"]["text"],end="" if r["block"]["text"].endswith("\n") else "\n")

    print("\n=== PROVEN smRendMaterial.Diffuse CALL CONTEXTS ===")
    if not smrend_calls: print("NONE")
    for r in smrend_calls[:8]:
        print(f"SOURCE_FILE {r['path']}")
        print(f"CONTEXT lines={r['start_line']}..{r['end_line']}")
        print(r["text"],end="" if r["text"].endswith("\n") else "\n")

    print("\n=== SETBLINKCOLOR CALLS ===")
    for r in setblink_calls[:20]:
        print(f"SOURCE_FILE {r['path']} line={r['line']}")
        print(r["context"]["text"],end="" if r["context"]["text"].endswith("\n") else "\n")

    print("\n=== COLOR PATH INDEX ===")
    for r in color_path_hits[:160]:
        print(f"{r['path']}:{r['line']}: {r['source_line']}")

    print(f"\nreport={rp}")
    print("source_modified=False")
    print("client_modified=False")
    print("diffuse_color_path_audit_check=PASS")

if __name__=="__main__":
    try: main()
    except Exception as exc:
        print(f"diffuse_color_path_audit_check=FAIL: {type(exc).__name__}: {exc}")
        raise
