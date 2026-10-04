# Summit — status do trabalho

## Lista inicial

Todos os temas iniciais têm agora uma solução técnica definida e um caminho de implementação/teste. Os dois temas de história foram promovidos a milestones próprios (M01/M02).

## Implementações/POCs já no bundle

- World phase runtime monotônico — smoke PASS.
- World story contribution service idempotente/CAS-oriented — smoke PASS.
- Equipment set service server-authoritative — smoke PASS.
- Covenant altar integration patch.
- EXP INT64 patch e party 2..8 config patch.
- canonical/model config sync tooling.
- canonical balance/equipment/world/head content templates.
- PT Asset Tool integration helpers.

## Trabalhos que exigem aplicação na checkout real

O bundle não altera diretamente a checkout local do usuário. Os pontos abaixo estão especificados/patchados, mas precisam ser aplicados/rebuildados no repositório real:

- BR001 map field53/StageArea;
- SQLite repository real para M01;
- gateway legado real para M02;
- equip event/stat integration do M05;
- save/network/render do M06;
- promover model config JSON a source oficial e gerar cache;
- integrar screens/workflows no PT Asset Tool.

## Regra para `/Servidor/char`

Não apagar hoje. O server ainda consome uma parte real de model configs, SMD/SMB e `tmABCD`. Fazer M11 antes de reduzir agressivamente a pasta.
