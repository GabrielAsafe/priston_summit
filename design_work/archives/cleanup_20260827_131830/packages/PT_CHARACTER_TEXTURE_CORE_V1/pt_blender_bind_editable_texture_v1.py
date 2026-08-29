#!/usr/bin/env python3
"""
Bind an editable standard texture BMP into a copy of an imported character .blend.

It finds image texture nodes/images whose basename corresponds to the original
game texture, including importer cache names like:
  MfbC01.<hash>.decoded.bmp

The input .blend is never overwritten.
"""

from __future__ import annotations
import argparse
import pathlib
import sys
import bpy


def args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    p = argparse.ArgumentParser()
    p.add_argument("--source-basename", required=True)
    p.add_argument("--texture", required=True)
    p.add_argument("--out", required=True)
    p.add_argument("--overwrite", action="store_true")
    return p.parse_args(argv)


def match_image(img, source_basename):
    wanted = pathlib.Path(source_basename).name.lower()
    stem = pathlib.Path(wanted).stem.lower()
    vals = [
        img.name or "",
        pathlib.Path(bpy.path.abspath(img.filepath or "")).name,
        pathlib.Path(bpy.path.abspath(img.filepath_raw or "")).name,
    ]
    for raw in vals:
        name = str(raw).lower()
        if name == wanted:
            return True
        if name.startswith(stem + ".") and ".decoded." in name:
            return True
        if pathlib.Path(name).stem == stem:
            return True
    return False


def main():
    a = args()
    texture = pathlib.Path(a.texture).expanduser().resolve()
    out = pathlib.Path(a.out).expanduser().resolve()
    if not texture.is_file():
        raise FileNotFoundError(texture)
    if out.exists() and not a.overwrite:
        raise FileExistsError(out)

    new_image = bpy.data.images.load(str(texture), check_existing=False)
    matched_images = set()
    replaced_nodes = 0

    for mat in bpy.data.materials:
        if not mat.use_nodes or not mat.node_tree:
            continue
        for node in mat.node_tree.nodes:
            if node.bl_idname != "ShaderNodeTexImage" or node.image is None:
                continue
            if match_image(node.image, a.source_basename):
                matched_images.add(node.image.name)
                node.image = new_image
                replaced_nodes += 1

    if replaced_nodes == 0:
        raise RuntimeError(
            f"no image texture node matched source basename {a.source_basename!r}"
        )

    out.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(out))

    print("PT_BLENDER_BIND_EDITABLE_TEXTURE_V1")
    print(f"source_basename={a.source_basename}")
    print(f"editable_texture={texture}")
    print(f"matched_images={sorted(matched_images)}")
    print(f"replaced_nodes={replaced_nodes}")
    print(f"output_blend={out}")
    print("artist_master_overwritten=False")
    print("texture_bind_blend_check=PASS")


if __name__ == "__main__":
    main()
