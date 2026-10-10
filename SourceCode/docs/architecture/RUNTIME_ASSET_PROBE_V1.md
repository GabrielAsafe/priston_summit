# Runtime Asset Attempt Probe V1

The runtime probe records asset paths at the engine load chokepoints used by both client and server. It is enabled by default.

Outputs are written relative to each executable working directory:

- `LOGS/runtime_asset_attempts.log`: chronological attempt/result timeline (tick, thread, role, category, operation, path).
- `LOGS/runtime_asset_unique.log`: one row per category/path first seen during the process.
- `LOGS/asset_load_v2_events.txt`: existing detailed texture streaming telemetry, also enabled by default again.

Covered runtime asset classes include runtime content packs, model/model-motion ASE and SMD/SMB cache paths, model INI/config files, stages, textures (including async V3 requests), particle scripts, animation image/sequence INIs, WAV/BGM audio and video render paths.

The probe intentionally observes attempts instead of suppressing failures. This makes missing aliases, wrong case/path, stale legacy names and fallback chains visible.

Environment switches:

- `SUMMIT_RUNTIME_ASSET_PROBE=0` disables the runtime-wide attempt probe.
- `SUMMIT_ASSET_TRACE=0` disables the detailed Asset Load V2/V3 texture trace.

From `SourceCode/`, summarize both deployment logs with:

```bash
python3 tools/runtime_asset_probe_summary.py --repo-root .. --show 200
python3 tools/runtime_asset_probe_summary.py --repo-root .. --contains eragon --show 200
```
