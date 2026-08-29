from __future__ import annotations
import os, pathlib, shlex
from dataclasses import dataclass


@dataclass(frozen=True)
class Settings:
    project_root: pathlib.Path
    client_root: pathlib.Path
    inventory: pathlib.Path
    pairing: pathlib.Path
    certifications_dir: pathlib.Path
    work_dir: pathlib.Path

    @classmethod
    def discover(cls, project_root: str | pathlib.Path | None = None, client_root: str | pathlib.Path | None = None):
        if project_root is None:
            env = os.environ.get("PT_PROJECT_ROOT")
            if env:
                root = pathlib.Path(env).expanduser().resolve()
            else:
                here = pathlib.Path(__file__).resolve()
                root = here.parent.parent
        else:
            root = pathlib.Path(project_root).expanduser().resolve()

        if client_root is None:
            env = os.environ.get("PT_CLIENT_ROOT")
            if env:
                client = pathlib.Path(env).expanduser().resolve()
            else:
                client = pathlib.Path.home() / "Área de Trabalho/summit/Summit Tale Client Full/cliente"
        else:
            client = pathlib.Path(client_root).expanduser().resolve()

        return cls(
            project_root=root,
            client_root=client,
            inventory=root / "logs/asset_similarity_v1_1/asset_inventory.json",
            pairing=root / "logs/asset_family_refine_v2/inx_physical_pairing.csv",
            certifications_dir=root / "certifications",
            work_dir=root / "pt_build/pt_asset_tool",
        )

    def blender_command(self) -> list[str]:
        raw = os.environ.get("PT_BLENDER_COMMAND")
        if raw:
            return shlex.split(raw)
        return ["flatpak", "run", "--filesystem=home", "org.blender.Blender"]
