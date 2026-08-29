#!/usr/bin/env python3
from __future__ import annotations
import argparse, importlib.util, json
from collections import Counter
from pathlib import Path

TOOL = "PT_SMB_ANIMATION_SEGMENT_RUNTIME_AUDIT_V3"

def load_evaluator(root: Path):
    p = root / "history_py/pt_smb_motion_evaluator_v1.py"
    if not p.is_file():
        raise FileNotFoundError(p)
    spec = importlib.util.spec_from_file_location("pt_smb_eval_seg_v3", p)
    mod = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(mod)
    return mod

def probe_frames(seg):
    s, e = seg["start"], seg["end"]
    if e <= s:
        return [s]
    vals = {s, e - 1, s + (e - s) // 2}
    if s + 1 < e:
        vals.add(s + 1)
    return sorted(vals)

def audit_track(ev, bone, kind):
    arr = bone[kind]
    segs = bone[kind + "_frames"]
    errors = []
    probes = 0
    inactive_garbage = 0

    for seg in segs:
        # Exact engine rule: PosNum is irrelevant unless PosCnt > 0.
        if seg["pos_cnt"] <= 0:
            if seg["pos_num"] < 0 or seg["pos_num"] > len(arr):
                inactive_garbage += 1
            continue

        if seg["end"] <= seg["start"]:
            errors.append(f"slot={seg['slot']}: active EndFrame<=StartFrame")
            continue

        if not 0 <= seg["pos_num"] < len(arr):
            errors.append(
                f"slot={seg['slot']}: active PosNum={seg['pos_num']} "
                f"outside key_count={len(arr)}"
            )
            continue

        for frame in probe_frames(seg):
            base, chosen = ev.get_tm_frame(segs, frame)
            if base < 0:
                errors.append(
                    f"slot={seg['slot']} frame={frame}: GetTmFrame returned inactive"
                )
                continue
            if chosen != seg["slot"]:
                errors.append(
                    f"slot={seg['slot']} frame={frame}: overlap causes source "
                    f"selection of slot={chosen}"
                )
                continue

            stats = Counter()
            try:
                if kind == "rot":
                    ev.get_rot_frame(bone, base, frame, None, stats)
                elif kind == "pos":
                    ev.get_pos_frame(bone, base, frame, (0.0, 0.0, 0.0), stats)
                else:
                    ev.get_scale_frame(bone, base, frame, (256, 256, 256), stats)
                probes += 1
            except Exception as exc:
                errors.append(
                    f"slot={seg['slot']} frame={frame} base={base}: "
                    f"{type(exc).__name__}: {exc}"
                )

    return {
        "track": kind,
        "key_count": len(arr),
        "active_segments": sum(x["pos_cnt"] > 0 for x in segs),
        "inactive_segments_with_ignored_garbage_posnum": inactive_garbage,
        "source_lookup_probes": probes,
        "errors": errors,
        "check": "PASS" if not errors else "FAIL",
    }

def audit_one(ev, p: Path):
    sk = ev.parse_smb(p)
    bone_rows = []
    errors = []
    for bone in sk["bones"]:
        tracks = [audit_track(ev, bone, k) for k in ("rot", "pos", "scale")]
        for t in tracks:
            errors.extend(
                f"{bone['name']} {t['track']}: {msg}" for msg in t["errors"]
            )
        bone_rows.append({"bone": bone["name"], "tracks": tracks})

    return {
        "smb": str(p),
        "bones": len(sk["bones"]),
        "tm_frame_count": sk["tm_frame_count"],
        "bone_rows": bone_rows,
        "error_count": len(errors),
        "errors": errors,
        "check": "PASS" if not errors else "FAIL",
    }

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--smb", action="append", required=True)
    ap.add_argument("--report", required=True)
    a = ap.parse_args()

    root = Path(a.root).expanduser().resolve()
    ev = load_evaluator(root)
    models = [audit_one(ev, Path(x).expanduser().resolve()) for x in a.smb]

    result = {
        "tool": TOOL,
        "validation_policy": (
            "source-faithful GetTmFrame/GetRotFrame/GetPosFrame/GetScaleFrame "
            "probe; inactive PosNum ignored; no invented slice-envelope rule"
        ),
        "models": models,
        "client_modified": False,
        "runtime_segment_audit_check": (
            "PASS" if all(x["check"] == "PASS" for x in models) else "FAIL"
        ),
    }

    rp = Path(a.report).expanduser().resolve()
    rp.parent.mkdir(parents=True, exist_ok=True)
    rp.write_text(
        json.dumps(result, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )

    print(TOOL)
    for m in models:
        ignored = sum(
            t["inactive_segments_with_ignored_garbage_posnum"]
            for b in m["bone_rows"] for t in b["tracks"]
        )
        probes = sum(
            t["source_lookup_probes"]
            for b in m["bone_rows"] for t in b["tracks"]
        )
        print(
            f"SMB {Path(m['smb']).name}: bones={m['bones']} "
            f"tmframes={m['tm_frame_count']} probes={probes} "
            f"inactive_garbage_ignored={ignored} "
            f"errors={m['error_count']} check={m['check']}"
        )
    print(f"report={rp}")
    print("client_modified=False")
    print(f"runtime_segment_audit_check={result['runtime_segment_audit_check']}")
    if result["runtime_segment_audit_check"] != "PASS":
        raise SystemExit(2)

if __name__ == "__main__":
    main()
