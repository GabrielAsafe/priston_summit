from __future__ import annotations
import collections, pathlib, re
from dataclasses import dataclass, field, asdict
from .utils import low_path, norm_path, read_csv, read_json, stable_id, truth
from .motion_core import parse_inx_link_header, verified_inx_link_target


@dataclass
class Asset:
    asset_id: str
    primary_path: str
    basename: str
    extension: str
    category: str
    family: str
    kind: str
    compatibility_tier: str
    size: int | None
    issues: list
    textures: list[str] = field(default_factory=list)
    configs: list[dict] = field(default_factory=list)
    motions: list[str] = field(default_factory=list)
    binding_mode: str = "NONE"
    binding_source: str = "NONE"
    binding_root_models: list[str] = field(default_factory=list)
    binding_skeleton_smb: str = ""
    binding_notes: list[str] = field(default_factory=list)
    object_count: int | None = None
    material_count: int | None = None
    total_vertices: int | None = None
    total_faces: int | None = None
    source_record: dict = field(default_factory=dict, repr=False)

    @property
    def rigid_binding_ready(self) -> bool:
        return (
            self.family == "SMD_MODEL_062_RIGID_PHYSIQUE"
            and self.binding_mode in {"SELF_CONTAINED", "ROOT_SKELETON_BOUND"}
            and bool(self.binding_skeleton_smb)
            and bool(self.configs)
        )

    @property
    def animation_binding_ready(self) -> bool:
        return self.rigid_binding_ready and len(self.motions) == 1 and any(bool(c.get("motion_smb")) for c in self.configs)

    @property
    def visual_mode(self) -> str | None:
        if self.rigid_binding_ready:
            return "RIGID"
        if self.family == "SMD_MODEL_062_STATIC":
            return "STATIC"
        return None

    def to_dict(self) -> dict:
        d = asdict(self)
        d.pop("source_record", None)
        d["rigid_binding_ready"] = self.rigid_binding_ready
        d["animation_binding_ready"] = self.animation_binding_ready
        d["visual_mode"] = self.visual_mode
        return d


class AssetRegistry:
    def __init__(self, inventory_path: pathlib.Path, pairing_path: pathlib.Path, resolver=None):
        self.inventory_path = pathlib.Path(inventory_path)
        self.pairing_path = pathlib.Path(pairing_path)
        self.resolver = resolver
        self._inx_link_cache: dict[str, dict] = {}
        self.inventory = read_json(self.inventory_path)
        self.pairing = read_csv(self.pairing_path)
        self.binding_cache_path = self.pairing_path.parent / "root_skeleton_bindings_v7.json"
        self.binding_cache = self._load_binding_cache()
        self.canonical_root_saved = self._canonical_root_saved()
        self.records = self._canonical_records()
        self.by_rel = {}
        self.duplicate_relpaths = []
        for r in self.records:
            key = low_path(r.get("relative_path"))
            if key in self.by_rel:
                self.duplicate_relpaths.append(key)
            self.by_rel[key] = r
        self.assets = self._build_assets()
        self.by_id = {a.asset_id: a for a in self.assets}
        self.by_primary = {low_path(a.primary_path): a for a in self.assets}

    def _canonical_root_saved(self) -> str:
        roots = [norm_path(x).rstrip("/") for x in self.inventory.get("roots", [])]
        for r in roots:
            if r.lower().endswith("/cliente"):
                return r
        raise RuntimeError("inventory does not identify a canonical client root ending in /cliente")

    def _canonical_records(self) -> list[dict]:
        prefix = self.canonical_root_saved.rstrip("/") + "/"
        out = []
        for r in self.inventory.get("records", []):
            p = norm_path(r.get("path"))
            if p.startswith(prefix):
                out.append(r)
        return out

    def _load_binding_cache(self) -> dict:
        if not self.binding_cache_path.is_file():
            return {"format": "PT_ROOT_SKELETON_BINDINGS_V7", "entries": {}}
        try:
            data = read_json(self.binding_cache_path)
        except Exception:
            return {"format": "PT_ROOT_SKELETON_BINDINGS_V7", "entries": {}}
        if not isinstance(data, dict):
            return {"format": "PT_ROOT_SKELETON_BINDINGS_V7", "entries": {}}
        data.setdefault("entries", {})
        return data

    def _cached_structural_binding(self, rel: str) -> dict | None:
        entries = self.binding_cache.get("entries") or {}
        row = entries.get(low_path(rel))
        if not isinstance(row, dict) or row.get("status") not in {"UNIQUE_PASS", "UNIQUE_SKELETON", "PHYSIQUE_SUBSET_EQUIVALENT", "CONFIG_EXPLICIT_SKELETON_UNIQUE", "ASSEMBLY_COMPONENT_SKELETON_UNIQUE"}:
            return None
        motion = norm_path(row.get("motion_smb") or row.get("representative_smb"))
        if not motion or not self._record(motion):
            return None
        return row

    def _motion_roots(self, motion_smb: str) -> tuple[list[str], list[str]]:
        roots: set[str] = set()
        inxs: set[str] = set()
        mk = low_path(motion_smb)
        for row in self.pairing:
            if low_path(self._row_motion(row)) != mk:
                continue
            model = self._row_model(row)
            inx = self._row_inx(row)
            if model and truth(row.get("model_resolved")):
                roots.add(model)
            if inx:
                inxs.add(inx)
        return sorted(roots, key=str.lower), sorted(inxs, key=str.lower)

    def _record(self, rel: str) -> dict | None:
        return self.by_rel.get(low_path(rel))

    @staticmethod
    def _row_model(row: dict) -> str:
        return norm_path(row.get("model_smd")) if row.get("model_smd") else ""

    @staticmethod
    def _row_inx(row: dict) -> str:
        return norm_path(row.get("inx")) if row.get("inx") else ""

    @staticmethod
    def _row_motion(row: dict) -> str:
        return norm_path(row.get("motion_smb")) if row.get("motion_smb") else ""

    def _shared_root_candidates(self, row: dict, by_inx_rows: dict[str, list[dict]]) -> list[tuple[str, str]]:
        """Return (root_model_smd, motion_smb) candidates from the same INX.

        A candidate must be a physically resolved model row with a physically
        resolved motion SMB.  This intentionally does not use basename/path
        similarity: the INX membership is the binding evidence.
        """
        inx = self._row_inx(row)
        if not inx:
            return []
        out: set[tuple[str, str]] = set()
        for other in by_inx_rows.get(low_path(inx), []):
            model = self._row_model(other)
            motion = self._row_motion(other)
            if not model or not motion:
                continue
            if not truth(other.get("model_resolved")):
                continue
            root_record = self._record(model)
            if not root_record or str(root_record.get("compatibility_family") or "") != "SMD_MODEL_062_RIGID_PHYSIQUE":
                continue
            # Older pairing reports may omit motion_resolved. If the column is
            # present, require it; otherwise the non-empty canonical path is the
            # strongest evidence available in that report version.
            if "motion_resolved" in other and not truth(other.get("motion_resolved")):
                continue
            out.add((model, motion))
        return sorted(out, key=lambda x: (x[1].lower(), x[0].lower()))

    def _verified_link_root_candidate(self, row: dict, by_inx_rows: dict[str, list[dict]]) -> dict:
        if self.resolver is None:
            return {"status":"NO_RESOLVER"}
        source_inx=self._row_inx(row)
        if not source_inx:
            return {"status":"NO_SOURCE_INX"}
        key=low_path(source_inx)
        if key in self._inx_link_cache:
            return dict(self._inx_link_cache[key])
        src=self.resolver.resolve(source_inx, required=False)
        if src is None:
            out={"status":"SOURCE_INX_MISSING","source_inx":source_inx}
            self._inx_link_cache[key]=out; return dict(out)
        try:
            link=verified_inx_link_target(src)
        except Exception as exc:
            out={"status":"SOURCE_INX_PARSE_FAIL","source_inx":source_inx,"error":f"{type(exc).__name__}: {exc}"}
            self._inx_link_cache[key]=out; return dict(out)
        if link.get("status") != "VERIFIED":
            out={"status":str(link.get("status") or "NO_LINK"),"source_inx":source_inx,
                 "raw_link_file":str(link.get("link_file") or ""),"link_code":link.get("link_code")}
            self._inx_link_cache[key]=out; return dict(out)
        target_inx=norm_path(str(link["target"]))
        target=self.resolver.resolve(target_inx, required=False)
        if target is None:
            out={"status":"TARGET_INX_MISSING","source_inx":source_inx,"target_inx":target_inx,
                 "raw_link_file":str(link.get("link_file") or ""),"link_code":link.get("link_code")}
            self._inx_link_cache[key]=out; return dict(out)
        try:
            target_header=parse_inx_link_header(target)
        except Exception as exc:
            out={"status":"TARGET_INX_PARSE_FAIL","source_inx":source_inx,"target_inx":target_inx,
                 "error":f"{type(exc).__name__}: {exc}"}
            self._inx_link_cache[key]=out; return dict(out)
        if int(target_header.get("file_code") or -1) != int(link.get("link_code")):
            out={"status":"TARGET_FILE_CODE_MISMATCH","source_inx":source_inx,"target_inx":target_inx,
                 "link_code":link.get("link_code"),"target_file_code":target_header.get("file_code")}
            self._inx_link_cache[key]=out; return dict(out)
        target_rows=by_inx_rows.get(low_path(target_inx), [])
        motions=sorted({self._row_motion(r) for r in target_rows if self._row_motion(r)}, key=str.lower)
        roots=sorted({self._row_model(r) for r in target_rows if self._row_model(r) and truth(r.get("model_resolved"))}, key=str.lower)
        if len(motions) != 1:
            out={"status":"TARGET_MOTION_AMBIGUOUS" if motions else "TARGET_MOTION_UNRESOLVED",
                 "source_inx":source_inx,"target_inx":target_inx,"motions":motions,"root_models":roots}
            self._inx_link_cache[key]=out; return dict(out)
        motion=motions[0]
        if not self._record(motion) or self.resolver.resolve(motion, required=False) is None:
            out={"status":"TARGET_MOTION_PHYSICAL_UNRESOLVED","source_inx":source_inx,"target_inx":target_inx,
                 "motion_smb":motion,"root_models":roots}
            self._inx_link_cache[key]=out; return dict(out)
        physical_roots=[r for r in roots if self.resolver.resolve(r, required=False) is not None]
        if roots and len(physical_roots) != len(roots):
            out={"status":"TARGET_ROOT_MODEL_PHYSICAL_UNRESOLVED","source_inx":source_inx,"target_inx":target_inx,
                 "motion_smb":motion,"root_models":roots,"physical_root_models":physical_roots}
            self._inx_link_cache[key]=out; return dict(out)
        out={"status":"VERIFIED","source_inx":source_inx,"target_inx":target_inx,
             "raw_link_file":str(link.get("link_file") or ""),"link_code":int(link.get("link_code")),
             "target_file_code":int(target_header.get("file_code")),"motion_smb":motion,"root_models":roots}
        self._inx_link_cache[key]=out
        return dict(out)

    def _build_assets(self) -> list[Asset]:
        by_model_rows: dict[str, list[dict]] = collections.defaultdict(list)
        by_inx_rows: dict[str, list[dict]] = collections.defaultdict(list)
        for row in self.pairing:
            inx = self._row_inx(row)
            if inx:
                by_inx_rows[low_path(inx)].append(row)
            if truth(row.get("model_resolved")) and row.get("model_smd"):
                by_model_rows[low_path(row["model_smd"])].append(row)

        assets: list[Asset] = []

        def make_asset(record: dict, source_rows: list[dict] | None = None) -> Asset:
            rel = norm_path(record.get("relative_path"))
            rows = source_rows or []
            direct_motions = sorted({self._row_motion(x) for x in rows if self._row_motion(x)})
            binding = "NONE"
            binding_source = "NONE"
            binding_root_models: list[str] = []
            binding_skeleton_smb = ""
            binding_notes: list[str] = []

            inherited_by_row: dict[int, tuple[str, list[str]]] = {}
            inherited_motion_inx_by_row: dict[int, str] = {}
            if len(direct_motions) == 1:
                binding = "SELF_CONTAINED"
                binding_source = "DIRECT_CONFIG_MOTION"
                binding_skeleton_smb = direct_motions[0]
            elif len(direct_motions) > 1:
                binding = "AMBIGUOUS_MULTIPLE_MOTIONS"
                binding_source = "DIRECT_CONFIG_MOTION"
                binding_notes.append(f"direct_motion_count={len(direct_motions)}")
            elif rows:
                # Runtime-authoritative config-only binding. Player modular INX
                # files serialize their root config in LinkFile, usually ending
                # in `.in`; link_code is calculated from the full `.inx` path.
                # We accept the relation only when the source link_code and the
                # target INX file_code both verify and the exact target pairing
                # contains one resolved motion SMB.
                is_rigid_physique = str(record.get("compatibility_family") or "") == "SMD_MODEL_062_RIGID_PHYSIQUE"
                link_attempts=([self._verified_link_root_candidate(row, by_inx_rows) for row in rows]
                               if is_rigid_physique else [])
                linked_rows={i:x for i,x in enumerate(link_attempts) if x.get("status") == "VERIFIED"}
                # Legacy fallbacks are permitted only when every physical source
                # INX was parsed successfully and explicitly says it has no link.
                # Missing/unsupported/corrupt INX evidence stays fail-closed.
                # Non-rigid families preserve their previous registry semantics.
                all_rows_explicitly_no_link=(not is_rigid_physique) or (
                    bool(link_attempts) and all(x.get("status") == "NO_LINK" for x in link_attempts))

                if linked_rows:
                    linked_motions=sorted({norm_path(x.get("motion_smb")) for x in linked_rows.values() if x.get("motion_smb")}, key=str.lower)
                    linked_targets=sorted({norm_path(x.get("target_inx")) for x in linked_rows.values() if x.get("target_inx")}, key=str.lower)
                    if len(linked_rows) == len(rows) and len(linked_motions) == 1 and len(linked_targets) == 1:
                        binding = "ROOT_SKELETON_BOUND"
                        binding_source = "INX_LINK_FILE"
                        binding_skeleton_smb = linked_motions[0]
                        binding_root_models = sorted({root for x in linked_rows.values() for root in (x.get("root_models") or [])}, key=str.lower)
                        inherited_by_row={i:(norm_path(x["motion_smb"]), list(x.get("root_models") or [])) for i,x in linked_rows.items()}
                        inherited_motion_inx_by_row={i:norm_path(x["target_inx"]) for i,x in linked_rows.items()}
                        binding_notes.append(f"linked_config_rows={len(linked_rows)}")
                        binding_notes.append(f"link_target_inx={linked_targets[0]}")
                        binding_notes.append("link_code_verified=true")
                    else:
                        binding = "CONFIG_ONLY"
                        binding_source = "INX_LINK_FILE_AMBIGUOUS"
                        binding_notes.append(f"verified_link_rows={len(linked_rows)}/{len(rows)}")
                        if len(linked_motions) > 1:
                            binding_notes.append(f"conflicting_link_motions={len(linked_motions)}")
                        if len(linked_targets) > 1:
                            binding_notes.append(f"conflicting_link_targets={len(linked_targets)}")
                        failures=collections.Counter(x.get("status") for x in link_attempts if x.get("status") != "VERIFIED")
                        for status,count in sorted(failures.items()):
                            binding_notes.append(f"link_{status.lower()}={count}")
                elif not all_rows_explicitly_no_link:
                    # Never override explicit, unreadable, missing, or otherwise
                    # unverifiable runtime linkage with weaker topology/name inference.
                    binding = "CONFIG_ONLY"
                    binding_source = "INX_LINK_FILE_UNRESOLVED"
                    failures=collections.Counter(x.get("status") for x in link_attempts)
                    for status,count in sorted(failures.items()):
                        binding_notes.append(f"link_{status.lower()}={count}")
                else:
                    # Legacy fallback for assets whose INX truly has no LinkFile.
                    per_row_motions: list[str] = []
                    all_roots: set[str] = set()
                    unresolved_rows = 0
                    ambiguous_rows = 0
                    for i, row in enumerate(rows):
                        candidates = self._shared_root_candidates(row, by_inx_rows)
                        motion_to_roots: dict[str, set[str]] = collections.defaultdict(set)
                        for root_model, motion in candidates:
                            if low_path(root_model) == low_path(rel):
                                continue
                            motion_to_roots[motion].add(root_model)
                        if len(motion_to_roots) == 1:
                            motion = next(iter(motion_to_roots))
                            roots = sorted(motion_to_roots[motion], key=str.lower)
                            inherited_by_row[i] = (motion, roots)
                            per_row_motions.append(motion)
                            all_roots.update(roots)
                        elif len(motion_to_roots) == 0:
                            unresolved_rows += 1
                        else:
                            ambiguous_rows += 1

                    inherited_motions = sorted(set(per_row_motions), key=str.lower)
                    if len(inherited_motions) == 1 and ambiguous_rows == 0 and unresolved_rows == 0:
                        binding = "ROOT_SKELETON_BOUND"
                        binding_source = "INX_SHARED_ROOT"
                        binding_skeleton_smb = inherited_motions[0]
                        binding_root_models = sorted(all_roots, key=str.lower)
                        binding_notes.append(f"inherited_motion={inherited_motions[0]}")
                    else:
                        binding = "CONFIG_ONLY"
                        binding_source = "UNRESOLVED_CONFIG_ONLY"
                        if ambiguous_rows:
                            binding_notes.append(f"ambiguous_config_rows={ambiguous_rows}")
                        if unresolved_rows:
                            binding_notes.append(f"config_rows_without_root={unresolved_rows}")
                        if len(inherited_motions) > 1:
                            binding_notes.append(f"conflicting_inherited_motions={len(inherited_motions)}")

                    # Historical discovery caches are retained only for assets
                    # with no explicit LinkFile evidence.
                    if binding == "CONFIG_ONLY":
                        cached = self._cached_structural_binding(rel)
                        if cached:
                            status=str(cached.get("status") or "")
                            skeleton_motion = norm_path(cached.get("motion_smb") or cached.get("representative_smb"))
                            roots = [norm_path(x) for x in (cached.get("root_models") or []) if x]
                            root_inxs = [norm_path(x) for x in (cached.get("root_inxs") or []) if x]
                            binding = "ROOT_SKELETON_BOUND"
                            binding_source = str(cached.get("source") or "SKELETON_SIGNATURE_UNIQUE")
                            binding_skeleton_smb = skeleton_motion
                            binding_root_models = sorted(set(roots), key=str.lower)
                            binding_notes.append(f"skeleton_smb={skeleton_motion}")
                            binding_notes.append(f"root_inx_count={len(set(root_inxs))}")
                            if status in {"PHYSIQUE_SUBSET_EQUIVALENT", "CONFIG_EXPLICIT_SKELETON_UNIQUE", "ASSEMBLY_COMPONENT_SKELETON_UNIQUE"}:
                                inherited_by_row = {}
                                inherited_motion_inx_by_row = {}
                                binding_notes.append("animation_context=UNRESOLVED")
                            else:
                                inherited_by_row = {i: (skeleton_motion, binding_root_models) for i in range(len(rows))}

            groups = []
            effective_motions: set[str] = set(direct_motions)
            for i, x in enumerate(rows):
                direct_motion = self._row_motion(x)
                inherited = inherited_by_row.get(i)
                effective_motion = direct_motion
                roots: list[str] = []
                binding_origin = "DIRECT" if direct_motion else "NONE"
                motion_inx = self._row_inx(x) if direct_motion else ""
                if not direct_motion and i in inherited_motion_inx_by_row:
                    motion_inx = inherited_motion_inx_by_row[i]
                if not direct_motion and binding == "ROOT_SKELETON_BOUND" and inherited:
                    effective_motion, roots = inherited
                    binding_origin = binding_source
                    effective_motions.add(effective_motion)
                    if binding_source in {"STRUCTURAL_GATE_UNIQUE", "SKELETON_SIGNATURE_UNIQUE", "GATE_FIRST_SKELETON_UNIQUE"}:
                        cached = self._cached_structural_binding(rel) or {}
                        root_inxs = sorted({norm_path(v) for v in (cached.get("root_inxs") or []) if v}, key=str.lower)
                        if len(root_inxs) == 1:
                            motion_inx = root_inxs[0]
                    elif binding_source != "INX_LINK_FILE":
                        motion_inx = self._row_inx(x)
                groups.append({
                    "inx": self._row_inx(x),
                    "motion_inx": motion_inx,
                    "motion_smb": effective_motion,
                    "direct_motion_smb": direct_motion,
                    "motion_binding_origin": binding_origin,
                    "root_model_smd": roots[0] if len(roots) == 1 else "",
                    "root_model_candidates": roots,
                    "motion_required": truth(x.get("motion_required")),
                    "motion_resolved": (truth(x.get("motion_resolved")) if direct_motion else bool(effective_motion)),
                    "lod_groups": {
                        "high": [n.strip() for n in re.split(r"[,;|]", str(x.get("high") or "")) if n.strip()],
                        "default": [n.strip() for n in re.split(r"[,;|]", str(x.get("default") or "")) if n.strip()],
                        "low": [n.strip() for n in re.split(r"[,;|]", str(x.get("low") or "")) if n.strip()],
                    },
                })

            motions = sorted(effective_motions, key=str.lower)
            # A ROOT_SKELETON_BOUND asset must remain single-SMB after effective
            # config expansion. If not, fail closed.
            if binding == "ROOT_SKELETON_BOUND" and len(motions) != 1 and binding_source not in {"PHYSIQUE_SUBSET_EQUIVALENT", "CONFIG_EXPLICIT_REF_UNIQUE", "ASSEMBLY_GRAPH_UNIQUE"}:
                binding_notes.append(f"effective_motion_count={len(motions)}")
                binding = "CONFIG_ONLY"
                binding_source = "UNRESOLVED_CONFIG_ONLY"
                binding_skeleton_smb = ""
                motions = []
                for g in groups:
                    if g.get("motion_binding_origin") in {"INX_SHARED_ROOT", "STRUCTURAL_GATE_UNIQUE", "SKELETON_SIGNATURE_UNIQUE", "GATE_FIRST_SKELETON_UNIQUE", "INX_LINK_FILE"}:
                        g["motion_smb"] = ""
                        g["motion_inx"] = ""
                        g["motion_binding_origin"] = "NONE"
                        g["root_model_smd"] = ""
                        g["root_model_candidates"] = []

            return Asset(
                asset_id=stable_id("asset", rel),
                primary_path=rel,
                basename=str(record.get("basename") or pathlib.PurePosixPath(rel).name),
                extension=str(record.get("extension") or "").lower(),
                category=str(record.get("category_hint") or "UNKNOWN"),
                family=str(record.get("compatibility_family") or "UNKNOWN"),
                kind=str(record.get("kind") or "UNKNOWN"),
                compatibility_tier=str(record.get("compatibility_tier") or ""),
                size=int(record["size"]) if record.get("size") not in (None, "") else None,
                issues=list(record.get("issues") or []),
                textures=[norm_path(x) for x in (record.get("texture_refs") or [])],
                configs=groups,
                motions=motions,
                binding_mode=binding,
                binding_source=binding_source,
                binding_root_models=binding_root_models,
                binding_skeleton_smb=binding_skeleton_smb,
                binding_notes=binding_notes,
                object_count=record.get("object_count"),
                material_count=record.get("material_count"),
                total_vertices=record.get("total_vertices"),
                total_faces=record.get("total_faces"),
                source_record=record,
            )

        # Model/stage/outlier SMDs are the primary 3D assets.
        for r in self.records:
            if str(r.get("extension", "")).lower() != ".smd":
                continue
            relkey = low_path(r.get("relative_path"))
            assets.append(make_asset(r, by_model_rows.get(relkey)))

        # Texture files are useful first-class validation targets.
        for r in self.records:
            ext = str(r.get("extension", "")).lower()
            if ext not in {".bmp", ".tga"}:
                continue
            assets.append(make_asset(r, []))

        assets.sort(key=lambda a: (a.category, a.primary_path.lower()))
        return assets

    def list_assets(self, *, category: str | None = None, family: str | None = None, extension: str | None = None, visual_only: bool = False) -> list[Asset]:
        out = self.assets
        if category:
            q = category.lower()
            out = [a for a in out if a.category.lower() == q]
        if family:
            q = family.lower()
            out = [a for a in out if a.family.lower() == q]
        if extension:
            x = extension.lower()
            if not x.startswith("."):
                x = "." + x
            out = [a for a in out if a.extension == x]
        if visual_only:
            out = [a for a in out if a.visual_mode]
        return out

    def resolve(self, target: str) -> Asset:
        if target in self.by_id:
            return self.by_id[target]
        key = low_path(target)
        if key in self.by_primary:
            return self.by_primary[key]
        exact_name = [a for a in self.assets if a.basename.lower() == str(target).lower()]
        if len(exact_name) == 1:
            return exact_name[0]
        contains = [a for a in self.assets if key in low_path(a.primary_path)]
        if len(contains) == 1:
            return contains[0]
        if not contains and not exact_name:
            raise KeyError(f"asset not found: {target}")
        choices = exact_name if exact_name else contains
        sample = "\n".join(f"  {a.asset_id}  {a.primary_path}" for a in choices[:20])
        raise KeyError(f"asset target is ambiguous ({len(choices)} matches): {target}\n{sample}")

    def family_counts(self) -> list[tuple[str, int]]:
        c = collections.Counter(a.family for a in self.assets)
        return sorted(c.items(), key=lambda x: (-x[1], x[0]))
