#!/usr/bin/env python3
from __future__ import annotations
import argparse, hashlib, json, pathlib, shlex, subprocess, sys, shutil

PRISTINE_SHA="9c5b137bc637f485419cafae6864c4a37f77835d32210810295c1b58c746443c"
SKELETON_SHA="48af614678593eaa0b659c2be514bd60f86b94f72cfb4305ed8352f48f12cc4a"
NAME="TmbC01_TbmC03_artist_remodel_g4_v1"
PROFILE="profiles/pt_character_profile_pikeman_c01_m4_v1.json"
PREVIEW="blender/pikeman_c01_m4/armor_family/C002_pikeman_armor_preview.blend"
ARTIST="blender/pikeman_c01_m4/geometry/C002_TbmC03_artist_remodel_g4_v1.blend"
EDIT_REPORT="logs/characters/pikeman_c01_m4/geometry/artist_remodel_g4_v1/edit_report.json"
PIPE_SMD=f"pt_build/characters/pikeman_c01_m4/TbmC03/{NAME}.smd"
CANON_DIR="pt_build/characters/pikeman_c01_m4/geometry/artist_remodel_g4_v1"
CANON_SMD=f"{CANON_DIR}/{NAME}.smd"
REIMPORT="blender/pikeman_c01_m4/geometry/C002_TbmC03_artist_remodel_g4_v1_reimport.blend"
LOGDIR="logs/characters/pikeman_c01_m4/geometry/artist_remodel_g4_v1"
MANIFEST=f"{LOGDIR}/{NAME}_build_manifest.json"
EXPECTED_SIZE=189040

def sha(p):
    h=hashlib.sha256()
    with open(p,"rb") as f:
        for b in iter(lambda:f.read(1024*1024),b""): h.update(b)
    return h.hexdigest()

def run(cmd,cwd):
    print("COMMAND="+shlex.join([str(x) for x in cmd]),flush=True)
    cp=subprocess.run(cmd,cwd=cwd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    print(cp.stdout,end="")
    if cp.returncode:
        raise RuntimeError(f"command failed rc={cp.returncode}")
    return cp.stdout

def need(root,rel):
    p=root/rel
    if not p.exists(): raise RuntimeError(f"missing required file: {p}")
    print(f"FILE_OK={p}")
    return p

def doctor(root):
    for rel in [
      "TmbC01.smd","m4.smb","C002.inx",PROFILE,PREVIEW,
      "pt_pikeman_artist_remodel_edit_g4_v1.py",
      "pt_character_artist_pipeline_v2_1.py",
      "pt_blender_to_smd_free_mesh_v3.py",
      "pt_free_mesh_validate_v2.py",
      "pt_rigid_skin_normalize_copy_v2.py",
      "history_py/pt_smd_blender_import_v3_static_armature.py",
      "summit_workspace_asset_bridge_v1_2.py",
      "summit_roundtrip_restore_v4.py",
      "pt_pikeman_smd_material_audit_v1.py",
    ]: need(root,rel)
    if sha(root/"TmbC01.smd") != PRISTINE_SHA:
        raise RuntimeError("TmbC01.smd pristine SHA mismatch")
    if sha(root/"m4.smb") != SKELETON_SHA:
        raise RuntimeError("m4.smb SHA mismatch")
    client=root.parent/"Summit Tale Client Full/cliente/char/tmABCD/TmbC01.smd"
    if not client.exists(): raise RuntimeError(f"client model missing: {client}")
    csha=sha(client); print(f"client_sha256={csha}")
    if csha != PRISTINE_SHA:
        raise RuntimeError("client is not pristine; restore G3 witness first")
    print("profile_id_expected=pikeman_c01_m4")
    print("source_counts_expected_TbmC03=74/122/122")
    print("output_counts_expected_TbmC03=82/138/138")
    print(f"output_size_expected={EXPECTED_SIZE}")
    print("doctor_check=PASS")

def edit(root):
    out=root/ARTIST; rep=root/EDIT_REPORT
    out.parent.mkdir(parents=True,exist_ok=True); rep.parent.mkdir(parents=True,exist_ok=True)
    text=run([
      "flatpak","run","--filesystem=home","org.blender.Blender",
      "--background",str(root/PREVIEW),"--python-exit-code","1",
      "--python",str(root/"pt_pikeman_artist_remodel_edit_g4_v1.py"),
      "--",str(out),str(rep)
    ],root)
    for m in [
      "output_counts=82/138","vertex_delta=+8","face_delta=+16",
      "old_vertex_weight_max_error=0","new_vertices_rigid=True",
      "nonmanifold_edge_count=1","artist_remodel_edit_check=PASS"
    ]:
        if m not in text: raise RuntimeError(f"edit missing marker: {m}")

def pipeline(root):
    text=run([
      sys.executable,str(root/"pt_character_artist_pipeline_v2_1.py"),
      "--root",str(root),"run",
      "--profile",str(root/PROFILE),
      "--artist",str(root/ARTIST),
      "--object","TbmC03","--name",NAME,"--lod","default"
    ],root)
    for m in [
      "output_counts=82/138/138",
      "all_vertices_rebuilt=True","all_faces_rebuilt=True",
      "all_texlinks_rebuilt=True","all_physique_rebuilt=True",
      "unrelated_object_blocks_byte_identical=8",
      "free_mesh_writer_check=PASS","pipeline_check=PASS"
    ]:
        if m not in text: raise RuntimeError(f"pipeline missing marker: {m}")
    smd=root/PIPE_SMD
    if not smd.exists(): raise RuntimeError(f"pipeline SMD missing: {smd}")
    if smd.stat().st_size != EXPECTED_SIZE:
        raise RuntimeError(f"unexpected SMD size {smd.stat().st_size}, expected {EXPECTED_SIZE}")
    dest=root/CANON_SMD; dest.parent.mkdir(parents=True,exist_ok=True)
    shutil.copy2(smd,dest)
    (root/LOGDIR).mkdir(parents=True,exist_ok=True)
    (root/LOGDIR/"pipeline_stdout.txt").write_text(text,encoding="utf-8")
    return dest

def reimport(root,smd):
    client_root=root.parent/"Summit Tale Client Full/cliente"
    out=root/REIMPORT; out.parent.mkdir(parents=True,exist_ok=True)
    text=run([
      "flatpak","run","--filesystem=home","org.blender.Blender","--background",
      "--python-exit-code","1",
      "--python",str(root/"history_py/pt_smd_blender_import_v3_static_armature.py"),
      "--","--smd",str(smd),"--skeleton",str(root/"m4.smb"),
      "--model-info",str(root/"C002.inx"),"--lod","default",
      "--client-root",str(client_root),"--out",str(out),"--no-pack"
    ],root)
    for m in [
      "Objects    : 9","Materials  : 2","Skeleton objects: 58",
      "Bind vertices   : 963","Bind bones used : 32",
      "OBJECT TbmC03: V=82 F=138 UV=138",
      "selected_lod=default","static_armature_import_check=PASS"
    ]:
        if m not in text: raise RuntimeError(f"reimport missing marker: {m}")
    if "visible_objects=['TbmC03', 'TamC01']" not in text and "visible_objects=['TamC01', 'TbmC03']" not in text:
        raise RuntimeError("C002 DefaultModel selection mismatch")
    (root/LOGDIR/"reimport_stdout.txt").write_text(text,encoding="utf-8")

def material_audit(root,smd):
    report=root/LOGDIR/"material_audit.json"
    text=run([
      sys.executable,str(root/"pt_pikeman_smd_material_audit_v1.py"),
      "--smd",str(smd),"--report",str(report)
    ],root)
    if "material_audit_check=PASS" not in text:
        raise RuntimeError("material audit failed")
    return report

def manifest(root,smd):
    obj={
      "tool":"PT_PIKEMAN_ARTIST_REMODEL_GATE_G4_V1",
      "profile_id":"pikeman_c01_m4",
      "asset_set_id":"CHARSET-PIKEMAN-C01-M4",
      "roundtrip_target":"char/tmABCD/TmbC01.smd",
      "artifact":str(smd.resolve()),
      "artifact_sha256":sha(smd),
      "artifact_size":smd.stat().st_size,
      "source_sha256":PRISTINE_SHA,
      "target_object":"TbmC03",
      "source_counts":{"vertices":74,"faces":122,"texlinks":122},
      "output_counts":{"vertices":82,"faces":138,"texlinks":138},
      "vertex_delta":8,"face_delta":16,
      "patch_face_count":8,
      "all_vertices_rebuilt":True,
      "all_faces_rebuilt":True,
      "all_texlinks_rebuilt":True,
      "all_physique_rebuilt":True,
      "unrelated_object_blocks_byte_identical":8,
      "independent_C002_reimport":"PASS",
      "material_audit":"PASS",
      "runtime_installed":False,
      "build_check":"PASS"
    }
    p=root/MANIFEST; p.parent.mkdir(parents=True,exist_ok=True)
    p.write_text(json.dumps(obj,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
    return p,obj

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("action",choices=["doctor","edit","build"])
    ap.add_argument("--root",default=".")
    a=ap.parse_args(); root=pathlib.Path(a.root).expanduser().resolve()
    print("PT_PIKEMAN_ARTIST_REMODEL_GATE_G4_V1")
    print(f"action={a.action}"); print(f"root={root}")
    doctor(root)
    if a.action=="doctor": return
    edit(root)
    if a.action=="edit":
        print("g4_edit_gate_check=PASS"); return
    smd=pipeline(root)
    reimport(root,smd)
    material_audit(root,smd)
    man,obj=manifest(root,smd)
    print(f"artifact={smd}")
    print(f"artifact_size={obj['artifact_size']}")
    print(f"artifact_sha256={obj['artifact_sha256']}")
    print(f"manifest={man}")
    print("runtime_installed=False")
    print("g4_structural_gate_check=PASS")

if __name__=="__main__":
    try: main()
    except Exception as e:
        print(f"g4_gate_check=FAIL: {type(e).__name__}: {e}")
        raise
