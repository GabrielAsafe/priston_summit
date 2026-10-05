#!/usr/bin/env python3
from pathlib import Path
import re, sys
root = Path(__file__).resolve().parents[2]
errors=[]
headers=[
    "Shared/rendering/legacy_3d/smStage3d.h",
    "Shared/rendering/legacy_3d/stage/smStage3d_legacy_client.h",
    "Shared/rendering/legacy_3d/stage/smStage3d_legacy_server.h",
]
fields=("VertexCapacity","FaceCapacity","TexLinkCapacity","LightCapacity")
for rel in headers:
    p=root/rel
    if not p.exists():
        errors.append(f"{rel}: missing")
        continue
    text=p.read_text(encoding="utf-8",errors="replace")
    for field in fields:
        if not re.search(r"\bint\s+"+re.escape(field)+r"\s*;", text):
            errors.append(f"{rel}: missing {field}")
for rel in (
    "Shared/rendering/legacy_3d/stage/smStage3d_legacy_client.inl",
    "Shared/rendering/legacy_3d/stage/smStage3d_legacy_server.inl",
):
    text=(root/rel).read_text(encoding="utf-8",errors="replace")
    for token in (
        "stage_storage::ResetStorageState(*this)",
        "stage_storage::InitializeOwnedStorage(*this, nv, nf)",
        "stage_storage::ReleaseOwnedStorage(*this)",
        "LightCapacity = nl",
        "nLight >= LightCapacity",
    ):
        if token not in text:
            errors.append(f"{rel}: missing lifecycle guard: {token}")
if errors:
    print("\n".join(errors), file=sys.stderr)
    sys.exit(1)
print("PATCH 197 gate: OK — stage header parity and owned-storage lifecycle are safe")
