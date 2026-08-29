#!/usr/bin/env python3
from __future__ import annotations
import argparse, hashlib, json, pathlib, re, shlex, shutil, subprocess, sys

PRISTINE_SHA="9c5b137bc637f485419cafae6864c4a37f77835d32210810295c1b58c746443c"
SKELETON_SHA="48af614678593eaa0b659c2be514bd60f86b94f72cfb4305ed8352f48f12cc4a"
PROFILE="profiles/pt_character_profile_pikeman_c01_m4_v1.json"
BASELINE="blender/pikeman_c01_m4/armor_family/C002_pikeman_armor_preview.blend"
ARTIST="blender/pikeman_c01_m4/artist/C002_TbmC03_artist_master_g5_v1.blend"
NAME="TmbC01_TbmC03_manual_artist_g5_v1"
LOGDIR="logs/characters/pikeman_c01_m4/geometry/manual_artist_g5_v1"
INSPECT_REPORT=f"{LOGDIR}/artist_inspect.json"
PIPE_SMD=f"pt_build/characters/pikeman_c01_m4/TbmC03/{NAME}.smd"
CANON_DIR="pt_build/characters/pikeman_c01_m4/geometry/manual_artist_g5_v1"
CANON_SMD=f"{CANON_DIR}/{NAME}.smd"
REIMPORT="blender/pikeman_c01_m4/artist/C002_TbmC03_manual_artist_g5_v1_reimport.blend"
MANIFEST=f"{LOGDIR}/{NAME}_build_manifest.json"

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
      "TmbC01.smd","m4.smb","C002.inx",PROFILE,BASELINE,ARTIST,
      "pt_pikeman_manual_artist_inspect_g5_v1.py",
      "pt_character_artist_pipeline_v2_1.py",
      "pt_blender_to_smd_free_mesh_v3.py",
      "pt_free_mesh_validate_v2.py",
      "pt_rigid_skin_normalize_copy_v2.py",
      "history_py/pt_smd_blender_import_v3_static_armature.py",
      "pt_pikeman_smd_material_audit_v1.py",
      "summit_workspace_asset_bridge_v1_2.py",
      "summit_roundtrip_restore_v4.py",
    ]: need(root,rel)
    if sha(root/"TmbC01.smd")!=PRISTINE_SHA:
        raise RuntimeError("root TmbC01.smd is not pristine")
    if sha(root/"m4.smb")!=SKELETON_SHA:
        raise RuntimeError("m4.smb SHA mismatch")
    client=root.parent/"Summit Tale Client Full/cliente/char/tmABCD/TmbC01.smd"
    if not client.exists(): raise RuntimeError(f"client model missing: {client}")
    csha=sha(client); print(f"client_sha256={csha}")
    if csha!=PRISTINE_SHA:
        raise RuntimeError("client is not pristine; restore G4 before G5")
    print(f"artist={root/ARTIST}")
    print(f"artist_sha256={sha(root/ARTIST)}")
    print("profile_id_expected=pikeman_c01_m4")
    print("target_object=TbmC03")
    print("doctor_check=PASS")

def inspect_artist(root):
    report=root/INSPECT_REPORT; report.parent.mkdir(parents=True,exist_ok=True)
    text=run([
      "flatpak","run","--filesystem=home","org.blender.Blender",
      "--background",str(root/ARTIST),"--python-exit-code","1",
      "--python",str(root/"pt_pikeman_manual_artist_inspect_g5_v1.py"),
      "--",str(root/BASELINE),str(report)
    ],root)
    for m in [
      "target=TbmC03","target_changed=True","used_materials=[0]",
      "uv_layer='SMD_UV'","vertex_group_count=11",
      "non_target_unchanged_count=8","manual_artist_inspect_check=PASS"
    ]:
        if m not in text: raise RuntimeError(f"artist inspect missing marker: {m}")
    data=json.loads(report.read_text(encoding="utf-8"))
    return data,text

def pipeline(root):
    text=run([
      sys.executable,str(root/"pt_character_artist_pipeline_v2_1.py"),
      "--root",str(root),"run","--profile",str(root/PROFILE),
      "--artist",str(root/ARTIST),"--object","TbmC03",
      "--name",NAME,"--lod","default"
    ],root)
    for m in [
      "all_vertices_rebuilt=True","all_faces_rebuilt=True",
      "all_texlinks_rebuilt=True","all_physique_rebuilt=True",
      "unrelated_object_blocks_byte_identical=8",
      "free_mesh_writer_check=PASS","pipeline_check=PASS"
    ]:
        if m not in text: raise RuntimeError(f"pipeline missing marker: {m}")
    mo=re.findall(r"output_counts=(\d+)/(\d+)/(\d+)",text)
    if not mo: raise RuntimeError("could not parse pipeline output_counts")
    v,f,t=map(int,mo[-1])
    if t!=f:
        raise RuntimeError(f"expected one TexLink per exported face, got {v}/{f}/{t}")
    smd=root/PIPE_SMD
    if not smd.exists(): raise RuntimeError(f"pipeline SMD missing: {smd}")
    dest=root/CANON_SMD; dest.parent.mkdir(parents=True,exist_ok=True)
    shutil.copy2(smd,dest)
    (root/LOGDIR/"pipeline_stdout.txt").write_text(text,encoding="utf-8")
    return dest,(v,f,t),text

def reimport(root,smd,counts):
    v,f,t=counts
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
    required=[
      "Objects    : 9","Materials  : 2","Skeleton objects: 58",
      "Bind bones used : 32",
      f"OBJECT TbmC03: V={v} F={f} UV={t}",
      "selected_lod=default","static_armature_import_check=PASS"
    ]
    for m in required:
        if m not in text: raise RuntimeError(f"C002 reimport missing marker: {m}")
    if "visible_objects=['TbmC03', 'TamC01']" not in text and "visible_objects=['TamC01', 'TbmC03']" not in text:
        raise RuntimeError("C002 DefaultModel selection mismatch")
    mv=re.search(r"Bind vertices\s*:\s*(\d+)",text)
    if not mv: raise RuntimeError("could not parse total bind vertices")
    bind_vertices=int(mv.group(1))
    expected=881+v
    if bind_vertices!=expected:
        raise RuntimeError(f"unexpected total bind vertices {bind_vertices}, expected {expected}")
    (root/LOGDIR/"reimport_stdout.txt").write_text(text,encoding="utf-8")
    return bind_vertices,text

def material_audit(root,smd):
    report=root/LOGDIR/"material_audit.json"
    text=run([
      sys.executable,str(root/"pt_pikeman_smd_material_audit_v1.py"),
      "--smd",str(smd),"--report",str(report)
    ],root)
    if "material_audit_check=PASS" not in text:
        raise RuntimeError("material audit failed")
    # Preserve source material semantics exactly.
    for marker in [
      "textures=[{'Name': 'char\\\\tmABCD\\\\TmbC03.bmp', 'NameA': ''}]",
      "textures=[{'Name': 'char\\\\tmABCD\\\\TmbC01.bmp', 'NameA': ''}]",
      "BlendType=1 Shade=1 TwoSide=0",
      "SelfIllum_raw_i32=0",
      "TextureClip=0 UseState=0 MeshState=1 WindMeshBottom=0",
    ]:
        if marker not in text:
            raise RuntimeError(f"material audit baseline marker missing: {marker}")
    return report,text

def manifest(root,smd,counts,inspect_data,bind_vertices):
    v,f,t=counts
    srcv=inspect_data["source_counts"]["vertices"]
    srcf=inspect_data["source_counts"]["faces"]
    obj={
      "tool":"PT_PIKEMAN_MANUAL_ARTIST_GATE_G5_V1",
      "profile_id":"pikeman_c01_m4",
      "asset_set_id":"CHARSET-PIKEMAN-C01-M4",
      "roundtrip_target":"char/tmABCD/TmbC01.smd",
      "artist_master":str((root/ARTIST).resolve()),
      "artist_master_sha256":sha(root/ARTIST),
      "artifact":str(smd.resolve()),
      "artifact_sha256":sha(smd),
      "artifact_size":smd.stat().st_size,
      "source_sha256":PRISTINE_SHA,
      "target_object":"TbmC03",
      "source_counts":{"vertices":srcv,"faces":srcf,"texlinks":srcf},
      "output_counts":{"vertices":v,"faces":f,"texlinks":t},
      "vertex_delta":v-srcv,
      "face_delta":f-srcf,
      "manual_target_changed":True,
      "manual_topology_changed":bool(inspect_data["topology_changed"]),
      "manual_positions_changed":bool(inspect_data["positions_changed"]),
      "manual_uv_changed":bool(inspect_data["uv_changed"]),
      "manual_weights_changed":bool(inspect_data["weights_changed"]),
      "artist_non_target_unchanged_count":8,
      "all_vertices_rebuilt":True,
      "all_faces_rebuilt":True,
      "all_texlinks_rebuilt":True,
      "all_physique_rebuilt":True,
      "unrelated_object_blocks_byte_identical":8,
      "independent_C002_reimport":"PASS",
      "bind_vertices":bind_vertices,
      "bind_bones_used":32,
      "material_audit":"PASS",
      "runtime_installed":False,
      "build_check":"PASS"
    }
    p=root/MANIFEST; p.parent.mkdir(parents=True,exist_ok=True)
    p.write_text(json.dumps(obj,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
    return p,obj

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("action",choices=["doctor","inspect","build"])
    ap.add_argument("--root",default=".")
    a=ap.parse_args(); root=pathlib.Path(a.root).expanduser().resolve()
    print("PT_PIKEMAN_MANUAL_ARTIST_GATE_G5_V1")
    print(f"action={a.action}"); print(f"root={root}")
    doctor(root)
    if a.action=="doctor": return
    inspect_data,_=inspect_artist(root)
    if a.action=="inspect":
        print("g5_artist_inspection_gate_check=PASS"); return
    smd,counts,_=pipeline(root)
    bind_vertices,_=reimport(root,smd,counts)
    material_audit(root,smd)
    man,obj=manifest(root,smd,counts,inspect_data,bind_vertices)
    print(f"manual_topology_changed={obj['manual_topology_changed']}")
    print(f"manual_positions_changed={obj['manual_positions_changed']}")
    print(f"manual_uv_changed={obj['manual_uv_changed']}")
    print(f"manual_weights_changed={obj['manual_weights_changed']}")
    print(f"output_counts={counts[0]}/{counts[1]}/{counts[2]}")
    print(f"bind_vertices={bind_vertices}")
    print(f"artifact={smd}")
    print(f"artifact_size={obj['artifact_size']}")
    print(f"artifact_sha256={obj['artifact_sha256']}")
    print(f"manifest={man}")
    print("runtime_installed=False")
    print("g5_structural_gate_check=PASS")

if __name__=="__main__":
    try: main()
    except Exception as e:
        print(f"g5_gate_check=FAIL: {type(e).__name__}: {e}")
        raise
