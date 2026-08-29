from __future__ import annotations
import json, os, pathlib, shutil, subprocess
from .utils import norm_path


def prepare_texture_overlay(*, asset, resolver, root: pathlib.Path, work: pathlib.Path) -> tuple[pathlib.Path, dict]:
    """Create a read-only client texture overlay for Blender import.

    - TGA is decoded through pt_protected_tga_v1.py (standard TGA remains byte-equivalent).
    - Other texture types are symlinked to the real client file.
    - The real client is never modified.
    """
    overlay = work / "texture_overlay_client"
    if overlay.exists():
        shutil.rmtree(overlay)
    overlay.mkdir(parents=True, exist_ok=True)

    rows = []
    ok = True
    for logical in asset.textures:
        logical_n = norm_path(logical)
        dst = overlay / pathlib.PurePosixPath(logical_n)
        dst.parent.mkdir(parents=True, exist_ok=True)
        try:
            src = resolver.resolve(logical_n)
        except Exception as exc:
            ok = False
            rows.append({"logical": logical_n, "status": "MISSING", "error": f"{type(exc).__name__}: {exc}"})
            continue

        ext = src.suffix.lower()
        if ext == ".tga":
            log = work / "texture_overlay_logs" / (dst.name + ".decode.log")
            log.parent.mkdir(parents=True, exist_ok=True)
            cmd = [
                "python3", str(root / "pt_protected_tga_v1.py"), "decode", str(src), "--out", str(dst)
            ]
            proc = subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            log.write_text(proc.stdout or "", encoding="utf-8")
            if proc.returncode != 0 or not dst.is_file():
                ok = False
                rows.append({
                    "logical": logical_n, "physical": str(src), "status": "TGA_DECODE_FAIL",
                    "log": str(log), "returncode": proc.returncode,
                })
                # Keep the original available for diagnostic import, but do not call it prepared.
                try:
                    if dst.exists() or dst.is_symlink():
                        dst.unlink()
                    os.symlink(src, dst)
                except Exception:
                    pass
            else:
                rows.append({
                    "logical": logical_n, "physical": str(src), "prepared": str(dst),
                    "status": "TGA_READY", "log": str(log),
                })
        else:
            try:
                os.symlink(src, dst)
            except FileExistsError:
                pass
            rows.append({
                "logical": logical_n, "physical": str(src), "prepared": str(dst),
                "status": "LINK_READY",
            })

    report = {
        "asset_id": asset.asset_id,
        "asset": asset.primary_path,
        "texture_count": len(asset.textures),
        "ready": ok,
        "overlay": str(overlay),
        "entries": rows,
        "client_modified": False,
    }
    rp = work / "texture_overlay_report.json"
    rp.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return overlay, report
