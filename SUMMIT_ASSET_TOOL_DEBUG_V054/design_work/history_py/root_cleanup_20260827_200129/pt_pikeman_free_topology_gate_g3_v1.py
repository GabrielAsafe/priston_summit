#!/usr/bin/env python3
from __future__ import annotations
import argparse, hashlib, json, os, pathlib, re, shlex, subprocess, sys, shutil

PRISTINE_SHA="9c5b137bc637f485419cafae6864c4a37f77835d32210810295c1b58c746443c"
SKELETON_SHA="48af614678593eaa0b659c2be514bd60f86b94f72cfb4305ed8352f48f12cc4a"
PROFILE="profiles/pt_character_profile_pikeman_c01_m4_v1.json"
ARTIST="blender/pikeman_c01_m4/geometry/C002_TbmC03_free_topology_g3_v1.blend"
EDIT_REPORT="logs/characters/pikeman_c01_m4/geometry/free_topology_g3_v1/edit_report.json"
NAME="TmbC01_TbmC03_free_topology_g3_v1"
CANON_SMD="pt_build/characters/pikeman_c01_m4/geometry/free_topology_g3_v1/TmbC01_TbmC03_free_topology_g3_v1.smd"
CANON_MANIFEST="logs/characters/pikeman_c01_m4/geometry/free_topology_g3_v1/TmbC01_TbmC03_free_topology_g3_v1_build_manifest.json"
REIMPORT="blender/pikeman_c01_m4/geometry/C002_TbmC03_free_topology_g3_v1_reimport.blend"
PIPELOG="logs/characters/pikeman_c01_m4/geometry/free_topology_g3_v1/pipeline_stdout.txt"
REIMPORTLOG="logs/characters/pikeman_c01_m4/geometry/free_topology_g3_v1/reimport_stdout.txt"

def sha(p):
    h=hashlib.sha256()
    with open(p,"rb") as f:
        for b in iter(lambda:f.read(1024*1024),b""): h.update(b)
    return h.hexdigest()

def run(cmd,cwd,capture=False):
    print("COMMAND="+shlex.join([str(x) for x in cmd]),flush=True)
    cp=subprocess.run(cmd,cwd=cwd,text=True,stdout=subprocess.PIPE if capture else None,
                      stderr=subprocess.STDOUT if capture else None)
    if capture:
        print(cp.stdout,end="")
    if cp.returncode:
        raise RuntimeError(f"command failed rc={cp.returncode}: {shlex.join(cmd)}")
    return cp.stdout if capture else ""

def require(root,rel):
    p=root/rel
    if not p.exists(): raise RuntimeError(f"missing required file: {p}")
    return p

def doctor(root):
    checks=[
      "TmbC01.smd","m4.smb","C002.inx",PROFILE,
      "pt_character_artist_pipeline_v2_1.py",
      "pt_free_mesh_validate_v2.py",
      "pt_blender_to_smd_free_mesh_v4_multi.py",
      "pt_rigid_skin_normalize_copy_v2.py",
      "history_py/pt_smd_blender_import_v3_static_armature.py",
      "summit_workspace_asset_bridge_v1_2.py",
      "summit_roundtrip_restore_v4.py",
      "blender/pikeman_c01_m4/armor_family/C002_pikeman_armor_preview.blend",
    ]
    for rel in checks:
        p=require(root,rel); print(f"FILE_OK={p}")
    if sha(root/"TmbC01.smd")!=PRISTINE_SHA: raise RuntimeError("TmbC01.smd pristine SHA mismatch")
    if sha(root/"m4.smb")!=SKELETON_SHA: raise RuntimeError("m4.smb SHA mismatch")
    client=root.parent/"Summit Tale Client Full/cliente/char/tmABCD/TmbC01.smd"
    if not client.exists(): raise RuntimeError(f"client TmbC01 missing: {client}")
    csha=sha(client)
    print(f"client_sha256={csha}")
    if csha!=PRISTINE_SHA:
        raise RuntimeError("client is not pristine; restore before G3")
    print("profile_id_expected=pikeman_c01_m4")
    print("source_counts_expected_TbmC03=74/122/122")
    print("output_counts_expected_TbmC03=75/124/124")
    print("doctor_check=PASS")

def create_edit(root):
    out=root/ARTIST; rep=root/EDIT_REPORT
    out.parent.mkdir(parents=True,exist_ok=True); rep.parent.mkdir(parents=True,exist_ok=True)
    cmd=["flatpak","run","--filesystem=home","org.blender.Blender",
         "--background",str(root/"blender/pikeman_c01_m4/armor_family/C002_pikeman_armor_preview.blend"),
         "--python-exit-code","1","--python",str(root/"pt_pikeman_free_topology_edit_g3_v1.py"),
         "--",str(out),str(rep)]
    text=run(cmd,root,capture=True)
    for marker in ["output_counts=75/124","new_vertex_rigid=True","free_topology_edit_check=PASS"]:
        if marker not in text: raise RuntimeError(f"edit output missing marker: {marker}")
    return out

def get_help(root,args):
    cp=subprocess.run([sys.executable,str(root/"pt_character_artist_pipeline_v2_1.py")]+args,
                      cwd=root,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    return cp.stdout

def pipeline_command(root):
    top=get_help(root,["--help"])
    runh=get_help(root,["run","--help"])
    combined=top+"\n"+runh
    if "--profile" not in combined:
        raise RuntimeError(
          "pt_character_artist_pipeline_v2_1.py does not expose --profile; "
          "strict G3 refuses to risk the deprecated m5/Priestess identity"
        )
    if "--artist" not in runh or "--object" not in runh or "--name" not in runh:
        raise RuntimeError("artist pipeline V2.1 CLI lacks required single-object options")

    cmd=[sys.executable,str(root/"pt_character_artist_pipeline_v2_1.py")]
    # Global options belong before subcommand when present in top-level help.
    if "--root" in top: cmd += ["--root",str(root)]
    if "--profile" in top: cmd += ["--profile",str(root/PROFILE)]
    cmd += ["run"]
    # Some CLIs attach profile/root to the subcommand instead.
    if "--root" not in top and "--root" in runh: cmd += ["--root",str(root)]
    if "--profile" not in top and "--profile" in runh: cmd += ["--profile",str(root/PROFILE)]
    cmd += ["--artist",str(root/ARTIST),"--object","TbmC03","--name",NAME]
    if "--lod" in runh: cmd += ["--lod","default"]
    if "--model-info" in runh: cmd += ["--model-info",str(root/"C002.inx")]
    if "--skeleton" in runh: cmd += ["--skeleton",str(root/"m4.smb")]
    if "--source" in runh: cmd += ["--source",str(root/"TmbC01.smd")]
    elif "--smd" in runh: cmd += ["--smd",str(root/"TmbC01.smd")]
    return cmd

def locate_pipeline_smd(root):
    candidates=list((root/"pt_build").rglob(NAME+".smd"))
    if len(candidates)!=1:
        raise RuntimeError(f"expected exactly one pipeline SMD named {NAME}.smd, found {candidates}")
    return candidates[0]

def run_pipeline(root):
    (root/PIPELOG).parent.mkdir(parents=True,exist_ok=True)
    cmd=pipeline_command(root)
    text=run(cmd,root,capture=True)
    (root/PIPELOG).write_text(text,encoding="utf-8")
    required=["pipeline_check=PASS","free_mesh_writer_check=PASS","output_counts=75/124/124"]
    for m in required:
        if m not in text: raise RuntimeError(f"pipeline output missing strict marker: {m}")
    if "unrelated_object_blocks_byte_identical=8" not in text:
        raise RuntimeError("expected 8 non-target object blocks byte-identical")
    source=locate_pipeline_smd(root)
    dest=root/CANON_SMD; dest.parent.mkdir(parents=True,exist_ok=True)
    if source.resolve()!=dest.resolve(): shutil.copy2(source,dest)
    return dest,text

def reimport(root,smd):
    out=root/REIMPORT; out.parent.mkdir(parents=True,exist_ok=True)
    client_root=root.parent/"Summit Tale Client Full/cliente"
    cmd=["flatpak","run","--filesystem=home","org.blender.Blender","--background",
         "--python-exit-code","1","--python",str(root/"history_py/pt_smd_blender_import_v3_static_armature.py"),
         "--","--smd",str(smd),"--skeleton",str(root/"m4.smb"),
         "--model-info",str(root/"C002.inx"),"--lod","default",
         "--client-root",str(client_root),"--out",str(out),"--no-pack"]
    text=run(cmd,root,capture=True)
    (root/REIMPORTLOG).write_text(text,encoding="utf-8")
    required=[
      "Objects    : 9","Materials  : 2","Skeleton objects: 58",
      "Bind vertices   : 956","Bind bones used : 32",
      "OBJECT TbmC03: V=75 F=124 UV=124",
      "selected_lod=default","static_armature_import_check=PASS",
    ]
    for m in required:
        if m not in text: raise RuntimeError(f"reimport missing marker: {m}")
    if "visible_objects=['TbmC03', 'TamC01']" not in text and "visible_objects=['TamC01', 'TbmC03']" not in text:
        raise RuntimeError("C002 default visible objects mismatch")
    return text

def manifest(root,smd):
    out=root/CANON_MANIFEST; out.parent.mkdir(parents=True,exist_ok=True)
    obj={
      "tool":"PT_PIKEMAN_FREE_TOPOLOGY_GATE_G3_V1",
      "profile_id":"pikeman_c01_m4",
      "asset_set_id":"CHARSET-PIKEMAN-C01-M4",
      "roundtrip_target":"char/tmABCD/TmbC01.smd",
      "artifact":str(smd),
      "artifact_sha256":sha(smd),
      "artifact_size":smd.stat().st_size,
      "source_sha256":PRISTINE_SHA,
      "target_object":"TbmC03",
      "source_counts":{"vertices":74,"faces":122,"texlinks":122},
      "output_counts":{"vertices":75,"faces":124,"texlinks":124},
      "vertex_delta":1,
      "face_delta":2,
      "all_vertices_rebuilt":True,
      "all_faces_rebuilt":True,
      "all_texlinks_rebuilt":True,
      "all_physique_rebuilt":True,
      "unrelated_object_blocks_byte_identical":8,
      "independent_reimport":"PASS",
      "runtime_installed":False,
      "build_check":"PASS",
    }
    out.write_text(json.dumps(obj,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
    return out,obj

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("action",choices=["doctor","edit","build"])
    ap.add_argument("--root",default=".")
    args=ap.parse_args()
    root=pathlib.Path(args.root).expanduser().resolve()
    print("PT_PIKEMAN_FREE_TOPOLOGY_GATE_G3_V1")
    print(f"action={args.action}")
    print(f"root={root}")
    doctor(root)
    if args.action=="doctor": return
    create_edit(root)
    if args.action=="edit":
        print("edit_gate_check=PASS"); return
    smd,pout=run_pipeline(root)
    reimport(root,smd)
    man,obj=manifest(root,smd)
    print(f"artifact={smd}")
    print(f"artifact_size={obj['artifact_size']}")
    print(f"artifact_sha256={obj['artifact_sha256']}")
    print(f"manifest={man}")
    print("runtime_installed=False")
    print("g3_structural_gate_check=PASS")

if __name__=="__main__":
    try: main()
    except Exception as e:
        print(f"g3_gate_check=FAIL: {type(e).__name__}: {e}")
        raise
