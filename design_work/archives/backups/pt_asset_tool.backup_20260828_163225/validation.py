from __future__ import annotations
import json, pathlib, subprocess
from dataclasses import dataclass
from .certification import CertificationRegistry
from .config import Settings
from .registry import Asset
from .resolver import ClientResolver
from .texture_overlay import prepare_texture_overlay
from .motion_core import build_bake_payload, write_payload
from .utils import run_logged, sha256_file


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
            if not asset.rigid_binding_ready:
                raise RuntimeError(f"rigid asset has no unambiguous skeleton binding: binding={asset.binding_mode}")
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
                motion_inx = cfg.get("motion_inx") or (cfg.get("inx") if asset.binding_source not in {"STRUCTURAL_GATE_UNIQUE", "SKELETON_SIGNATURE_UNIQUE"} else "")
                if not motion_inx:
                    continue
                inx = self.ctx.resolver.resolve(motion_inx)
                motion = self.ctx.resolver.resolve(cfg["motion_smb"])
                rp = work / f"inx_{i:03d}.json"
                run_logged([
                    "python3", str(self.root / "pt_inx_motion_core_v2.py"),
                    "--inx", str(inx), "--smb", str(motion), "--report", str(rp)
                ], work / f"inx_{i:03d}.log")
                inx_rows.append(json.loads(rp.read_text(encoding="utf-8")))
            result.update({"mode": "RIGID", "binding_mode": asset.binding_mode, "binding_source": asset.binding_source,
                "binding_root_models": asset.binding_root_models,
                "structural_report": str(structural_report), "inx_checks": len(inx_rows), "check": "PASS"})

        elif asset.family == "SMD_MODEL_062_STATIC":
            rp = work / "static_capability.json"
            run_logged([
                "python3", str(self.root / "pt_smd_static_capability_probe_v1.py"),
                "--smd", str(src), "--report", str(rp)
            ], work / "static_capability.log")
            probe = json.loads(rp.read_text(encoding="utf-8"))
            result.update({
                "mode": "STATIC", "static_capability": probe,
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

    def _select_rigid_lod(self, asset: Asset, requested: str) -> tuple[str, dict]:
        cfg = next((x for x in asset.configs if x.get("motion_smb")), asset.configs[0])
        groups = cfg.get("lod_groups") or {}
        if requested != "auto":
            if not groups.get(requested):
                available = [x for x in ("high", "default", "low") if groups.get(x)]
                raise RuntimeError(f"requested LOD '{requested}' has no active INX nodes; available={available}")
            return requested, cfg
        for lod in ("default", "high", "low"):
            if groups.get(lod):
                return lod, cfg
        raise RuntimeError("INX has no active high/default/low nodes for this model")

    def _texture_overlay(self, asset: Asset, work: pathlib.Path) -> tuple[pathlib.Path, dict]:
        overlay, report = prepare_texture_overlay(
            asset=asset, resolver=self.ctx.resolver, root=self.root, work=work
        )
        if asset.textures:
            self.ctx.certifications.add(
                asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
                test="TEXTURE_PREPARE", layer="AUTOMATIC", result="PASS" if report["ready"] else "FAIL",
                source_sha256=sha256_file(self.ctx.resolver.resolve(asset.primary_path)),
                metadata={"report": str(work / "texture_overlay_report.json"), "overlay": str(overlay)},
            )
        return overlay, report

    def _prepare_rigid_blend(self, asset: Asset, lod: str, work: pathlib.Path, client_root: pathlib.Path) -> tuple[pathlib.Path, pathlib.Path]:
        src = self.ctx.resolver.resolve(asset.primary_path)
        if not asset.rigid_binding_ready:
            raise RuntimeError(f"asset cannot use rigid Blender importer without unambiguous skeleton binding: binding={asset.binding_mode}")
        actual_lod, cfg = self._select_rigid_lod(asset, lod)
        inx = self.ctx.resolver.resolve(cfg["inx"])
        smb = self.ctx.resolver.resolve(asset.motions[0])
        pristine = work / "pristine_import.blend"
        cmd = self.ctx.settings.blender_command() + [
            "--background", "--python-exit-code", "1",
            "--python", str(self.root / "history_py/pt_smd_blender_import_v3_static_armature.py"), "--",
            "--smd", str(src), "--skeleton", str(smb), "--model-info", str(inx),
            "--lod", actual_lod, "--client-root", str(client_root),
            "--out", str(pristine), "--no-pack",
        ]
        run_logged(cmd, work / "01_import.log")
        return src, pristine, actual_lod

    def _prepare_static_blend(self, asset: Asset, work: pathlib.Path, client_root: pathlib.Path) -> tuple[pathlib.Path, pathlib.Path, str]:
        src = self.ctx.resolver.resolve(asset.primary_path)
        pristine = work / "pristine_import.blend"
        cmd = self.ctx.settings.blender_command() + [
            "--background", "--python-exit-code", "1",
            "--python", str(self.root / "pt_smd_blender_import_v4_generic.py"), "--",
            "--smd", str(src), "--client-root", str(client_root),
            "--out", str(pristine), "--no-pack",
        ]
        run_logged(cmd, work / "01_import.log")
        return src, pristine, "default"

    @staticmethod
    def _ask(label: str) -> str:
        while True:
            ans = input(f"{label} [y]es / [n]o / [s]kip: ").strip().lower()
            if ans in {"y", "yes"}: return "PASS"
            if ans in {"n", "no"}: return "FAIL"
            if ans in {"s", "skip", ""}: return "SKIP"

    @staticmethod
    def _overall(geometry: str, texture: str, has_textures: bool) -> str:
        relevant = [geometry] + ([texture] if has_textures else [])
        if "FAIL" in relevant:
            return "FAIL"
        if relevant and all(x == "PASS" for x in relevant):
            return "PASS"
        return "SKIP"

    def visual_geometry(self, asset: Asset, lod: str = "auto", non_interactive_result: str | None = None) -> dict:
        mode = asset.visual_mode
        if not mode:
            raise RuntimeError(f"visual geometry gate not wired for family/binding: {asset.family}/{asset.binding_mode}")
        work = self._asset_workdir(asset, "VISUAL_GEOMETRY")
        work.mkdir(parents=True, exist_ok=True)

        self.structural(asset)
        overlay_root, texture_report = self._texture_overlay(asset, work)
        if mode == "RIGID":
            src, pristine, actual_lod = self._prepare_rigid_blend(asset, lod, work, overlay_root)
        else:
            src, pristine, actual_lod = self._prepare_static_blend(asset, work, overlay_root)

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
            geometry_result = self._ask("Geometria/witness visualmente correto?")
            texture_result = self._ask("Texturas e materiais carregaram corretamente?") if asset.textures else "SKIP"
        else:
            geometry_result = non_interactive_result.upper()
            if geometry_result not in {"PASS", "FAIL", "SKIP"}:
                raise ValueError("non_interactive_result must be PASS/FAIL/SKIP")
            texture_result = geometry_result if asset.textures else "SKIP"

        source_sha = sha256_file(src)
        artifact_sha = sha256_file(witness)
        common_meta = {
            "witness": str(witness), "pristine_blend": str(pristine), "lod": actual_lod, "mode": mode,
            "texture_overlay_report": str(work / "texture_overlay_report.json"),
            "binding_mode": asset.binding_mode, "binding_source": asset.binding_source,
            "binding_root_models": asset.binding_root_models,
        }
        geom_cert = self.ctx.certifications.add(
            asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
            test="VISUAL_GEOMETRY", layer="HUMAN_VISUAL", result=geometry_result,
            source_sha256=source_sha, artifact_sha256=artifact_sha, metadata=common_meta,
        )
        tex_cert = None
        if asset.textures:
            tex_cert = self.ctx.certifications.add(
                asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
                test="VISUAL_TEXTURE_MATERIAL", layer="HUMAN_VISUAL", result=texture_result,
                source_sha256=source_sha, artifact_sha256=artifact_sha, metadata=common_meta,
            )
        overall = self._overall(geometry_result, texture_result, bool(asset.textures))
        overall_cert = self.ctx.certifications.add(
            asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
            test="VISUAL_ASSET", layer="HUMAN_VISUAL", result=overall,
            source_sha256=source_sha, artifact_sha256=artifact_sha, metadata=common_meta,
        )
        summary = {
            "asset_id": asset.asset_id, "asset": asset.primary_path, "family": asset.family,
            "mode": mode, "requested_lod": lod, "lod": actual_lod,
            "witness": str(witness), "source_sha256": source_sha, "artifact_sha256": artifact_sha,
            "geometry_visual": geometry_result, "texture_material_visual": texture_result,
            "visual_asset": overall, "texture_prepare": "PASS" if texture_report["ready"] else "FAIL",
            "certification_id": overall_cert["certification_id"], "client_modified": False,
        }
        (work / "summary.json").write_text(json.dumps(summary, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        return summary
    def visual_animation(self, asset: Asset, lod: str = "auto", motion: str = "auto", load_actions: str = "core", fps: int = 30, non_interactive_result: str | None = None) -> dict:
        if asset.visual_mode != "RIGID":
            raise RuntimeError(f"animation gate currently requires self-contained rigid asset; got {asset.family}/{asset.binding_mode}")
        work = self._asset_workdir(asset, "VISUAL_ANIMATION")
        work.mkdir(parents=True, exist_ok=True)

        self.structural(asset)
        overlay_root, texture_report = self._texture_overlay(asset, work)
        src, pristine, actual_lod = self._prepare_rigid_blend(asset, lod, work, overlay_root)
        cfg = next((x for x in asset.configs if x.get("motion_smb")), asset.configs[0])
        motion_inx = cfg.get("motion_inx") or (cfg.get("inx") if asset.binding_source not in {"STRUCTURAL_GATE_UNIQUE", "SKELETON_SIGNATURE_UNIQUE"} else "")
        if not motion_inx:
            raise RuntimeError("root skeleton is structurally bound, but root motion INX is not uniquely resolved")
        inx = self.ctx.resolver.resolve(motion_inx)
        smb = self.ctx.resolver.resolve(cfg["motion_smb"])

        payload = build_bake_payload(self.root, inx, smb, selector=motion, load_actions=load_actions, fps=fps)
        payload_path = work / "motion_bake_payload.json"
        write_payload(payload_path, payload)
        animated = work / "animated_validation.blend"
        bake_report = work / "animation_bake_report.json"
        cmd = self.ctx.settings.blender_command() + [
            "--background", str(pristine), "--python-exit-code", "1",
            "--python", str(self.root / "pt_asset_tool/blender/bake_actions.py"), "--",
            "--payload", str(payload_path), "--out", str(animated), "--report", str(bake_report),
        ]
        run_logged(cmd, work / "02_animation_bake.log")

        if non_interactive_result is None:
            gui_cmd = self.ctx.settings.blender_command() + [str(animated)]
            gui_log = work / "03_blender_gui.log"
            proc = subprocess.run(gui_cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            gui_log.write_text(proc.stdout or "", encoding="utf-8")
            if proc.returncode:
                raise RuntimeError(f"Blender GUI exited rc={proc.returncode}; log={gui_log}")
            rig_result = self._ask("Rig/deformação durante a animação está correto?")
            if rig_result == "FAIL":
                print("animation_visual=SKIP (blocked_by_rig_fail)")
                anim_result = "SKIP"
            else:
                anim_result = self._ask("A animação ativa está visualmente correta?")
            texture_result = self._ask("Texturas e materiais carregaram corretamente?") if asset.textures else "SKIP"
        else:
            v = non_interactive_result.upper()
            if v not in {"PASS","FAIL","SKIP"}: raise ValueError("non_interactive_result must be PASS/FAIL/SKIP")
            rig_result = anim_result = v
            texture_result = v if asset.textures else "SKIP"

        source_sha = sha256_file(src)
        artifact_sha = sha256_file(animated)
        selected = payload["selected_action"]
        selected_spec = next((x for x in payload["actions"] if x["name"] == selected), {})
        common_meta = {
            "animated_blend": str(animated), "pristine_blend": str(pristine), "lod": actual_lod,
            "selected_action": selected, "selected_motion_index": payload["selected_motion_index"],
            "actions_loaded": [x["name"] for x in payload["actions"]], "load_actions": load_actions,
            "fps": fps, "timing_status": "PREVIEW_NOT_RUNTIME_SPEED_CERTIFIED",
            "quaternion_null_semantics": payload.get("quaternion_null_semantics", "UNKNOWN"),
            "source_pose_reference": selected_spec.get("source_pose_reference", "GLOBAL_BIND"),
            "pose_guard": selected_spec.get("pose_guard", "NONE"),
            "pose_guard_bones": selected_spec.get("pose_guard_bones", []),
            "quaternion_sign_continuity": True, "interpolation": "LINEAR",
            "texture_overlay_report": str(work / "texture_overlay_report.json"),
            "binding_mode": asset.binding_mode, "binding_source": asset.binding_source,
            "binding_root_models": asset.binding_root_models,
        }
        for test, result in (("VISUAL_RIG", rig_result), (f"VISUAL_ANIMATION_{selected}", anim_result)):
            self.ctx.certifications.add(asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
                test=test, layer="HUMAN_VISUAL", result=result, source_sha256=source_sha, artifact_sha256=artifact_sha, metadata=common_meta)
        if asset.textures:
            self.ctx.certifications.add(asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
                test="VISUAL_TEXTURE_MATERIAL", layer="HUMAN_VISUAL", result=texture_result, source_sha256=source_sha, artifact_sha256=artifact_sha, metadata=common_meta)
        relevant=[rig_result,anim_result]+([texture_result] if asset.textures else [])
        overall="FAIL" if "FAIL" in relevant else ("PASS" if all(x=="PASS" for x in relevant) else "SKIP")
        cert=self.ctx.certifications.add(asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
            test="VISUAL_ANIMATION_ASSET", layer="HUMAN_VISUAL", result=overall, source_sha256=source_sha, artifact_sha256=artifact_sha, metadata=common_meta)
        summary={
            "asset_id":asset.asset_id,"asset":asset.primary_path,"family":asset.family,"requested_lod":lod,"lod":actual_lod,
            "binding_mode":asset.binding_mode,"binding_source":asset.binding_source,"binding_root_models":asset.binding_root_models,
            "selected_action":selected,"selected_motion_index":payload["selected_motion_index"],
            "actions_loaded":[x["name"] for x in payload["actions"]],"active_motion_count":payload["active_motion_count"],
            "animated_blend":str(animated),"source_sha256":source_sha,"artifact_sha256":artifact_sha,
            "rig_visual":rig_result,"animation_visual":anim_result,"texture_material_visual":texture_result,"visual_animation_asset":overall,
            "preview_fps":fps,"timing_status":"PREVIEW_NOT_RUNTIME_SPEED_CERTIFIED",
            "quaternion_null_semantics":payload.get("quaternion_null_semantics","UNKNOWN"),
            "source_pose_reference":selected_spec.get("source_pose_reference","GLOBAL_BIND"),
            "pose_guard":selected_spec.get("pose_guard","NONE"),
            "pose_guard_bones":selected_spec.get("pose_guard_bones",[]),
            "quaternion_sign_continuity":True,"interpolation":"LINEAR",
            "certification_id":cert["certification_id"],"client_modified":False,
        }
        (work / "summary.json").write_text(json.dumps(summary,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
        return summary

    def animation_diagnostic(self, asset: Asset, lod: str = "auto", motion: str = "ATTACK", load_actions: str = "selected", fps: int = 30, non_interactive: bool = False) -> dict:
        if asset.visual_mode != "RIGID":
            raise RuntimeError(f"animation diagnostic requires self-contained rigid asset; got {asset.family}/{asset.binding_mode}")
        work = self._asset_workdir(asset, "ANIMATION_DIAGNOSTIC")
        work.mkdir(parents=True, exist_ok=True)

        self.structural(asset)
        overlay_root, texture_report = self._texture_overlay(asset, work)
        src, pristine, actual_lod = self._prepare_rigid_blend(asset, lod, work, overlay_root)
        cfg = next((x for x in asset.configs if x.get("motion_smb")), asset.configs[0])
        motion_inx = cfg.get("motion_inx") or (cfg.get("inx") if asset.binding_source not in {"STRUCTURAL_GATE_UNIQUE", "SKELETON_SIGNATURE_UNIQUE"} else "")
        if not motion_inx:
            raise RuntimeError("root skeleton is structurally bound, but root motion INX is not uniquely resolved")
        inx = self.ctx.resolver.resolve(motion_inx)
        smb = self.ctx.resolver.resolve(cfg["motion_smb"])

        payload = build_bake_payload(self.root, inx, smb, selector=motion, load_actions=load_actions, fps=fps)
        payload_path = work / "motion_bake_payload.json"
        write_payload(payload_path, payload)

        animated = work / "animated_validation.blend"
        bake_report = work / "animation_bake_report.json"
        run_logged(self.ctx.settings.blender_command() + [
            "--background", str(pristine), "--python-exit-code", "1",
            "--python", str(self.root / "pt_asset_tool/blender/bake_actions.py"), "--",
            "--payload", str(payload_path), "--out", str(animated), "--report", str(bake_report),
        ], work / "02_animation_bake.log")

        diagnostic = work / "source_vs_armature_diagnostic.blend"
        diagnostic_report = work / "animation_bridge_diagnostic.json"
        run_logged(self.ctx.settings.blender_command() + [
            "--background", str(animated), "--python-exit-code", "1",
            "--python", str(self.root / "pt_asset_tool/blender/diagnose_animation_bridge.py"), "--",
            "--payload", str(payload_path), "--out", str(diagnostic), "--report", str(diagnostic_report),
        ], work / "03_animation_diagnostic.log")

        report = json.loads(diagnostic_report.read_text(encoding="utf-8"))
        direct_result = armature_match = "SKIP"
        if not non_interactive:
            gui_log = work / "04_blender_gui.log"
            proc = subprocess.run(self.ctx.settings.blender_command() + [
                str(diagnostic), "--python", str(self.root / "pt_asset_tool/blender/open_animation_diagnostic.py")
            ], text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            gui_log.write_text(proc.stdout or "", encoding="utf-8")
            if proc.returncode:
                raise RuntimeError(f"Blender GUI exited rc={proc.returncode}; log={gui_log}")
            direct_result = self._ask(f"SOURCE_DIRECT (esquerda, animado) executa {payload.get('selected_action', 'a motion selecionada')} de forma coerente?")
            armature_match = self._ask("BLENDER_ARMATURE (direita, animado) acompanha o SOURCE_DIRECT sem explodir?")

        source_sha = sha256_file(src)
        artifact_sha = sha256_file(diagnostic)
        selected_spec = next((x for x in payload["actions"] if x["name"] == payload["selected_action"]), {})
        meta = {
            "diagnostic_blend": str(diagnostic),
            "diagnostic_report": str(diagnostic_report),
            "selected_action": payload["selected_action"],
            "selected_motion_index": payload["selected_motion_index"],
            "lod": actual_lod,
            "worst_frame": report.get("worst_frame", {}),
            "source_pose_reference": selected_spec.get("source_pose_reference", "GLOBAL_BIND"),
            "pose_guard": selected_spec.get("pose_guard", "NONE"),
            "pose_guard_bones": selected_spec.get("pose_guard_bones", []),
            "comparison": "ANIMATED_SOURCE_DIRECT_vs_BLENDER_ARMATURE",
        }
        if not non_interactive:
            self.ctx.certifications.add(asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
                test="DIAG_SOURCE_DIRECT", layer="HUMAN_VISUAL", result=direct_result, source_sha256=source_sha, artifact_sha256=artifact_sha, metadata=meta)
            self.ctx.certifications.add(asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
                test="DIAG_ARMATURE_MATCH", layer="HUMAN_VISUAL", result=armature_match, source_sha256=source_sha, artifact_sha256=artifact_sha, metadata=meta)

        summary = {
            "asset_id": asset.asset_id, "asset": asset.primary_path, "family": asset.family,
            "requested_lod": lod, "lod": actual_lod,
            "selected_action": payload["selected_action"],
            "selected_motion_index": payload["selected_motion_index"],
            "frames_checked": report.get("frames_checked"),
            "worst_frame": report.get("worst_frame"),
            "top_bone_max_error": report.get("top_bone_max_error", [])[:10],
            "source_pose_reference": selected_spec.get("source_pose_reference", "GLOBAL_BIND"),
            "pose_guard": selected_spec.get("pose_guard", "NONE"),
            "pose_guard_bones": selected_spec.get("pose_guard_bones", []),
            "source_direct_visual": direct_result,
            "armature_match_visual": armature_match,
            "diagnostic_blend": str(diagnostic),
            "diagnostic_report": str(diagnostic_report),
            "source_sha256": source_sha, "artifact_sha256": artifact_sha,
            "client_modified": False,
        }
        (work / "summary.json").write_text(json.dumps(summary, ensure_ascii=False, indent=2)+"\n", encoding="utf-8")
        return summary

