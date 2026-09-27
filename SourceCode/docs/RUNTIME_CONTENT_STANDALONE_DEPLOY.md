# Runtime content standalone deploy

## Contract

`SourceCode/Content/packs` is a **build-time source**, not a runtime dependency.
A production client and server are standalone and must contain their own compiled content:

```text
cliente/
  Game.exe
  Content/
    RUNTIME_CONTENT.json
    packs/
      definitions_client.scfg
      maps_client.smap
      stage_spawns.sspb
      text_resources.sres
      localization.sloc
      gameplay_config.sgcf

Servidor/
  Server.exe
  Content/
    RUNTIME_CONTENT.json
    packs/
      definitions_server.scfg
      maps_server.smap
      stage_spawns.sspb
      text_resources.sres
      localization.sloc
      gameplay_config.sgcf
```

`Game.exe` and `Server.exe` resolve content only from `Content/packs` next to their own executable tree. They do not read `SourceCode`, shell environment variables, or the caller's working directory to activate compiled content.

## Integrated build flow

`compboth.sh` owns the complete local artifact flow:

1. build/link/send the client through `cliente.sh`;
2. build/link/send the server through `server.sh`;
3. validate compiled item materialization and NPC shop coverage;
4. copy the correct runtime packs into `cliente/Content` and `Servidor/Content`;
5. validate deployed files against the source packs by SHA-256.

The content deployment runs only after both build scripts return success. `server.sh` is fail-fast, so server compile/link errors propagate to `compboth.sh` instead of being masked by a later send command. A content validation/deploy failure likewise makes `compboth.sh` fail instead of leaving a build that appears ready while its runtime packs are stale or absent.

Normal development command:

```bash
cd SourceCode
sh compboth.sh
```

A manual content-only refresh remains available when no recompilation is needed:

```bash
python3 SourceCode/tools/content_port/deploy_runtime_content.py --repo-root .
python3 SourceCode/tools/content_port/validate_runtime_pack_resolution.py --repo-root .
```

## Item materialization invariant

The historical `DecodeItemInfo` zeroed the complete item definition before parsing. The server-side `sDEF_ITEMINFO` constructor did not zero its embedded `sITEMINFO Item`, so using only C++ value-initialization in the compiled adapter could leave fields such as `UniqueItem` indeterminate.

Compiled materialization now explicitly initializes the embedded item to zero before applying directives, and the server constructor also zeros `Item`. This is required for shop linking, item creation and any code that assumes omitted legacy fields default to zero.

The build preflight runs:

```bash
python3 tools/content_port/validate_compiled_item_materialization.py --source-root .
```

It checks the zero-initialization contract and verifies that every server-resolvable NPC shop code has a normal (`UniqueItem == 0`) direct OpenItem record in `definitions_server.scfg`, including the six Della potion codes.

## Distribution

Distribute/copy the `cliente/` and `Servidor/` trees **with their `Content/` directories**. Do not ship `SourceCode/` as a runtime dependency.
