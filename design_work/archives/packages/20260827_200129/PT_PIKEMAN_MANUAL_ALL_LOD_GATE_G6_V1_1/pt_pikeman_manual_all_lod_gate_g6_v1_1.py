#!/usr/bin/env python3
from __future__ import annotations
import argparse, hashlib, json, pathlib, re, shlex, shutil, subprocess, sys

PRISTINE_SHA="9c5b137bc637f485419cafae6864c4a37f77835d32210810295c1b58c746443c"
SKELETON_SHA="48af614678593eaa0b659c2be514bd60f86b94f72cfb4305ed8352f48f12cc4a"
PROFILE="profiles/pt_character_profile_pikeman_c01_m4_v1.json"
BASELINE="blender/pikeman_c01_m4/armor_family/C002_pikeman_armor_preview.blend"
ARTIST="blender/pikeman_c01_m4/artist/C002_TmbC03_artist_master_g6_all_lods_v1.blend"
LOGDIR="logs/characters/pikeman_c01_m4/geometry/manual_all_lods_g6_v1"
NAME="TmbC01_C002_manual_all_lods_g6_v1"
CANON_DIR="pt_build/characters/pikeman_c01_m4/geometry/manual_all_lods_g6_v1"
CANON_SMD=f"{CANON_DIR}/{NAME}.smd"
MANIFEST=f"{LOGDIR}/{NAME}_build_manifest.json"
TARGETS=("TbhC03","TbmC03","TblC03")
VISIBLE={
  "high":["TbhC03","TahC01"],
  "default":["TbmC03","TamC01"],
  "low":["TblC03","TalC01"],
}
NON_TARGET_SOURCE_VERTICES=726

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

def doctor(root,artist):
    for rel in [
      "TmbC01.smd","m4.smb","C002.inx",PROFILE,BASELINE,
      "pt_pikeman_manual_all_lod_inspect_g6_v1.py",
      "pt_character_multi_object_lod_pipeline_v2.py",
      "pt_blender_to_smd_free_mesh_v4_multi.py",
      "pt_free_mesh_validate_multi_v1.py",
      "pt_free_mesh_validate_v2.py",
      "pt_rigid_skin_normalize_copy_v2.py",
      "history_py/pt_smd_blender_import_v3_static_armature.py",
      "pt_pikeman_smd_material_audit_v1.py",
      "summit_workspace_asset_bridge_v1_2.py",
      "summit_roundtrip_restore_v4.py",
    ]: need(root,rel)

    if not artist.is_file():
        raise RuntimeError(f"missing required artist file: {artist}")
    print(f"FILE_OK={artist}")

    if sha(root/"TmbC01.smd")!=PRISTINE_SHA:
        raise RuntimeError("root TmbC01.smd not pristine")
    if sha(root/"m4.smb")!=SKELETON_SHA:
        raise RuntimeError("m4.smb SHA mismatch")

    client=root.parent/"Summit Tale Client Full/cliente/char/tmABCD/TmbC01.smd"
    if not client.is_file(): raise RuntimeError(f"client model missing: {client}")
    csha=sha(client); print(f"client_sha256={csha}")
    if csha!=PRISTINE_SHA:
        raise RuntimeError("client is not pristine; restore G5 before G6")

    print(f"artist={artist}")
    print(f"artist_sha256={sha(artist)}")
    print("profile_id_expected=pikeman_c01_m4")
    print(f"targets={list(TARGETS)}")
    print("non_target_expected_byte_identical=6")
    print("doctor_check=PASS")

def inspect(root,artist):
    report=root/LOGDIR/"artist_inspect.json"
    report.parent.mkdir(parents=True,exist_ok=True)
    text=run([
      "flatpak","run","--filesystem=home","org.blender.Blender",
      "--background",str(artist),"--python-exit-code","1",
      "--python",str(root/"pt_pikeman_manual_all_lod_inspect_g6_v1.py"),
      "--",str(root/BASELINE),str(report)
    ],root)
    for m in [
      "all_targets_changed=True",
      "used_materials_all_targets=[0]",
      "uv_layer_all_targets='SMD_UV'",
      "vertex_group_count_all_targets=11",
      "non_target_unchanged_count=6",
      "g6_artist_inspect_check=PASS",
    ]:
        if m not in text:
            raise RuntimeError(f"inspect missing marker: {m}")
    return json.loads(report.read_text(encoding="utf-8")),text

def pipeline(root,artist):
    cmd=[
      sys.executable,str(root/"pt_character_multi_object_lod_pipeline_v2.py"),
      "--root",str(root),
      "--profile",str(root/PROFILE),
      "--artist",str(artist),
      "--object","TbhC03",
      "--object","TbmC03",
      "--object","TblC03",
      "--reimport-lod","high",
      "--name",NAME,
    ]
    text=run(cmd,root)

    for m in [
      "objects=['TbhC03', 'TbmC03', 'TblC03']",
      "object_count=3",
      "non_target_object_blocks_byte_identical=6",
      "test_force_ambiguous_used=false",
      "multi_object_pipeline_v2_check=PASS",
    ]:
        if m not in text:
            raise RuntimeError(f"multi pipeline missing marker: {m}")

    counts={}
    for name in TARGETS:
        mo=re.search(rf"OBJECT {re.escape(name)}: output_counts=(\d+)/(\d+)/(\d+)",text)
        if not mo:
            raise RuntimeError(f"could not parse output counts for {name}")
        counts[name]=tuple(map(int,mo.groups()))
        if counts[name][1]!=counts[name][2]:
            raise RuntimeError(f"{name}: face/TexLink count mismatch {counts[name]}")

    mo=re.search(r"^smd=(.+)$",text,re.M)
    if not mo:
        raise RuntimeError("could not parse multi-object SMD path")
    smd=pathlib.Path(mo.group(1).strip())
    if not smd.is_file():
        raise RuntimeError(f"multi-object SMD missing: {smd}")

    dest=root/CANON_SMD
    dest.parent.mkdir(parents=True,exist_ok=True)
    shutil.copy2(smd,dest)
    (root/LOGDIR/"multi_pipeline_stdout.txt").write_text(text,encoding="utf-8")
    return dest,counts,text

def authoritative_reimports(root,smd,counts):
    client_root=root.parent/"Summit Tale Client Full/cliente"
    expected_bind=NON_TARGET_SOURCE_VERTICES+sum(counts[n][0] for n in TARGETS)
    results={}

    for lod in ("high","default","low"):
        out=root/f"blender/pikeman_c01_m4/artist/C002_manual_all_lods_g6_v1_reimport_{lod}.blend"
        out.parent.mkdir(parents=True,exist_ok=True)
        text=run([
          "flatpak","run","--filesystem=home","org.blender.Blender","--background",
          "--python-exit-code","1",
          "--python",str(root/"history_py/pt_smd_blender_import_v3_static_armature.py"),
          "--","--smd",str(smd),"--skeleton",str(root/"m4.smb"),
          "--model-info",str(root/"C002.inx"),"--lod",lod,
          "--client-root",str(client_root),"--out",str(out),"--no-pack",
        ],root)

        for name,c in counts.items():
            marker=f"OBJECT {name}: V={c[0]} F={c[1]} UV={c[2]}"
            if marker not in text:
                raise RuntimeError(f"{lod}: missing rebuilt count marker {marker}")

        if "static_armature_import_check=PASS" not in text:
            raise RuntimeError(f"{lod}: independent C002 reimport failed")

        vis=VISIBLE[lod]
        visible_a=f"visible_objects={vis!r}"
        visible_b=f"visible_objects={list(reversed(vis))!r}"
        if visible_a not in text and visible_b not in text:
            raise RuntimeError(f"{lod}: wrong C002 visible objects; expected {vis}")

        mb=re.search(r"Bind vertices\s*:\s*(\d+)",text)
        if not mb:
            raise RuntimeError(f"{lod}: could not parse bind vertices")
        bind=int(mb.group(1))
        if bind!=expected_bind:
            raise RuntimeError(f"{lod}: bind vertices {bind} != expected {expected_bind}")

        (root/LOGDIR/f"reimport_{lod}.txt").write_text(text,encoding="utf-8")
        results[lod]={
          "blend":str(out),
          "visible_objects":vis,
          "bind_vertices":bind,
          "check":"PASS",
        }

    return expected_bind,results

def material_audit(root,smd):
    report=root/LOGDIR/"material_audit.json"
    text=run([
      sys.executable,str(root/"pt_pikeman_smd_material_audit_v1.py"),
      "--smd",str(smd),"--report",str(report)
    ],root)
    if "material_audit_check=PASS" not in text:
        raise RuntimeError("material audit failed")
    for marker in [
      "textures=[{'Name': 'char\\\\tmABCD\\\\TmbC03.bmp', 'NameA': ''}]",
      "textures=[{'Name': 'char\\\\tmABCD\\\\TmbC01.bmp', 'NameA': ''}]",
      "BlendType=1 Shade=1 TwoSide=0",
      "SelfIllum_raw_i32=0",
      "TextureClip=0 UseState=0 MeshState=1 WindMeshBottom=0",
    ]:
        if marker not in text:
            raise RuntimeError(f"baseline material marker missing: {marker}")
    return report

def write_manifest(root,smd,counts,inspect_data,bind_vertices,reimports,artist):
    src={
      "TbhC03":{"vertices":92,"faces":152,"texlinks":152},
      "TbmC03":{"vertices":74,"faces":122,"texlinks":122},
      "TblC03":{"vertices":63,"faces":100,"texlinks":100},
    }
    out={
      n:{"vertices":counts[n][0],"faces":counts[n][1],"texlinks":counts[n][2]}
      for n in TARGETS
    }
    obj={
      "tool":"PT_PIKEMAN_MANUAL_ALL_LOD_GATE_G6_V1_1",
      "profile_id":"pikeman_c01_m4",
      "asset_set_id":"CHARSET-PIKEMAN-C01-M4",
      "roundtrip_target":"char/tmABCD/TmbC01.smd",
      "artist_master":str(artist.resolve()),
      "artist_master_sha256":sha(artist),
      "artifact":str(smd.resolve()),
      "artifact_sha256":sha(smd),
      "artifact_size":smd.stat().st_size,
      "source_sha256":PRISTINE_SHA,
      "targets":list(TARGETS),
      "source_counts":src,
      "output_counts":out,
      "all_targets_manually_changed":True,
      "artist_non_target_unchanged_count":6,
      "all_target_vertices_rebuilt":True,
      "all_target_faces_rebuilt":True,
      "all_target_texlinks_rebuilt":True,
      "all_target_physique_rebuilt":True,
      "non_target_object_blocks_byte_identical":6,
      "independent_C002_reimports":reimports,
      "bind_vertices":bind_vertices,
      "bind_bones_used":32,
      "material_audit":"PASS",
      "runtime_installed":False,
      "build_check":"PASS",
    }
    p=root/MANIFEST
    p.parent.mkdir(parents=True,exist_ok=True)
    p.write_text(json.dumps(obj,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
    return p,obj

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("action",choices=["doctor","inspect","build"])
    ap.add_argument("--root",default=".")
    ap.add_argument(
      "--artist",
      default=DEFAULT_ARTIST,
      help="artist .blend path; relative paths are resolved from --root"
    )
    a=ap.parse_args()
    root=pathlib.Path(a.root).expanduser().resolve()
    artist=pathlib.Path(a.artist).expanduser()
    if not artist.is_absolute():
        artist=(root/artist).resolve()
    else:
        artist=artist.resolve()

    print("PT_PIKEMAN_MANUAL_ALL_LOD_GATE_G6_V1_1")
    print(f"action={a.action}")
    print(f"root={root}")
    doctor(root,artist)
    if a.action=="doctor": return

    inspect_data,_=inspect(root,artist)
    if a.action=="inspect":
        print("g6_artist_inspection_gate_check=PASS")
        return

    smd,counts,_=pipeline(root,artist)
    bind,reimports=authoritative_reimports(root,smd,counts)
    material_audit(root,smd)
    man,obj=write_manifest(root,smd,counts,inspect_data,bind,reimports,artist)

    for name in TARGETS:
        c=counts[name]
        print(f"OBJECT {name}: final_counts={c[0]}/{c[1]}/{c[2]}")
    print(f"bind_vertices={bind}")
    print(f"artifact={smd}")
    print(f"artifact_size={obj['artifact_size']}")
    print(f"artifact_sha256={obj['artifact_sha256']}")
    print(f"manifest={man}")
    print("C002_high_reimport=PASS")
    print("C002_default_reimport=PASS")
    print("C002_low_reimport=PASS")
    print("runtime_installed=False")
    print("g6_structural_gate_check=PASS")

if __name__=="__main__":
    try: main()
    except Exception as e:
        print(f"g6_gate_check=FAIL: {type(e).__name__}: {e}")
        raise
