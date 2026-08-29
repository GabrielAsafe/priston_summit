from __future__ import annotations
import json, pathlib, subprocess
from dataclasses import dataclass
from .certification import CertificationRegistry
from .config import Settings
from .registry import Asset
from .resolver import ClientResolver
from .texture_overlay import prepare_texture_overlay
from .motion_core import build_bake_payload, write_payload
from .static_roundtrip import compare_static_roundtrip
from .object_local import compact_report, parse_object_local_smd
from .multilayer_texlink import compact_semantics, parse_multilayer_semantics
from .texlink_roundtrip import compare_texlink_roundtrip
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
            smb = self.ctx.resolver.resolve(asset.binding_skeleton_smb)
            structural_report = work / "character_structural.json"
            run_logged([
                "python3", str(self.root / "pt_character_structural_gate_v2.py"),
                "--smd", str(src), "--smb", str(smb), "--report", str(structural_report)
            ], work / "character_structural.log")
            inx_rows = []
            for i, cfg in enumerate(asset.configs):
                if not cfg.get("motion_smb"):
                    continue
                motion_inx = cfg.get("motion_inx") or (cfg.get("inx") if asset.binding_source not in {"STRUCTURAL_GATE_UNIQUE", "SKELETON_SIGNATURE_UNIQUE", "GATE_FIRST_SKELETON_UNIQUE", "PHYSIQUE_SUBSET_EQUIVALENT"} else "")
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
        smb = self.ctx.resolver.resolve(asset.binding_skeleton_smb)
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
            "texture_usage_semantics": texture_report.get("usage_semantics"),
            "texture_required_count": texture_report.get("required_texture_count"),
            "texture_historical_unused_count": texture_report.get("historical_unused_texture_count"),
            "texture_prepare_warning_count": texture_report.get("historical_unused_failures", 0),
            "certification_id": overall_cert["certification_id"], "client_modified": False,
        }
        (work / "summary.json").write_text(json.dumps(summary, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        return summary

    def multilayer_texlink(self, asset: Asset, lod: str = "auto", non_interactive_result: str | None = None) -> dict:
        if asset.kind != "SMD_MODEL_062":
            raise RuntimeError(f"multilayer TexLink requires Model0.62, got {asset.kind}")
        work = self._asset_workdir(asset, "MULTILAYER_TEXLINK")
        work.mkdir(parents=True, exist_ok=True)
        src = self.ctx.resolver.resolve(asset.primary_path)
        source_sha = sha256_file(src)
        parsed = parse_multilayer_semantics(src)
        compact = compact_semantics(parsed)
        source_report = work / "multilayer_source.json"
        source_report.write_text(json.dumps(compact, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        if parsed.get("source_semantics") != "PASS":
            raise RuntimeError("multilayer TexLink source semantics failed")
        if int(parsed.get("max_layers") or 0) != 2:
            raise RuntimeError(f"multilayer preview V1 requires max_layers=2; got {parsed.get('max_layers')}")
        if int(parsed.get("multilayer_faces") or 0) < 1:
            raise RuntimeError("asset has no multilayer TexLink faces")
        mixed_depth = [
            m for m in (parsed.get("multilayer_materials") or [])
            if len(m.get("chain_depth_histogram") or {}) > 1
        ]
        if mixed_depth:
            raise RuntimeError(
                "multilayer preview V1 refuses material ids with mixed 1/2-stage face depth: "
                + ",".join(str(m.get("material_id")) for m in mixed_depth)
            )

        auto_cert = self.ctx.certifications.add(
            asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
            test="MULTILAYER_TEXLINK_SOURCE_SEMANTICS", layer="AUTOMATIC", result="PASS",
            source_sha256=source_sha, metadata={
                "report": str(source_report),
                "max_layers": parsed.get("max_layers"),
                "multilayer_faces": parsed.get("multilayer_faces"),
                "profile_counts": parsed.get("profile_counts"),
                "htexture_slot_mapping": parsed.get("htexture_slot_mapping"),
            },
        )

        overlay_root, texture_report = self._texture_overlay(asset, work)
        if asset.family == "SMD_MODEL_062_RIGID_PHYSIQUE":
            src2, pristine, actual_lod = self._prepare_rigid_blend(asset, lod, work, overlay_root)
            mode = "RIGID"
        elif asset.family == "SMD_MODEL_062_STATIC":
            src2, pristine, actual_lod = self._prepare_static_blend(asset, work, overlay_root)
            mode = "STATIC"
        else:
            raise RuntimeError(f"multilayer Blender preview not wired for family {asset.family}")
        if pathlib.Path(src2).resolve() != pathlib.Path(src).resolve():
            raise RuntimeError("multilayer preview imported an unexpected source")

        preview = work / "multilayer_preview.blend"
        preview_report = work / "multilayer_preview.json"
        script = self.root / "pt_asset_tool/blender/apply_multilayer_texlink.py"
        run_logged(self.ctx.settings.blender_command() + [
            "--background", str(pristine), "--python-exit-code", "1",
            "--python", str(script), "--",
            "--smd", str(src), "--client-root", str(overlay_root),
            "--out", str(preview), "--report", str(preview_report),
        ], work / "02_multilayer_preview.log")
        post = json.loads(preview_report.read_text(encoding="utf-8"))
        if (
            post.get("check") != "PASS"
            or post.get("htexture_slot_mapping") != "SMATERIAL_SM_TEXTURE_SLOT_EXACT"
            or post.get("uv_assignment_check") != "PASS"
            or int(post.get("uv_assignment_mismatch_count") or 0) != 0
            or int(post.get("uv_assignment_depth_mismatch_count") or 0) != 0
        ):
            raise RuntimeError(f"multilayer Blender postprocess failed semantics/UV transport: {post}")

        print(f"MULTILAYER_TEXLINK source={asset.primary_path} mode={mode} lod={actual_lod}")
        print(f"LAYERS=2 multilayer_faces={parsed['multilayer_faces']} uv_distinct_faces={parsed['uv_distinct_multilayer_faces']}")
        print(f"PROFILES={parsed['profile_counts']}")
        print("UV layers PT_TEXLINK_STAGE_0 / PT_TEXLINK_STAGE_1 foram preservados separadamente.")
        print(
            "UV_ASSIGNMENT=PASS "
            f"loops={post.get('uv_assignment_checked_loops')} "
            f"max_error={float(post.get('uv_assignment_max_abs_error') or 0.0):.9g}; "
            "a ordem face/canto foi verificada automaticamente contra o SMD."
        )
        profiles = parsed.get("profile_counts") or {}
        if "REFLEX_ADD" in profiles:
            print("REFLEX_ADD: o stage 1 usa coordenada Reflection no preview; sobreposição no UV Editor não é evidência de UV embaralhado.")
        if "SECOND_UV_LIGHTMAP_MODULATE" in profiles:
            print("LIGHTMAP: o stage 1 possui UV próprio; ilhas sobrepostas/visualmente densas no UV Editor podem ser normais.")
        if int(parsed.get("uv_distinct_multilayer_faces") or 0) == 0:
            print("UV_SERIALIZED: stage 0 e stage 1 não diferem nas faces multilayer deste asset.")
        print("Materiais com FS_SCROLL* são mostrados no frame-base; o movimento do UV dinâmico não é certificado neste gate.")

        if non_interactive_result is None:
            gui_log = work / "03_blender_gui.log"
            proc = subprocess.run(self.ctx.settings.blender_command() + [str(preview)], text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            gui_log.write_text(proc.stdout or "", encoding="utf-8")
            if proc.returncode:
                raise RuntimeError(f"Blender GUI exited rc={proc.returncode}; log={gui_log}")
            layer_result = self._ask("No MODELO renderizado, o efeito da segunda camada (reflexo/lightmap/overlay) aparece nas superfícies corretas, sem artefatos visuais evidentes?")
            texture_result = self._ask("Texturas e materiais do preview multilayer carregaram corretamente?") if asset.textures else "SKIP"
        else:
            layer_result = non_interactive_result.upper()
            if layer_result not in {"PASS", "FAIL", "SKIP"}:
                raise ValueError("non_interactive_result must be PASS/FAIL/SKIP")
            texture_result = layer_result if asset.textures else "SKIP"

        overall = self._overall(layer_result, texture_result, bool(asset.textures))
        meta = {
            "source_report": str(source_report),
            "preview_report": str(preview_report),
            "preview_blend": str(preview),
            "automatic_certification_id": auto_cert["certification_id"],
            "mode": mode, "lod": actual_lod,
            "profile_counts": parsed.get("profile_counts"),
            "multilayer_faces": parsed.get("multilayer_faces"),
            "uv_distinct_multilayer_faces": parsed.get("uv_distinct_multilayer_faces"),
            "htexture_slot_mapping": parsed.get("htexture_slot_mapping"),
            "dynamic_form_preview": post.get("dynamic_form_preview"),
            "uv_assignment_check": post.get("uv_assignment_check"),
            "uv_assignment_checked_loops": post.get("uv_assignment_checked_loops"),
            "uv_assignment_mismatch_count": post.get("uv_assignment_mismatch_count"),
            "uv_assignment_depth_mismatch_count": post.get("uv_assignment_depth_mismatch_count"),
            "uv_assignment_max_abs_error": post.get("uv_assignment_max_abs_error"),
        }
        cert = self.ctx.certifications.add(
            asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
            test="VISUAL_MULTILAYER_TEXLINK", layer="HUMAN_VISUAL", result=overall,
            source_sha256=source_sha, artifact_sha256=sha256_file(preview), metadata=meta,
        )
        summary = {
            "asset_id": asset.asset_id, "asset": asset.primary_path, "family": asset.family,
            "mode": mode, "requested_lod": lod, "lod": actual_lod,
            "max_layers": parsed.get("max_layers"),
            "multilayer_faces": parsed.get("multilayer_faces"),
            "singlelayer_faces": parsed.get("singlelayer_faces"),
            "uv_distinct_multilayer_faces": parsed.get("uv_distinct_multilayer_faces"),
            "profile_counts": parsed.get("profile_counts"),
            "htexture_slot_mapping": parsed.get("htexture_slot_mapping"),
            "uv_layers": post.get("uv_layers"),
            "uv_assignment_check": post.get("uv_assignment_check"),
            "uv_assignment_checked_loops": post.get("uv_assignment_checked_loops"),
            "uv_assignment_mismatch_count": post.get("uv_assignment_mismatch_count"),
            "uv_assignment_depth_mismatch_count": post.get("uv_assignment_depth_mismatch_count"),
            "uv_assignment_max_abs_error": post.get("uv_assignment_max_abs_error"),
            "dynamic_form_preview": post.get("dynamic_form_preview"),
            "combiner_exact_material_count": post.get("combiner_exact_material_count"),
            "combiner_approximate_material_count": post.get("combiner_approximate_material_count"),
            "multilayer_visual": layer_result,
            "texture_material_visual": texture_result,
            "visual_multilayer_asset": overall,
            "texture_prepare": "PASS" if texture_report["ready"] else "FAIL",
            "preview_blend": str(preview),
            "source_sha256": source_sha, "artifact_sha256": sha256_file(preview),
            "certification_id": cert["certification_id"],
            "automatic_certification_id": auto_cert["certification_id"],
            "client_modified": False,
        }
        (work / "summary.json").write_text(json.dumps(summary, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        return summary

    def multilayer_texlink_roundtrip(self, asset: Asset, lod: str = "auto", uv_delta: float = 0.03125) -> dict:
        if asset.kind != "SMD_MODEL_062":
            raise RuntimeError(f"multilayer TexLink roundtrip requires Model0.62, got {asset.kind}")
        if not (1e-5 <= abs(float(uv_delta)) <= 0.25):
            raise ValueError("uv_delta magnitude must be between 1e-5 and 0.25")
        work = self._asset_workdir(asset, "MULTILAYER_TEXLINK_ROUNDTRIP")
        work.mkdir(parents=True, exist_ok=True)
        src = self.ctx.resolver.resolve(asset.primary_path)
        source_sha = sha256_file(src)
        parsed = parse_multilayer_semantics(src)
        if parsed.get("source_semantics") != "PASS":
            raise RuntimeError("multilayer TexLink source semantics failed")
        if int(parsed.get("max_layers") or 0) != 2 or int(parsed.get("multilayer_faces") or 0) < 1:
            raise RuntimeError("TexLink roundtrip V1 requires a source-proven two-stage asset")

        overlay_root, texture_report = self._texture_overlay(asset, work)
        if asset.family == "SMD_MODEL_062_RIGID_PHYSIQUE":
            src2, pristine, actual_lod = self._prepare_rigid_blend(asset, lod, work, overlay_root)
            mode = "RIGID"
        elif asset.family == "SMD_MODEL_062_STATIC":
            src2, pristine, actual_lod = self._prepare_static_blend(asset, work, overlay_root)
            mode = "STATIC"
        else:
            raise RuntimeError(f"TexLink roundtrip not wired for family {asset.family}")
        if pathlib.Path(src2).resolve() != pathlib.Path(src).resolve():
            raise RuntimeError("TexLink roundtrip imported an unexpected source")

        preview = work / "multilayer_pristine.blend"
        preview_report = work / "multilayer_pristine.json"
        apply_script = self.root / "pt_asset_tool/blender/apply_multilayer_texlink.py"
        run_logged(self.ctx.settings.blender_command() + [
            "--background", str(pristine), "--python-exit-code", "1",
            "--python", str(apply_script), "--",
            "--smd", str(src), "--client-root", str(overlay_root),
            "--out", str(preview), "--report", str(preview_report),
        ], work / "02_multilayer_pristine.log")
        preview_meta = json.loads(preview_report.read_text(encoding="utf-8"))
        if preview_meta.get("check") != "PASS" or preview_meta.get("uv_assignment_check") != "PASS":
            raise RuntimeError(f"pristine multilayer preview did not pass: {preview_meta}")

        edited = work / "multilayer_uv_witness.blend"
        witness_report = work / "texlink_uv_witness.json"
        witness_script = self.root / "pt_asset_tool/blender/witness_multilayer_texlink.py"
        run_logged(self.ctx.settings.blender_command() + [
            "--background", str(preview), "--python-exit-code", "1",
            "--python", str(witness_script), "--",
            "--smd", str(src), "--out", str(edited), "--report", str(witness_report),
            "--delta-u", str(float(uv_delta)),
        ], work / "03_texlink_witness.log")
        witness = json.loads(witness_report.read_text(encoding="utf-8"))
        if witness.get("check") != "PASS" or not witness.get("edits"):
            raise RuntimeError(f"TexLink UV witness did not pass: {witness}")

        rebuilt = work / "rebuilt_texlink_roundtrip.smd"
        writer_report = work / "texlink_writer.json"
        writer_script = self.root / "pt_asset_tool/blender/write_multilayer_texlink.py"
        run_logged(self.ctx.settings.blender_command() + [
            "--background", str(edited), "--python-exit-code", "1",
            "--python", str(writer_script), "--",
            "--smd", str(src), "--witness", str(witness_report),
            "--out", str(rebuilt), "--report", str(writer_report),
            "--expected-source-sha256", source_sha,
        ], work / "04_texlink_writer.log")
        writer_meta = json.loads(writer_report.read_text(encoding="utf-8"))
        if writer_meta.get("check") != "PASS" or writer_meta.get("strict_blender_edit_set") != "PASS":
            raise RuntimeError(f"strict TexLink writer did not pass: {writer_meta}")

        compare_report = work / "texlink_roundtrip_compare.json"
        comparison = compare_texlink_roundtrip(src, rebuilt, witness_report, report_path=compare_report)

        # Reimport the rebuilt SMD with the same family/LOD path, then run the
        # automatic source->Blender UV transport gate again on the rebuilt bytes.
        rebuilt_import = work / "rebuilt_import.blend"
        if mode == "RIGID":
            actual_lod2, cfg = self._select_rigid_lod(asset, actual_lod)
            if actual_lod2 != actual_lod:
                raise RuntimeError("rebuilt rigid LOD selection drift")
            inx = self.ctx.resolver.resolve(cfg["inx"])
            smb = self.ctx.resolver.resolve(asset.binding_skeleton_smb)
            run_logged(self.ctx.settings.blender_command() + [
                "--background", "--python-exit-code", "1",
                "--python", str(self.root / "history_py/pt_smd_blender_import_v3_static_armature.py"), "--",
                "--smd", str(rebuilt), "--skeleton", str(smb), "--model-info", str(inx),
                "--lod", actual_lod, "--client-root", str(overlay_root),
                "--out", str(rebuilt_import), "--no-pack",
            ], work / "05_rebuilt_import.log")
        else:
            run_logged(self.ctx.settings.blender_command() + [
                "--background", "--python-exit-code", "1",
                "--python", str(self.root / "pt_smd_blender_import_v4_generic.py"), "--",
                "--smd", str(rebuilt), "--client-root", str(overlay_root),
                "--out", str(rebuilt_import), "--no-pack",
            ], work / "05_rebuilt_import.log")

        rebuilt_preview = work / "rebuilt_multilayer_preview.blend"
        rebuilt_preview_report = work / "rebuilt_multilayer_preview.json"
        run_logged(self.ctx.settings.blender_command() + [
            "--background", str(rebuilt_import), "--python-exit-code", "1",
            "--python", str(apply_script), "--",
            "--smd", str(rebuilt), "--client-root", str(overlay_root),
            "--out", str(rebuilt_preview), "--report", str(rebuilt_preview_report),
        ], work / "06_rebuilt_multilayer_preview.log")
        rebuilt_preview_meta = json.loads(rebuilt_preview_report.read_text(encoding="utf-8"))
        if (
            rebuilt_preview_meta.get("check") != "PASS"
            or rebuilt_preview_meta.get("uv_assignment_check") != "PASS"
            or int(rebuilt_preview_meta.get("uv_assignment_mismatch_count") or 0) != 0
            or int(rebuilt_preview_meta.get("uv_assignment_depth_mismatch_count") or 0) != 0
        ):
            raise RuntimeError(f"rebuilt multilayer reimport/UV gate failed: {rebuilt_preview_meta}")

        rebuilt_parsed = parse_multilayer_semantics(rebuilt)
        if rebuilt_parsed.get("source_semantics") != "PASS":
            raise RuntimeError("rebuilt source semantics failed after TexLink roundtrip")

        edit = (witness.get("edits") or [{}])[0]
        metadata = {
            "mode": mode, "lod": actual_lod,
            "source_report_semantics": parsed.get("source_semantics"),
            "witness_report": str(witness_report), "writer_report": str(writer_report),
            "compare_report": str(compare_report), "rebuilt_smd": str(rebuilt),
            "rebuilt_preview": str(rebuilt_preview), "rebuilt_preview_report": str(rebuilt_preview_report),
            "uv_delta": float(uv_delta), "witness_edit": edit,
            "texlink_structure_exact": comparison.get("texlink_structure_exact"),
            "htexture_nexttex_byte_identical": comparison.get("htexture_nexttex_byte_identical"),
            "non_witness_bytes_byte_identical": comparison.get("non_witness_bytes_byte_identical"),
            "rebuilt_uv_assignment_check": rebuilt_preview_meta.get("uv_assignment_check"),
            "texture_overlay_report": str(work / "texture_overlay_report.json"),
        }
        cert = self.ctx.certifications.add(
            asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
            test="MULTILAYER_TEXLINK_ROUNDTRIP", layer="AUTOMATIC", result="PASS",
            source_sha256=source_sha, artifact_sha256=sha256_file(rebuilt), metadata=metadata,
        )
        print(f"MULTILAYER_TEXLINK_ROUNDTRIP source={asset.primary_path} mode={mode} lod={actual_lod}")
        print(
            "WITNESS "
            f"object={edit.get('object_name')} face={edit.get('face_index')} stage={edit.get('stage_index')} "
            f"corner={edit.get('corner')} component={edit.get('component')} "
            f"before={edit.get('before')} after={edit.get('after')}"
        )
        print(
            "ROUNDTRIP=PASS "
            f"structure_exact={comparison.get('texlink_structure_exact')} "
            f"hTexture_NextTex_byte_identical={comparison.get('htexture_nexttex_byte_identical')} "
            f"non_witness_bytes_byte_identical={comparison.get('non_witness_bytes_byte_identical')}"
        )
        print(
            "REIMPORT_UV=PASS "
            f"loops={rebuilt_preview_meta.get('uv_assignment_checked_loops')} "
            f"max_error={float(rebuilt_preview_meta.get('uv_assignment_max_abs_error') or 0.0):.9g}"
        )
        summary = {
            "asset_id": asset.asset_id, "asset": asset.primary_path, "family": asset.family,
            "mode": mode, "requested_lod": lod, "lod": actual_lod,
            "source_sha256": source_sha, "rebuilt_sha256": sha256_file(rebuilt),
            "rebuilt_smd": str(rebuilt), "rebuilt_preview": str(rebuilt_preview),
            "intentional_edit": "TEXLINK_STAGE1_UV_U", "uv_delta": float(uv_delta),
            "witness_edit": edit,
            "source_semantics": parsed.get("source_semantics"),
            "rebuilt_semantics": rebuilt_parsed.get("source_semantics"),
            "texlink_structure_exact": comparison.get("texlink_structure_exact"),
            "htexture_nexttex_byte_identical": comparison.get("htexture_nexttex_byte_identical"),
            "materials_exact": comparison.get("materials_exact"),
            "non_witness_bytes_byte_identical": comparison.get("non_witness_bytes_byte_identical"),
            "diff_byte_count": comparison.get("diff_byte_count"),
            "rebuilt_uv_assignment_check": rebuilt_preview_meta.get("uv_assignment_check"),
            "rebuilt_uv_assignment_mismatch_count": rebuilt_preview_meta.get("uv_assignment_mismatch_count"),
            "rebuilt_uv_assignment_depth_mismatch_count": rebuilt_preview_meta.get("uv_assignment_depth_mismatch_count"),
            "multilayer_texlink_roundtrip": "PASS",
            "texture_prepare": "PASS" if texture_report["ready"] else "FAIL",
            "certification_id": cert["certification_id"],
            "client_modified": False,
        }
        (work / "summary.json").write_text(json.dumps(summary, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        return summary

    def static_roundtrip(self, asset: Asset, witness_strength: float = 0.08, non_interactive_result: str | None = None) -> dict:
        if asset.family != "SMD_MODEL_062_STATIC":
            raise RuntimeError(f"static roundtrip requires SMD_MODEL_062_STATIC, got {asset.family}")
        if not (0.005 <= float(witness_strength) <= 0.25):
            raise ValueError("witness_strength must be between 0.005 and 0.25")
        work = self._asset_workdir(asset, "STATIC_ROUNDTRIP")
        work.mkdir(parents=True, exist_ok=True)

        structural = self.structural(asset)
        probe = structural.get("static_capability") or {}
        ready = list(probe.get("ready_objects") or [])
        blocked = list(probe.get("blocked_objects") or [])
        object_local_preservation = False
        if blocked:
            rows = list(probe.get("objects") or [])
            ol_rows = [r for r in rows if r.get("name") in blocked and r.get("blockers") and all(str(b).startswith("OBJECT_LOCAL_ANIMATION:") for b in r.get("blockers") or [])]
            if len(rows) == 1 and len(ol_rows) == 1 and not ready:
                target_object = str(ol_rows[0]["name"])
                object_local_preservation = True
            else:
                raise RuntimeError(f"static roundtrip V1 refuses blockers other than a single object-local-animation-only object: {blocked}")
        else:
            if len(ready) != 1:
                raise RuntimeError(f"static roundtrip V1 currently requires exactly one authoring-ready object; ready={ready}")
            target_object = ready[0]

        overlay_root, texture_report = self._texture_overlay(asset, work)
        src, pristine, actual_lod = self._prepare_static_blend(asset, work, overlay_root)
        source_sha = sha256_file(src)

        edited = work / "edited_authoring.blend"
        witness_report = work / "witness_edit.json"
        run_logged(self.ctx.settings.blender_command() + [
            "--background", str(pristine), "--python-exit-code", "1",
            "--python", str(self.root / "pt_asset_tool/blender/witness_geometry.py"), "--",
            "--out", str(edited), "--report", str(witness_report),
            "--strength", str(float(witness_strength)),
        ], work / "02_edit_witness.log")

        witness = json.loads(witness_report.read_text(encoding="utf-8"))
        if str(witness.get("object") or "").lower() != target_object.lower():
            raise RuntimeError(f"witness edited unexpected object: expected={target_object} got={witness.get('object')}")

        # Certification writer: this gate intentionally edits vertex positions only.
        # Preserve every non-XYZ source byte exactly instead of routing the witness
        # through the broader free-mesh writer (whose normal/topology regeneration
        # belongs to a separate authoring capability).
        writer_script = self.root / "pt_asset_tool/blender/write_static_witness.py"
        if not writer_script.is_file():
            raise FileNotFoundError(f"static witness writer missing: {writer_script}")
        rebuilt = work / "rebuilt_roundtrip.smd"
        writer_report = work / "writer.json"
        run_logged(self.ctx.settings.blender_command() + [
            "--background", str(edited), "--python-exit-code", "1",
            "--python", str(writer_script), "--",
            "--smd", str(src), "--object", target_object,
            "--witness", str(witness_report),
            "--out", str(rebuilt), "--report", str(writer_report),
            "--expected-source-sha256", source_sha,
        ], work / "03_static_writer.log")

        compare_report = work / "roundtrip_compare.json"
        comparison = compare_static_roundtrip(src, rebuilt, target_object, witness_report, compare_report)

        rebuilt_probe_path = work / "rebuilt_static_capability.json"
        run_logged([
            "python3", str(self.root / "pt_smd_static_capability_probe_v1.py"),
            "--smd", str(rebuilt), "--report", str(rebuilt_probe_path),
        ], work / "04_rebuilt_probe.log")
        rebuilt_probe = json.loads(rebuilt_probe_path.read_text(encoding="utf-8"))
        if rebuilt_probe.get("ready_objects") != ready or rebuilt_probe.get("blocked_objects") != blocked:
            raise RuntimeError(f"rebuilt capability drift: ready={rebuilt_probe.get('ready_objects')} blocked={rebuilt_probe.get('blocked_objects')}")
        if object_local_preservation:
            src_rows = probe.get("objects") or []
            out_rows = rebuilt_probe.get("objects") or []
            signature = lambda rows: [(r.get("name"), r.get("rot_keys"), r.get("pos_keys"), r.get("scale_keys"), tuple(r.get("blockers") or [])) for r in rows]
            if signature(src_rows) != signature(out_rows):
                raise RuntimeError(f"object-local capability metadata drift: source={signature(src_rows)} output={signature(out_rows)}")

        reimport = work / "rebuilt_reimport.blend"
        run_logged(self.ctx.settings.blender_command() + [
            "--background", "--python-exit-code", "1",
            "--python", str(self.root / "pt_smd_blender_import_v4_generic.py"), "--",
            "--smd", str(rebuilt), "--client-root", str(overlay_root),
            "--out", str(reimport), "--no-pack",
        ], work / "05_reimport.log")

        automatic_meta = {
            "target_object": target_object, "pristine_blend": str(pristine), "edited_blend": str(edited),
            "rebuilt_smd": str(rebuilt), "rebuilt_reimport": str(reimport),
            "witness_report": str(witness_report), "writer_report": str(writer_report),
            "compare_report": str(compare_report), "rebuilt_probe": str(rebuilt_probe_path),
            "witness_strength": float(witness_strength),
            "changed_vertices": comparison.get("details", {}).get("actual_changed_vertices", []),
            "byte_identical": comparison.get("byte_identical"),
            "object_local_animation_preserved_byte_identical": comparison.get("details", {}).get("object_local_animation_preserved_byte_identical"),
            "texture_overlay_report": str(work / "texture_overlay_report.json"),
        }
        auto_cert = self.ctx.certifications.add(
            asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
            test="STATIC_ROUNDTRIP_STRUCTURAL", layer="AUTOMATIC", result="PASS",
            source_sha256=source_sha, artifact_sha256=sha256_file(rebuilt), metadata=automatic_meta,
        )

        print(f"STATIC_ROUNDTRIP source={asset.primary_path} object={target_object}")
        print(f"INTENTIONAL_EDIT vertices={automatic_meta['changed_vertices']} strength={float(witness_strength):.4f}")
        print("Blender abrirá o SMD reconstruído. A pequena deformação do witness deve continuar visível; qualquer outra corrupção é FAIL.")
        if non_interactive_result is None:
            gui_log = work / "06_blender_gui.log"
            proc = subprocess.run(self.ctx.settings.blender_command() + [str(reimport)], text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            gui_log.write_text(proc.stdout or "", encoding="utf-8")
            if proc.returncode:
                raise RuntimeError(f"Blender GUI exited rc={proc.returncode}; log={gui_log}")
            geometry_result = self._ask("Rebuild preservou o objeto e somente o witness intencional está alterado?")
            texture_result = self._ask("Texturas e materiais do rebuild carregaram corretamente?") if asset.textures else "SKIP"
        else:
            geometry_result = non_interactive_result.upper()
            if geometry_result not in {"PASS","FAIL","SKIP"}:
                raise ValueError("non_interactive_result must be PASS/FAIL/SKIP")
            texture_result = geometry_result if asset.textures else "SKIP"

        overall = self._overall(geometry_result, texture_result, bool(asset.textures))
        human_meta = dict(automatic_meta)
        human_meta.update({"automatic_certification_id": auto_cert["certification_id"], "automatic_compare": "PASS", "rebuilt_capability": "PASS"})
        overall_cert = self.ctx.certifications.add(
            asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
            test="STATIC_ROUNDTRIP", layer="HUMAN_VISUAL", result=overall,
            source_sha256=source_sha, artifact_sha256=sha256_file(rebuilt), metadata=human_meta,
        )
        summary = {
            "asset_id": asset.asset_id, "asset": asset.primary_path, "family": asset.family,
            "mode": "STATIC", "target_object": target_object,
            "source_sha256": source_sha, "rebuilt_sha256": sha256_file(rebuilt),
            "rebuilt_smd": str(rebuilt), "edited_blend": str(edited), "rebuilt_reimport": str(reimport),
            "intentional_edit": "GEOMETRY_VERTEX_RADIAL", "witness_strength": float(witness_strength),
            "changed_vertices": automatic_meta["changed_vertices"],
            "byte_identical": comparison.get("byte_identical"),
            "object_local_animation_preserved_byte_identical": comparison.get("details", {}).get("object_local_animation_preserved_byte_identical"),
            "face_uv_material_semantic_exact": comparison.get("details", {}).get("face_uv_material_semantic_exact"),
            "static_roundtrip_structural": "PASS", "rebuilt_capability": "PASS",
            "object_local_animation_preserved": "PASS" if object_local_preservation else "NOT_APPLICABLE",
            "geometry_visual": geometry_result, "texture_material_visual": texture_result,
            "static_roundtrip": overall, "texture_prepare": "PASS" if texture_report["ready"] else "FAIL",
            "texture_usage_semantics": texture_report.get("usage_semantics"),
            "texture_required_count": texture_report.get("required_texture_count"),
            "texture_historical_unused_count": texture_report.get("historical_unused_texture_count"),
            "texture_prepare_warning_count": texture_report.get("historical_unused_failures", 0),
            "certification_id": overall_cert["certification_id"], "automatic_certification_id": auto_cert["certification_id"],
            "client_modified": False,
        }
        (work / "summary.json").write_text(json.dumps(summary, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        return summary

    def object_local_animation(self, asset: Asset, fps: int = 30, non_interactive_result: str | None = None) -> dict:
        if asset.family != "SMD_MODEL_062_STATIC":
            raise RuntimeError(f"object-local animation requires SMD_MODEL_062_STATIC, got {asset.family}")
        if fps <= 0 or fps > 240:
            raise ValueError("fps must be 1..240")
        work = self._asset_workdir(asset, "OBJECT_LOCAL_ANIMATION")
        work.mkdir(parents=True, exist_ok=True)
        src = self.ctx.resolver.resolve(asset.primary_path)
        source_sha = sha256_file(src)

        parsed = parse_object_local_smd(src, strict_v1=True)
        source_report = compact_report(parsed)
        source_report_path = work / "object_local_source_report.json"
        source_report_path.write_text(json.dumps(source_report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        if parsed["animated_object_count"] < 1:
            raise RuntimeError("object-local source is HOLD/NONE: serialized transform arrays exist but no visible local transform changes")
        segmented = [o["name"] for o in parsed["objects"] if o.get("TmFrameCnt")]
        if segmented:
            raise RuntimeError(f"SEGMENTED_OBJECT_LOCAL_UNSUPPORTED_V2 objects={segmented}")
        animated = [o for o in parsed["objects"] if o["motion_class"] == "ANIMATED"]
        channel_union = sorted({c for o in animated for c in o["channels_animated"]})
        max_rot_error = max((float(o.get("rotation_cumulative_max_error") or 0.0) for o in parsed["objects"]), default=0.0)
        legacy_affine = bool(parsed.get("legacy_affine_object_count"))
        zero_scale_spawn = [o for o in animated if o.get("zero_scale_reference_axes")]
        rotation_semantics = "RAW_NONUNIT_QUATERNION_MATRIX" if legacy_affine else "ACCUMULATED_TM_PREV_ROT"
        blender_semantics = "AFFINE_WORLD_DELTA_ABSOLUTE_SHAPE_KEYS" if legacy_affine else "DELTA_TRANSFORMS_OVER_STATIC_BIND"
        auto_cert = self.ctx.certifications.add(
            asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
            test="OBJECT_LOCAL_SOURCE_SEMANTICS", layer="AUTOMATIC", result="PASS", source_sha256=source_sha,
            metadata={"report": str(source_report_path), "object_count": parsed["object_count"],
                      "animated_objects": [o["name"] for o in animated], "channels_animated": channel_union,
                      "parent_edges": parsed.get("parent_edges", []),
                      "unresolved_parents": parsed.get("unresolved_parents", []),
                      "duplicate_node_name_groups": parsed.get("duplicate_node_name_groups", {}),
                      "parent_resolution_semantics": parsed.get("parent_resolution_semantics"),
                      "hierarchy_semantics": "LOCAL_QMAT_TIMES_PARENT_TMRESULT",
                      "blender_transform_semantics": blender_semantics,
                      "rotation_semantics": rotation_semantics,
                      "legacy_affine_object_count": parsed.get("legacy_affine_object_count",0),
                      "legacy_affine_objects": parsed.get("legacy_affine_objects",[]),
                      "zero_scale_spawn_objects": [o["name"] for o in zero_scale_spawn],
                      "scale_reference_semantics": "ZERO_SCALE_SPAWN_EXCEPTION_WHEN_REFERENCE_AXIS_IS_ZERO",
                      "source_rot_key_semantics": "INCREMENTAL_LOCAL_QUATERNION"},
        )

        overlay_root, texture_report = self._texture_overlay(asset, work)
        _src, pristine, _lod = self._prepare_static_blend(asset, work, overlay_root)
        preview = work / "object_local_preview.blend"
        bake_report_path = work / "object_local_bake_report.json"
        run_logged(self.ctx.settings.blender_command() + [
            "--background", str(pristine), "--python-exit-code", "1",
            "--python", str(self.root / "pt_asset_tool/blender/bake_object_local.py"), "--",
            "--smd", str(src), "--out", str(preview), "--report", str(bake_report_path), "--fps", str(int(fps)),
        ], work / "02_object_local_bake.log")
        bake_report = json.loads(bake_report_path.read_text(encoding="utf-8"))
        if bake_report.get("status") != "PASS":
            raise RuntimeError(f"object-local Blender bake did not pass: {bake_report.get('status')}")

        if non_interactive_result is None:
            gui_log = work / "03_blender_gui.log"
            proc = subprocess.run(self.ctx.settings.blender_command() + [str(preview)], text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            gui_log.write_text(proc.stdout or "", encoding="utf-8")
            if proc.returncode:
                raise RuntimeError(f"Blender GUI exited rc={proc.returncode}; log={gui_log}")
            if legacy_affine:
                animation_result = self._ask("Efeito object-local affine legado está visualmente correto (movimento/escala/shear), sem explosões ou saltos espúrios?")
            elif parsed["object_count"] > 1:
                animation_result = self._ask("Movimento multiobjeto/hierarquia está correto (pais, filhos e peças independentes), sem deformar as meshes?")
            else:
                animation_result = self._ask("Movimento object-local (rotação/posição/escala) está correto, sem deformar a mesh?")
            texture_result = self._ask("Texturas e materiais carregaram corretamente?") if asset.textures else "SKIP"
        else:
            animation_result = non_interactive_result.upper()
            if animation_result not in {"PASS", "FAIL", "SKIP"}:
                raise ValueError("non_interactive_result must be PASS/FAIL/SKIP")
            texture_result = animation_result if asset.textures else "SKIP"

        overall = self._overall(animation_result, texture_result, bool(asset.textures))
        artifact_sha = sha256_file(preview)
        cert = self.ctx.certifications.add(
            asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
            test="VISUAL_OBJECT_LOCAL_ANIMATION", layer="HUMAN_VISUAL", result=overall,
            source_sha256=source_sha, artifact_sha256=artifact_sha,
            metadata={"automatic_certification_id": auto_cert["certification_id"], "source_report": str(source_report_path),
                      "bake_report": str(bake_report_path), "preview": str(preview), "preview_fps": int(fps),
                      "object_count": parsed["object_count"], "parent_edge_count": parsed.get("parent_edge_count", 0),
                      "unresolved_parent_count": parsed.get("unresolved_parent_count", 0),
                      "duplicate_node_name_group_count": parsed.get("duplicate_node_name_group_count", 0),
                      "parent_resolution_semantics": parsed.get("parent_resolution_semantics"),
                      "hierarchy_semantics": "LOCAL_QMAT_TIMES_PARENT_TMRESULT",
                      "blender_transform_semantics": blender_semantics,
                      "rotation_semantics": rotation_semantics,
                      "legacy_affine_object_count": parsed.get("legacy_affine_object_count",0),
                      "zero_scale_spawn_object_count": len(zero_scale_spawn),
                      "zero_scale_spawn_objects": [o["name"] for o in zero_scale_spawn],
                      "timing_status": "PREVIEW_NOT_RUNTIME_SPEED_CERTIFIED"},
        )
        summary = {
            "asset_id": asset.asset_id, "asset": asset.primary_path, "family": asset.family, "mode": "STATIC_OBJECT_LOCAL",
            "preview_version": int(bake_report.get("preview_version") or (4 if legacy_affine else 3)), "object_count": parsed["object_count"],
            "animated_object_count": parsed["animated_object_count"], "animated_objects": [o["name"] for o in animated],
            "root_objects": parsed.get("root_objects", []), "parent_edge_count": parsed.get("parent_edge_count", 0),
            "parent_edges": parsed.get("parent_edges", []),
            "unresolved_parent_count": parsed.get("unresolved_parent_count", 0),
            "unresolved_parents": parsed.get("unresolved_parents", []),
            "duplicate_node_name_group_count": parsed.get("duplicate_node_name_group_count", 0),
            "duplicate_node_name_groups": parsed.get("duplicate_node_name_groups", {}),
            "parent_resolution_semantics": parsed.get("parent_resolution_semantics"),
            "channels_animated": channel_union,
            "reference_engine_frame": bake_report.get("reference_engine_frame"), "max_engine_frame": bake_report.get("max_engine_frame"),
            "preview_frame_start": bake_report.get("preview_frame_start"), "preview_frame_end": bake_report.get("preview_frame_end"),
            "preview_fps": int(fps), "preview_reference": bake_report.get("preview_reference"),
            "hierarchy_semantics": "LOCAL_QMAT_TIMES_PARENT_TMRESULT",
            "blender_transform_semantics": blender_semantics,
            "animated_child_count": parsed.get("animated_child_count", 0),
            "legacy_affine_object_count": parsed.get("legacy_affine_object_count",0),
            "legacy_affine_objects": parsed.get("legacy_affine_objects",[]),
            "affine_affected_objects": bake_report.get("affine_affected_objects",[]),
            "zero_scale_spawn_object_count": len(zero_scale_spawn),
            "zero_scale_spawn_objects": [o["name"] for o in zero_scale_spawn],
            "scale_reference_semantics": "ZERO_SCALE_SPAWN_EXCEPTION_WHEN_REFERENCE_AXIS_IS_ZERO",
            "rotation_semantics": rotation_semantics, "source_rot_key_semantics": "INCREMENTAL_LOCAL_QUATERNION",
            "legacy_affine_interpolation": bake_report.get("legacy_affine_interpolation"),
            "interpolation": "LINEAR_SAMPLED_EVERY_SOURCE_TICK", "timing_status": "PREVIEW_NOT_RUNTIME_SPEED_CERTIFIED",
            "rotation_cumulative_max_error": max_rot_error,
            "source_semantics": "PASS", "animation_visual": animation_result, "texture_material_visual": texture_result,
            "visual_object_local_asset": overall, "texture_prepare": "PASS" if texture_report["ready"] else "FAIL",
            "preview_blend": str(preview), "source_sha256": source_sha, "artifact_sha256": artifact_sha,
            "certification_id": cert["certification_id"], "automatic_certification_id": auto_cert["certification_id"],
            "client_modified": False,
        }
        (work / "summary.json").write_text(json.dumps(summary, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        return summary

    def visual_animation(self, asset: Asset, lod: str = "auto", motion: str = "auto", load_actions: str = "core", fps: int = 30, non_interactive_result: str | None = None, head_asset: Asset | None = None, head_variant: dict | None = None) -> dict:
        if asset.visual_mode != "RIGID" or not asset.animation_binding_ready:
            raise RuntimeError(f"animation gate requires a resolved motion context; skeleton binding={asset.binding_mode}/{asset.binding_source} animation_binding_ready={asset.animation_binding_ready}")
        work = self._asset_workdir(asset, "VISUAL_ANIMATION")
        work.mkdir(parents=True, exist_ok=True)

        self.structural(asset)
        if head_asset is not None:
            if head_asset.visual_mode != "RIGID" or not head_asset.animation_binding_ready:
                raise RuntimeError(f"head part has no resolved animation context: {head_asset.primary_path}")
            if head_asset.binding_skeleton_smb.lower() != asset.binding_skeleton_smb.lower():
                raise RuntimeError(f"head/body skeleton SMB mismatch: body={asset.binding_skeleton_smb} head={head_asset.binding_skeleton_smb}")
            body_motion_inx={str(c.get("motion_inx") or "").lower() for c in asset.configs if c.get("motion_inx")}
            head_motion_inx={str(c.get("motion_inx") or "").lower() for c in head_asset.configs if c.get("motion_inx")}
            if len(body_motion_inx)!=1 or head_motion_inx != body_motion_inx:
                raise RuntimeError(f"head/body root motion INX mismatch: body={sorted(body_motion_inx)} head={sorted(head_motion_inx)}")
            self.structural(head_asset)

        body_work=work / "body" if head_asset is not None else work
        body_work.mkdir(parents=True,exist_ok=True)
        overlay_root, texture_report = self._texture_overlay(asset, body_work)
        src, pristine, actual_lod = self._prepare_rigid_blend(asset, lod, body_work, overlay_root)
        cfg = next((x for x in asset.configs if x.get("motion_smb")), asset.configs[0])
        motion_inx = cfg.get("motion_inx") or (cfg.get("inx") if asset.binding_source not in {"STRUCTURAL_GATE_UNIQUE", "SKELETON_SIGNATURE_UNIQUE", "GATE_FIRST_SKELETON_UNIQUE", "PHYSIQUE_SUBSET_EQUIVALENT"} else "")
        if not motion_inx:
            raise RuntimeError("root skeleton is structurally bound, but root motion INX is not uniquely resolved")
        inx = self.ctx.resolver.resolve(motion_inx)
        smb = self.ctx.resolver.resolve(cfg["motion_smb"])

        payload = build_bake_payload(self.root, inx, smb, selector=motion, load_actions=load_actions, fps=fps)
        payload_path = work / "motion_bake_payload.json"
        write_payload(payload_path, payload)

        body_animated = work / ("body_animated.blend" if head_asset is not None else "animated_validation.blend")
        body_bake_report = work / ("body_animation_bake_report.json" if head_asset is not None else "animation_bake_report.json")
        cmd = self.ctx.settings.blender_command() + [
            "--background", str(pristine), "--python-exit-code", "1",
            "--python", str(self.root / "pt_asset_tool/blender/bake_actions.py"), "--",
            "--payload", str(payload_path), "--out", str(body_animated), "--report", str(body_bake_report),
        ]
        run_logged(cmd, work / ("02_body_animation_bake.log" if head_asset is not None else "02_animation_bake.log"))

        animated=body_animated
        head_src=None; head_lod=None; head_texture_report=None; head_animated=None; assembly_report=None
        if head_asset is not None:
            head_work=work / "head"; head_work.mkdir(parents=True,exist_ok=True)
            head_overlay, head_texture_report = self._texture_overlay(head_asset, head_work)
            # Player heads serialize H/M/L as three independent model objects.
            # Match the body's active LOD whenever that head exposes it.
            try:
                head_src, head_pristine, head_lod = self._prepare_rigid_blend(head_asset, actual_lod, head_work, head_overlay)
            except RuntimeError:
                head_src, head_pristine, head_lod = self._prepare_rigid_blend(head_asset, "auto", head_work, head_overlay)
            head_animated=work / "head_animated.blend"
            head_bake_report=work / "head_animation_bake_report.json"
            run_logged(self.ctx.settings.blender_command() + [
                "--background", str(head_pristine), "--python-exit-code", "1",
                "--python", str(self.root / "pt_asset_tool/blender/bake_actions.py"), "--",
                "--payload", str(payload_path), "--out", str(head_animated), "--report", str(head_bake_report),
            ], work / "03_head_animation_bake.log")
            animated=work / "animated_validation.blend"
            assembly_report=work / "player_assembly_report.json"
            run_logged(self.ctx.settings.blender_command() + [
                "--background", str(body_animated), "--python-exit-code", "1",
                "--python", str(self.root / "pt_asset_tool/blender/assemble_player_parts.py"), "--",
                "--part", str(head_animated), "--out", str(animated), "--report", str(assembly_report),
                "--prefix", "PT_HEAD__",
            ], work / "04_player_assembly.log")

        if non_interactive_result is None:
            gui_cmd = self.ctx.settings.blender_command() + [str(animated)]
            gui_log = work / ("05_blender_gui.log" if head_asset is not None else "03_blender_gui.log")
            proc = subprocess.run(gui_cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            gui_log.write_text(proc.stdout or "", encoding="utf-8")
            if proc.returncode:
                raise RuntimeError(f"Blender GUI exited rc={proc.returncode}; log={gui_log}")
            if head_asset is not None:
                assembly_visual=self._ask("Corpo e cabeça estão montados/alinhados corretamente durante a animação?")
                if assembly_visual == "FAIL":
                    rig_result="FAIL"; anim_result="SKIP"
                    print("rig_visual=FAIL animation_visual=SKIP (blocked_by_player_assembly_fail)")
                else:
                    rig_result = self._ask("Rig/deformação do conjunto corpo+cabeça está correto?")
                    if rig_result == "FAIL":
                        print("animation_visual=SKIP (blocked_by_rig_fail)")
                        anim_result = "SKIP"
                    else:
                        anim_result = self._ask("A animação ativa do conjunto está visualmente correta?")
                has_textures=bool(asset.textures or head_asset.textures)
                texture_result = self._ask("Texturas e materiais do corpo+cabeça carregaram corretamente?") if has_textures else "SKIP"
            else:
                assembly_visual="SKIP"
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
            assembly_visual = v if head_asset is not None else "SKIP"
            rig_result = anim_result = v
            texture_result = v if (asset.textures or (head_asset and head_asset.textures)) else "SKIP"

        source_sha = sha256_file(src)
        head_source_sha=sha256_file(head_src) if head_src is not None else None
        artifact_sha = sha256_file(animated)
        selected = payload["selected_action"]
        selected_spec = next((x for x in payload["actions"] if x["name"] == selected), {})
        assembly_mode="BODY_PLUS_INDEPENDENT_HEAD" if head_asset is not None else "SINGLE_ASSET"
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
            "texture_overlay_report": str(body_work / "texture_overlay_report.json"),
            "binding_mode": asset.binding_mode, "binding_source": asset.binding_source,
            "binding_root_models": asset.binding_root_models,
            "assembly_mode":assembly_mode,
            "assembly_visual":assembly_visual,
            "head_asset_id":head_asset.asset_id if head_asset else None,
            "head_asset":head_asset.primary_path if head_asset else None,
            "head_lod":head_lod,
            "head_appearance_key":(head_variant or {}).get("appearance_key") if head_asset else None,
            "head_tier":(head_variant or {}).get("tier_name") if head_asset else None,
            "head_tier_code":(head_variant or {}).get("tier_code") if head_asset else None,
            "head_tier_index":(head_variant or {}).get("tier_index") if head_asset else None,
            "head_tier_kind":(head_variant or {}).get("tier_kind") if head_asset else None,
            "head_descriptor_kind":(head_variant or {}).get("descriptor_kind") if head_asset else None,
            "head_source_sha256":head_source_sha,
            "head_animated_blend":str(head_animated) if head_animated else None,
            "head_texture_overlay_report":str((work / "head" / "texture_overlay_report.json")) if head_asset else None,
            "assembly_report":str(assembly_report) if assembly_report else None,
        }
        for test, result in (("VISUAL_RIG", rig_result), (f"VISUAL_ANIMATION_{selected}", anim_result)):
            self.ctx.certifications.add(asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
                test=test, layer="HUMAN_VISUAL", result=result, source_sha256=source_sha, artifact_sha256=artifact_sha, metadata=common_meta)
        if asset.textures or (head_asset and head_asset.textures):
            self.ctx.certifications.add(asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
                test="VISUAL_TEXTURE_MATERIAL", layer="HUMAN_VISUAL", result=texture_result, source_sha256=source_sha, artifact_sha256=artifact_sha, metadata=common_meta)
        relevant=[rig_result,anim_result]+([texture_result] if (asset.textures or (head_asset and head_asset.textures)) else [])
        if head_asset is not None:
            relevant.append(assembly_visual)
        overall="FAIL" if "FAIL" in relevant else ("PASS" if all(x=="PASS" for x in relevant) else "SKIP")
        if head_asset is not None:
            self.ctx.certifications.add(asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
                test="VISUAL_PLAYER_ASSEMBLY", layer="HUMAN_VISUAL", result=assembly_visual, source_sha256=source_sha, artifact_sha256=artifact_sha, metadata=common_meta)
            self.ctx.certifications.add(asset_id=head_asset.asset_id, asset_path=head_asset.primary_path, family=head_asset.family,
                test="VISUAL_ANIMATION_ASSEMBLED_PART", layer="HUMAN_VISUAL", result=overall, source_sha256=head_source_sha, artifact_sha256=artifact_sha, metadata=common_meta)
        cert=self.ctx.certifications.add(asset_id=asset.asset_id, asset_path=asset.primary_path, family=asset.family,
            test="VISUAL_ANIMATION_ASSET", layer="HUMAN_VISUAL", result=overall, source_sha256=source_sha, artifact_sha256=artifact_sha, metadata=common_meta)
        summary={
            "asset_id":asset.asset_id,"asset":asset.primary_path,"family":asset.family,"requested_lod":lod,"lod":actual_lod,
            "binding_mode":asset.binding_mode,"binding_source":asset.binding_source,"binding_root_models":asset.binding_root_models,
            "assembly_mode":assembly_mode,"assembly_visual":assembly_visual,
            "head_asset_id":head_asset.asset_id if head_asset else None,"head_asset":head_asset.primary_path if head_asset else None,"head_lod":head_lod,
            "head_appearance_key":(head_variant or {}).get("appearance_key") if head_asset else None,
            "head_tier":(head_variant or {}).get("tier_name") if head_asset else None,
            "head_tier_code":(head_variant or {}).get("tier_code") if head_asset else None,
            "head_tier_index":(head_variant or {}).get("tier_index") if head_asset else None,
            "head_tier_kind":(head_variant or {}).get("tier_kind") if head_asset else None,
            "selected_action":selected,"selected_motion_index":payload["selected_motion_index"],
            "actions_loaded":[x["name"] for x in payload["actions"]],"active_motion_count":payload["active_motion_count"],
            "animated_blend":str(animated),"source_sha256":source_sha,"head_source_sha256":head_source_sha,"artifact_sha256":artifact_sha,
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
        if asset.visual_mode != "RIGID" or not asset.animation_binding_ready:
            raise RuntimeError(f"animation diagnostic requires a resolved motion context; skeleton binding={asset.binding_mode}/{asset.binding_source} animation_binding_ready={asset.animation_binding_ready}")
        work = self._asset_workdir(asset, "ANIMATION_DIAGNOSTIC")
        work.mkdir(parents=True, exist_ok=True)

        self.structural(asset)
        overlay_root, texture_report = self._texture_overlay(asset, work)
        src, pristine, actual_lod = self._prepare_rigid_blend(asset, lod, work, overlay_root)
        cfg = next((x for x in asset.configs if x.get("motion_smb")), asset.configs[0])
        motion_inx = cfg.get("motion_inx") or (cfg.get("inx") if asset.binding_source not in {"STRUCTURAL_GATE_UNIQUE", "SKELETON_SIGNATURE_UNIQUE", "GATE_FIRST_SKELETON_UNIQUE", "PHYSIQUE_SUBSET_EQUIVALENT"} else "")
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

