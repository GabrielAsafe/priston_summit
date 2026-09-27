# Canonical content source of truth

`SourceCode/Content/canonical` is the permanent, versionable source of truth for Summit runtime content.

The executable deployments are standalone. Neither `Game.exe` nor `Server.exe` reads canonical JSON, `SourceCode`, environment variables, or an external migration tree at runtime. They read only the compiled packs in their own `Content/packs` directory.

## One-time consolidation from the portable migration tree

If the newest canonical tree still lives outside the repository, import it once:

```bash
cd <repo-root>
python3 SourceCode/tools/content_port/consolidate_canonical_content.py \
  --repo-root . \
  --import-root <portable-canonical-root>
```

The import replaces only the external migration families `maps/`, `stage_spawns/` and `text/`. `item_profiles/` remains owned by the Summit source tree because the client/server executable catalogs define those profiles.

After the command succeeds, the external canonical tree is no longer needed for builds or runtime.

## Normal build flow

`SourceCode/compboth.sh` builds both executables and then calls `consolidate_canonical_content.py` **without** `--import-root`.

That step:

1. validates the canonical tree inside `SourceCode`;
2. verifies the four Forever Fall maps have resolved actors and weights summing to 100;
3. builds `definitions_client.scfg`, `definitions_server.scfg`, `maps_client.smap`, `maps_server.smap`, `stage_spawns.sspb`, `text_resources.sres`, `localization.sloc` and `gameplay_config.sgcf` from the internal canonical source;
4. validates item materialization and the internal runtime contract;
5. deploys the role-appropriate packs to `cliente/Content/packs` and `Servidor/Content/packs`;
6. validates hashes of every deployed pack.

A successful build writes `SourceCode/Content/CANONICAL_SOURCE.json` and `SourceCode/Content/BUILD.json`.

## Distribution contract

Ship:

- `cliente/` with its `Content/` directory;
- `Servidor/` with its `Content/` directory.

Do not ship `SourceCode` as a runtime dependency.
