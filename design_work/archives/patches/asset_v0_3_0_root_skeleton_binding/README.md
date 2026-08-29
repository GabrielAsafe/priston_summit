# PT Asset Tool V0.3.0 — Config-only → Root Skeleton Binding

## Contexto

A cobertura visual de animação rigid foi fechada em representantes de NPC/mob (Ahin, Acero e Monbagon).
O próximo bloqueio do roadmap era permitir que modelos rigid que aparecem no INX sem `motion_smb` próprio
(config-only) reutilizem o skeleton root compartilhado pelo mesmo INX, sem heurística por nome/path.

## Nova regra

Novo `binding_mode`:

    ROOT_SKELETON_BOUND

Um asset config-only só recebe esse binding quando:

1. ele pertence a um ou mais INX conhecidos no pairing;
2. em cada INX existe candidato(s) de model resolvido com `motion_smb` resolvido;
3. o root candidato pertence à família `SMD_MODEL_062_RIGID_PHYSIQUE`;
4. todos os candidatos daquele INX convergem para um único SMB efetivo;
5. todas as linhas config do asset convergem para o mesmo SMB;
6. nenhuma linha config fica sem root e nenhuma apresenta SMBs conflitantes.

Se qualquer condição falhar, o asset continua `CONFIG_ONLY` e não ganha acesso aos gates rigid.

## Evidência/proveniência

`asset inspect` agora expõe:

- `binding_mode`
- `binding_source`
- `binding_root_models`
- `binding_notes`
- por config: `direct_motion_smb`, `motion_binding_origin`, `root_model_smd`,
  `root_model_candidates`

Para binding herdado:

    binding_source = INX_SHARED_ROOT
    motion_binding_origin = INX_SHARED_ROOT

O SMB herdado passa a ser o `motion_smb` efetivo daquela config somente após a resolução inequívoca.

## Integração com os gates

`Asset.rigid_binding_ready` aceita:

- `SELF_CONTAINED`
- `ROOT_SKELETON_BOUND`

Os dois continuam exigindo exatamente um SMB efetivo e configs válidas.

Structural/visual/animation não ignoram validações: o structural gate executa o mesmo
`pt_character_structural_gate_v2.py` contra o SMD config-only + SMB root. Portanto uma associação
semanticamente incompatível ainda falha antes do Blender.

## Novo comando

    python3 pt_asset.py bindings

Filtros:

    python3 pt_asset.py bindings --mode ROOT_SKELETON_BOUND --limit 100
    python3 pt_asset.py bindings --category CHARACTER --limit 100

O comando mostra asset, categoria, binding, SMB efetivo, root model(s) e path.

## Segurança

- Nenhum SMD/SMB/INX é alterado.
- Nenhum arquivo do cliente é alterado.
- Ambiguidade continua bloqueada.
- Não há matching por basename, proximidade de diretório ou suposição de naming convention.
- O vínculo usa pertencimento ao mesmo INX + SMB resolvido + família rigid do root.
- `client_modified=false` permanece para os gates Blender.

## Testes locais

- `python3 -m compileall`: PASS.
- Fixture sintética: root self-contained + peça config-only no mesmo INX => `ROOT_SKELETON_BOUND`: PASS.
- Fixture sintética com dois SMBs distintos no mesmo INX => permanece `CONFIG_ONLY`: PASS.

O runtime real do cliente/inventory não está disponível neste ambiente; a contagem real de bindings deve
ser auditada no PC do projeto com `python3 pt_asset.py bindings` antes de escolher o primeiro representante.
