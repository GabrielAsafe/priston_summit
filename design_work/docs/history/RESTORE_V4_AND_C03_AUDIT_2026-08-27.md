# Restore V4 + Pikeman C03 Audit

## Bug confirmed in Restore V3

Observed real state:

```text
workspace TmbC01.bmp = registered source SHA
client    TmbC01.bmp = previously published red SHA
```

Restore V3 returned:

```text
SKIP ... reason=UNCHANGED
```

because it skipped as soon as the workspace SHA matched `source_sha256`, before
checking the current client SHA.

That is incorrect for a restoration workflow.

## V4 rule

`UNCHANGED` now means:

```text
workspace == registered source
AND
client    == registered source
```

If:

```text
workspace == registered source
client    == previously published managed build
```

the asset becomes a real candidate:

```text
client_state=PREVIOUSLY_PUBLISHED_BUILD
```

and `--apply` performs the verified backup/write/post-hash transaction.

Unknown client hashes still fail as `CLIENT_DRIFT`.

## C03

`TmbC03.bmp` is structurally ready:

```text
inspect PASS
prepare PASS
noop byte-identical
Blender binding PASS
red test BMP PASS
```

But `M4Bip.inx` does not select:

```text
TbhC03
TbmC03
TblC03
```

in its High/Default/Low groups.

Before runtime-deploying C03, scan the client for another INX that actually
selects those nodes.

## Commands

Restore TmbC01 first with V1.4:

```bash
python3 pt_character_asset_v1_4.py \
  --profile profiles/pt_character_profile_pikeman_c01_m4_v1.json \
  texture-deploy-plan \
  --workspace ./blender_workspace_pikeman_real \
  --role texture
```

Expected:

```text
candidate_count=1
client_state=PREVIOUSLY_PUBLISHED_BUILD
```

Then apply and verify status.

C03 read-only audit:

```bash
python3 pt_pikeman_c03_inx_selector_audit_v1.py \
  --client-root "/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente" \
  --report logs/pikeman_c03_inx_selector_audit_v1.json
```
