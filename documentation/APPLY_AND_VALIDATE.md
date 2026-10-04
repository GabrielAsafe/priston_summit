# Ordem de aplicação e validação

## 1. Conteúdo/tooling sem alterar comportamento

1. copiar schemas/tools novos;
2. integrar `model_configs` e `model_config_id`;
3. rodar `validate_extended_content.py`;
4. rodar `sync_model_config_catalog.py` contra os assets reais;
5. revisar os 8 model refs ausentes no servidor;
6. manter `legacy_variants` inativos.

## 2. EXP/balance

1. aplicar `fix_srGetTotalExp_int64.patch`;
2. aplicar `configure_party_2_to_8.patch`;
3. gerar `ExpManager.ini` a partir de `experience.json`;
4. executar smoke de solo/party 2..8;
5. comparar XP antes/depois para 2..6 (deve manter paridade).

## 3. Covenant altar

1. aplicar `covenant_altar.patch`;
2. rebuild client/server;
3. verificar que tecla/aba Covenant existente continua normal;
4. spawnar apenas um altar de teste;
5. clicar e confirmar state packet + abertura da aba;
6. preencher/validar placement plan mapa a mapa;
7. aplicar placements e recompilar `field_runtime.sfld`.

## 4. Equipment sets

1. mover `EquipmentSetService` para `Shared/gameplay/equipment_sets/v2`;
2. adicionar loader/codegen do canonical;
3. integrar com evento de equip/unequip server-side;
4. aplicar modifiers no cálculo autoritativo;
5. migrar os dois sets legados mantendo paridade;
6. só então remover cálculo hardcoded do client.

## 5. Maps

Implementar `MAP_CONTENT_DRIVEN_BUGREPORT.md` antes de criar field53 normal.

## 6. World phases

1. map ticket pronto;
2. integrar `WorldPhaseRuntime` ao event hub;
3. criar gateway que resolve profiles + relocations;
4. testar phase sem geometry (só mobs/NPCs);
5. testar uma cidade A -> B;
6. restart e relog na phase B;
7. corrida de dois triggers simultâneos.

## 7. Head cosmetic

Somente depois do audit de slot/protocolo. Primeiro cosmético sem stats; depois helmet defensivo se necessário.

## 8. Server char cleanup

Não apagar antes dos passos anteriores. Gerar `render-safe` manifest, fazer deployment controlado, verificar boot/spawn/login. Headless cleanup vem por último.
