from __future__ import annotations
import collections, pathlib, re
from dataclasses import dataclass, field, asdict
from .utils import low_path, norm_path, read_csv, read_json, stable_id, truth


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
    object_count: int | None = None
    material_count: int | None = None
    total_vertices: int | None = None
    total_faces: int | None = None
    source_record: dict = field(default_factory=dict, repr=False)

    @property
    def is_model(self) -> bool:
        return self.extension == ".smd"

    @property
    def visual_mode(self) -> str | None:
        if self.family == "SMD_MODEL_062_RIGID_PHYSIQUE" and self.binding_mode == "SELF_CONTAINED" and self.configs:
            return "RIGID"
        if self.family == "SMD_MODEL_062_STATIC":
            return "STATIC"
        return None

    def to_dict(self) -> dict:
        d = asdict(self)
        d.pop("source_record", None)
        d["visual_mode"] = self.visual_mode
        return d


class AssetRegistry:
    def __init__(self, inventory_path: pathlib.Path, pairing_path: pathlib.Path):
        self.inventory_path = pathlib.Path(inventory_path)
        self.pairing_path = pathlib.Path(pairing_path)
        self.inventory = read_json(self.inventory_path)
        self.pairing = read_csv(self.pairing_path)
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

    def _record(self, rel: str) -> dict | None:
        return self.by_rel.get(low_path(rel))

    def _build_assets(self) -> list[Asset]:
        by_model_rows: dict[str, list[dict]] = collections.defaultdict(list)
        for row in self.pairing:
            if truth(row.get("model_resolved")) and row.get("model_smd"):
                by_model_rows[low_path(row["model_smd"])].append(row)

        assets: list[Asset] = []
        seen: set[str] = set()

        def make_asset(record: dict, configs: list[dict] | None = None) -> Asset:
            rel = norm_path(record.get("relative_path"))
            cfgs = configs or []
            motions = sorted({norm_path(x.get("motion_smb")) for x in cfgs if x.get("motion_smb")})
            if len(motions) == 1:
                binding = "SELF_CONTAINED"
            elif len(motions) == 0:
                binding = "CONFIG_ONLY" if cfgs else "NONE"
            else:
                binding = "AMBIGUOUS_MULTIPLE_MOTIONS"
            groups = []
            for x in cfgs:
                groups.append({
                    "inx": norm_path(x.get("inx")),
                    "motion_smb": norm_path(x.get("motion_smb")),
                    "motion_required": truth(x.get("motion_required")),
                    "motion_resolved": truth(x.get("motion_resolved")),
                    "lod_groups": {
                        "high": [n.strip() for n in re.split(r"[,;|]", str(x.get("high") or "")) if n.strip()],
                        "default": [n.strip() for n in re.split(r"[,;|]", str(x.get("default") or "")) if n.strip()],
                        "low": [n.strip() for n in re.split(r"[,;|]", str(x.get("low") or "")) if n.strip()],
                    },
                })
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
            seen.add(relkey)

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
