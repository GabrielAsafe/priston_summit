#!/usr/bin/env python3
"""
PT SMD Blink Color Control Source Audit V1

Read-only audit for the control fields required to make smMATERIAL::Diffuse
visible in the character renderer:

  UseState & sMATS_SCRIPT_BLINK_COLOR
  WindMeshBottom -> BlinkStep

The previous audits proved:
  Diffuse = smFCOLOR { float r,g,b }
  SetBlinkColor converts Diffuse to RGB amplitude
  SetBlinkColorVertex adds that RGB to rv->bCol and spills overflow to specular

This audit closes:
  - exact sMATS_SCRIPT_BLINK_COLOR constant value;
  - related sMATS_SCRIPT_* bit definitions;
  - every write/assignment to UseState;
  - every write/assignment to WindMeshBottom;
  - parser/script tokens that set blink color or blink step;
  - actual BlinkStep literals/ranges found in source.

No source/client modification.
"""
from __future__ import annotations

import argparse, json, pathlib, re

EXTS={".h",".hpp",".hh",".c",".cc",".cpp",".cxx",".inc",".inl"}
SKIP={"dependency","dependencies","thirdparty","third_party","vendor","vendors",
      "build","builds","bin","obj","node_modules",".git",".svn",".hg"}

TERMS=[
    "sMATS_SCRIPT_BLINK_COLOR",
    "sMATS_SCRIPT_",
    "BLINK_COLOR",
    "BlinkColor",
    "BlinkStep",
    "WindMeshBottom",
    "UseState",
    "SetBlinkColor",
    "MATERIAL_SCRIPT",
]

def read_text(p):
    raw=p.read_bytes()
    for enc in ("utf-8","cp1252","latin1"):
        try: return raw.decode(enc)
        except UnicodeDecodeError: pass
    return raw.decode("latin1","replace")

def skip(rel):
    return any(x.lower() in SKIP for x in rel.parts[:-1])

def ctx(lines,i,before=14,after=24):
    a=max(0,i-before); b=min(len(lines),i+after+1)
    return {
        "match_line":i+1,
        "start_line":a+1,
        "end_line":b,
        "text":"".join(f"{j+1:6d}: {lines[j]}" for j in range(a,b))
    }

def classify_assignment(line, member):
    # conservative lexical classification of member access / identifier
    low=line.lower()
    pos=low.find(member.lower())
    if pos<0: return "NO_MATCH"
    eq=line.find("=")
    if eq<0 or any(op in line for op in ("==","!=",">=","<=")):
        return "READ_OR_USE"
    return "WRITE_LHS" if pos < eq else "READ_OR_USE"

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--source-root",required=True)
    ap.add_argument("--report",default="logs/pt_smd_blink_color_control_source_audit_v1_1.json")
    args=ap.parse_args()

    root=pathlib.Path(args.source_root).expanduser().resolve()
    if not root.is_dir(): raise FileNotFoundError(root)

    files_scanned=0
    term_hits={t:[] for t in TERMS}
    use_hits=[]
    wind_hits=[]
    constants=[]
    numeric_candidates=[]

    const_re=re.compile(
        r"\b(sMATS_SCRIPT_[A-Za-z0-9_]+)\b\s*(?:=|\\s)\\s*"
        r"(0x[0-9A-Fa-f]+|[0-9]+|\\([^\\n;]+\\)|1\\s*<<\\s*[0-9]+)"
    )
    define_re=re.compile(
        r"^\s*#\s*define\s+(sMATS_SCRIPT_[A-Za-z0-9_]+)\s+(.+?)\s*$"
    )
    blinkstep_num_re=re.compile(
        r"(?:BlinkStep|WindMeshBottom)\s*(?:=|,|\))\s*(-?[0-9]+)"
    )

    for p in sorted(root.rglob("*")):
        if not p.is_file() or p.suffix.lower() not in EXTS: continue
        rel=p.relative_to(root)
        if skip(rel): continue
        files_scanned+=1
        try: text=read_text(p)
        except OSError: continue
        low=text.lower()
        if not any(t.lower() in low for t in TERMS):
            continue

        lines=text.splitlines(keepends=True)
        rels=str(rel).replace("\\","/")

        for i,line in enumerate(lines):
            llo=line.lower()
            for t in TERMS:
                if t.lower() in llo:
                    term_hits[t].append({
                        "path":rels,"line":i+1,
                        "source_line":line.rstrip("\r\n"),
                        **ctx(lines,i)
                    })

            if "usestate" in llo:
                use_hits.append({
                    "path":rels,"line":i+1,
                    "classification":classify_assignment(line,"UseState"),
                    "source_line":line.rstrip("\r\n"),
                    **ctx(lines,i,10,16)
                })

            if "windmeshbottom" in llo:
                wind_hits.append({
                    "path":rels,"line":i+1,
                    "classification":classify_assignment(line,"WindMeshBottom"),
                    "source_line":line.rstrip("\r\n"),
                    **ctx(lines,i,10,16)
                })

            dm=define_re.match(line)
            if dm:
                constants.append({
                    "path":rels,"line":i+1,
                    "name":dm.group(1),"expr":dm.group(2).strip(),
                    **ctx(lines,i,6,10)
                })
            else:
                for cm in const_re.finditer(line):
                    constants.append({
                        "path":rels,"line":i+1,
                        "name":cm.group(1),"expr":cm.group(2).strip(),
                        **ctx(lines,i,6,10)
                    })

            for nm in blinkstep_num_re.finditer(line):
                numeric_candidates.append({
                    "path":rels,"line":i+1,
                    "value":int(nm.group(1)),
                    "source_line":line.rstrip("\r\n"),
                })

    # dedupe helper
    def dedupe(rows,key):
        out=[]; seen=set()
        for r in rows:
            k=key(r)
            if k not in seen:
                seen.add(k); out.append(r)
        return out

    constants=dedupe(constants,lambda r:(r["path"],r["line"],r["name"],r["expr"]))
    use_hits=dedupe(use_hits,lambda r:(r["path"],r["line"],r["source_line"]))
    wind_hits=dedupe(wind_hits,lambda r:(r["path"],r["line"],r["source_line"]))
    numeric_candidates=dedupe(numeric_candidates,lambda r:(r["path"],r["line"],r["value"]))

    result={
        "tool":"PT_SMD_BLINK_COLOR_CONTROL_SOURCE_AUDIT_V1_1",
        "source_root":str(root),
        "text_files_scanned":files_scanned,
        "script_constant_count":len(constants),
        "script_constants":constants,
        "use_state_hit_count":len(use_hits),
        "use_state_hits":use_hits,
        "wind_mesh_bottom_hit_count":len(wind_hits),
        "wind_mesh_bottom_hits":wind_hits,
        "blink_step_numeric_candidate_count":len(numeric_candidates),
        "blink_step_numeric_candidates":numeric_candidates,
        "term_counts":{k:len(v) for k,v in term_hits.items()},
        "term_hits":term_hits,
        "source_modified":False,
        "client_modified":False,
        "blink_color_control_source_audit_check":"PASS",
    }

    rp=pathlib.Path(args.report).expanduser()
    if not rp.is_absolute(): rp=pathlib.Path.cwd()/rp
    rp=rp.resolve(); rp.parent.mkdir(parents=True,exist_ok=True)
    rp.write_text(json.dumps(result,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")

    print("PT_SMD_BLINK_COLOR_CONTROL_SOURCE_AUDIT_V1_1")
    print(f"source_root={root}")
    print(f"text_files_scanned={files_scanned}")
    print(f"script_constant_count={len(constants)}")
    print(f"use_state_hit_count={len(use_hits)}")
    print(f"wind_mesh_bottom_hit_count={len(wind_hits)}")
    print(f"blink_step_numeric_candidate_count={len(numeric_candidates)}")

    print("\n=== sMATS_SCRIPT CONSTANTS ===")
    if not constants: print("NONE")
    for r in constants[:100]:
        print(f"{r['path']}:{r['line']}: {r['name']} = {r['expr']}")

    print("\n=== BLINK COLOR FLAG CONTEXTS ===")
    rows=term_hits["sMATS_SCRIPT_BLINK_COLOR"]
    if not rows: print("NONE")
    for r in rows[:40]:
        print(f"SOURCE_FILE {r['path']} line={r['line']}")
        print(r["text"],end="" if r["text"].endswith("\n") else "\n")

    print("\n=== UseState ACCESSES ===")
    for r in use_hits[:120]:
        print(f"{r['path']}:{r['line']} class={r['classification']}: {r['source_line']}")

    print("\n=== WindMeshBottom ACCESSES ===")
    for r in wind_hits[:120]:
        print(f"{r['path']}:{r['line']} class={r['classification']}: {r['source_line']}")

    print("\n=== BLINK STEP NUMERIC CANDIDATES ===")
    if not numeric_candidates: print("NONE")
    for r in numeric_candidates[:100]:
        print(f"{r['path']}:{r['line']}: value={r['value']} :: {r['source_line']}")

    print("\n=== BLINK / SCRIPT PRIORITY CONTEXTS ===")
    printed=0
    for term in ("BLINK_COLOR","BlinkColor","BlinkStep","MATERIAL_SCRIPT"):
        print(f"TERM {term}: matches={len(term_hits[term])}")
        for r in term_hits[term][:20]:
            print(f"SOURCE_FILE {r['path']} line={r['line']}")
            print(r["text"],end="" if r["text"].endswith("\n") else "\n")
            printed+=1
            if printed>=60: break
        if printed>=60: break

    print(f"\nreport={rp}")
    print("source_modified=False")
    print("client_modified=False")
    print("blink_color_control_source_audit_check=PASS")

if __name__=="__main__":
    try: main()
    except Exception as exc:
        print(f"blink_color_control_source_audit_check=FAIL: {type(exc).__name__}: {exc}")
        raise
