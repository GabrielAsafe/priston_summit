from __future__ import annotations
import json, pathlib, subprocess
from dataclasses import dataclass
from .certification import CertificationRegistry
from .config import Settings
from .registry import Asset
from .resolver import ClientResolver
from .utils import run_logged, sha256_file, utc_now


@dataclass
class ValidationContext:
    settings: Settings
    resolver: ClientResolver
    certifications: CertificationRegistry


class ValidationRunner:
    def __init__(self, ctx: ValidationContext):
        self.ctx = ctx
        self.root = ctx.settings.project_root

    def _asset_workdir(self, asset: Asset, test: str) -> pathlib.Path:
        return self.ctx.settings.work_dir / asset.asset_id / test.lower()

    def structural(self, asset: Asset) -> dict:
        src = self.ctx.resolver.resolve(asset.primary_path)
        work = self._asset_workdir(asset, "STRUCTURAL")
        work.mkdir(parents=True, exist_ok=True)
        source_sha = sha256_file(src)
        result = {"asset_id": asset.asset_id, "asset": asset.primary_path, "source_sha256": source_sha}

        if asset.family == "SMD_MODEL_062_RIGID_PHYSIQUE":
            if asset.binding_mode != "SELF_CONTAINED" or len(asset.motions) != 1 or not asset.configs:
                raise RuntimeError(f"rigid asset is not self-contained: binding={asset.binding_mode}")
            smb = self.ctx.resolver.resolve(asset.motions[0])
            structural_report = work / "character_structural.json"
            run_logged([
                "python3", str(self.root / "pt_character_structural_gate_v2.py"),
                "--smd", str(src), "--smb", str(smb), "--report", str(structural_report)
            ], work / "character_structural.log")
            inx_rows = []
            for i, cfg in enumerate(asset.configs):
                if not cfg.get("motion_smb"):
                    continue
                inx = self.ctx.resolver.resolve(cfg["inx"])
                motion = self.ctx.resolver.resolve(cfg["motion_smb"])
                rp = work / f"inx_{i:03d}.json"
                run_logged([
                    "python3", str(self.root / "pt_inx_motion_core_v2.py"),
                    "--inx", str(inx), "--smb", str(motion), "--report", str(rp)
                ], work / f"inx_{i:03d}.log")
                inx_rows.append(json.loads(rp.read_text(encoding="utf-8")))
            result.update({"mode": "RIGID", "structural_report": str(structural_report), "inx_checks": len(inx_rows), "check": "PASS"})

        elif asset.family == "SMD_MODEL_062_STATIC":
            rp = work / "static_capability.json"
            run_logged([
                "python3", str(self.root / "pt_smd_static_capability_probe_v1.py"),
                "--smd", str(src), "--report", str(rp)
            ], work / "static_capability.log")
            probe = json.loads(rp.read_text(encoding="utf-8"))
            result.update({
                "mode": "STATIC",
                "static_capability": probe,
                "authoring_ready_v1": bool(probe.get("ready_objects")) and not probe.get("blocked_objects"),
                "check": "PASS",
            })
        else:
            raise RuntimeError(f"no structural validator wired yet for family {asset.family}")

        (work / "summary.json").write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        self.ctx.certifications.add(
            asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
            test="STRUCTURAL", layer="AUTOMATIC", result="PASS", source_sha256=source_sha,
            metadata={"workdir": str(work), "mode": result.get("mode")},
        )
        return result

    def _prepare_rigid_blend(self, asset: Asset, lod: str, work: pathlib.Path) -> tuple[pathlib.Path, pathlib.Path]:
        src = self.ctx.resolver.resolve(asset.primary_path)
        if asset.binding_mode != "SELF_CONTAINED" or len(asset.motions) != 1 or not asset.configs:
            raise RuntimeError(f"asset cannot use rigid Blender importer: binding={asset.binding_mode}")
        cfg = next((x for x in asset.configs if x.get("motion_smb")), asset.configs[0])
        inx = self.ctx.resolver.resolve(cfg["inx"])
        smb = self.ctx.resolver.resolve(asset.motions[0])
        pristine = work / "pristine_import.blend"
        cmd = self.ctx.settings.blender_command() + [
            "--background", "--python-exit-code", "1",
            "--python", str(self.root / "history_py/pt_smd_blender_import_v3_static_armature.py"), "--",
            "--smd", str(src), "--skeleton", str(smb), "--model-info", str(inx),
            "--lod", lod, "--client-root", str(self.ctx.settings.client_root),
            "--out", str(pristine), "--no-pack",
        ]
        run_logged(cmd, work / "01_import.log")
        return src, pristine

    def _prepare_static_blend(self, asset: Asset, work: pathlib.Path) -> tuple[pathlib.Path, pathlib.Path]:
        src = self.ctx.resolver.resolve(asset.primary_path)
        pristine = work / "pristine_import.blend"
        cmd = self.ctx.settings.blender_command() + [
            "--background", "--python-exit-code", "1",
            "--python", str(self.root / "pt_smd_blender_import_v4_generic.py"), "--",
            "--smd", str(src), "--client-root", str(self.ctx.settings.client_root),
            "--out", str(pristine), "--no-pack",
        ]
        run_logged(cmd, work / "01_import.log")
        return src, pristine

    def visual_geometry(self, asset: Asset, lod: str = "default", non_interactive_result: str | None = None) -> dict:
        mode = asset.visual_mode
        if not mode:
            raise RuntimeError(f"visual geometry gate not wired for family/binding: {asset.family}/{asset.binding_mode}")
        work = self._asset_workdir(asset, "VISUAL_GEOMETRY")
        work.mkdir(parents=True, exist_ok=True)

        # Structural validation is always a prerequisite and records its own certification.
        self.structural(asset)
        if mode == "RIGID":
            src, pristine = self._prepare_rigid_blend(asset, lod, work)
        else:
            src, pristine = self._prepare_static_blend(asset, work)

        witness = work / "visual_geometry_witness.blend"
        witness_report = work / "witness.json"
        cmd = self.ctx.settings.blender_command() + [
            "--background", str(pristine), "--python-exit-code", "1",
            "--python", str(self.root / "pt_asset_tool/blender/witness_geometry.py"), "--",
            "--out", str(witness), "--report", str(witness_report),
        ]
        run_logged(cmd, work / "02_witness.log")

        if non_interactive_result is None:
            gui_cmd = self.ctx.settings.blender_command() + [str(witness)]
            gui_log = work / "03_blender_gui.log"
            proc = subprocess.run(gui_cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            gui_log.write_text(proc.stdout or "", encoding="utf-8")
            if proc.returncode:
                raise RuntimeError(f"Blender GUI exited rc={proc.returncode}; log={gui_log}")
            while True:
                ans = input("Visualmente correto? [y]es / [n]o / [s]kip: ").strip().lower()
                if ans in {"y", "yes"}:
                    result = "PASS"; break
                if ans in {"n", "no"}:
                    result = "FAIL"; break
                if ans in {"s", "skip", ""}:
                    result = "SKIP"; break
        else:
            result = non_interactive_result.upper()
            if result not in {"PASS", "FAIL", "SKIP"}:
                raise ValueError("non_interactive_result must be PASS/FAIL/SKIP")

        source_sha = sha256_file(src)
        artifact_sha = sha256_file(witness)
        cert = self.ctx.certifications.add(
            asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
            test="VISUAL_GEOMETRY", layer="HUMAN_VISUAL", result=result,
            source_sha256=source_sha, artifact_sha256=artifact_sha,
            metadata={"witness": str(witness), "pristine_blend": str(pristine), "lod": lod, "mode": mode},
        )
        summary = {
            "asset_id": asset.asset_id, "asset": asset.primary_path, "family": asset.family,
            "mode": mode, "lod": lod, "witness": str(witness), "source_sha256": source_sha,
            "artifact_sha256": artifact_sha, "human_visual": result, "certification_id": cert["certification_id"],
            "client_modified": False,
        }
        (work / "summary.json").write_text(json.dumps(summary, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        return summary
