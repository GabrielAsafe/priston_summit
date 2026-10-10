# Auditoria dos caminhos legacy UI — snapshot 2026-10-08

Inventário de referências C++ do cliente; **não é prova de que cada linha desenhe UI antiga**. Classificação de risco para migração V2.

## Fronteira atual e riscos

| Caminho | Estado observado | Ação necessária |
|---|---|
| `LegacyNpcDialogWindow::SupportedKind` | Teleportes, SOD, warpgate, research, trade BP/EXP excluídos | Implementar escolhas e ações V2 antes de assumir ownership |
| `LegacyUIV2Bridge.cpp` | Adaptadores V2 existem, mas retain legacy runtime | Reutilizar estado/ações, suprimir apenas desenho antigo |
| `sinHelp.cpp`, `sinInvenTory.cpp` | Tratamento de help e teleport-scroll legacy | Extrair os handlers, não simular clique arbitrário em transações |
| `ReStartFlag`/morte | PATCH 219/220 adicionou modal V2, mas snapshot anterior | Compilar e validar fluxo completo, fallback e timeout |
| Lojas NPC legacy e ContextPanel V2 | Venda de alguns códigos rejeitada | Validar servidor e informar erro antes de migrar mais UI |
| `CoinShop`/`CoinShop_T` | Atalhos e overlays próprios | Inspecionar separadamente sem remover rota funcional |

## Código encontrado (agrupado por ficheiro)

| Ficheiro relativo a `SrcGame/src` | Referências de interesse |
|---|---:|
| `systems/inventory/legacy/sinInvenTory.cpp` | 618 |
| `systems/network/client/netplay.cpp` | 271 |
| `systems/ui/help/legacy_sin/sinHelp.cpp` | 204 |
| `systems/commerce/premium/HaPremiumItem.cpp` | 130 |
| `systems/commerce/legacy/sinTrade.cpp` | 70 |
| `systems/gameplay/quests/legacy/HaQuest.cpp` | 55 |
| `systems/commerce/legacy/sinShop.cpp` | 48 |
| `core/client/legacy_sin_runtime/sinMain.cpp` | 46 |
| `systems/ui/v2/legacy/LegacyNpcDialogWindow.cpp` | 46 |
| `systems/ui/messages/legacy_sin/sinMessageBox.cpp` | 38 |
| `systems/ui/v2/legacy/LegacyContextGateway.cpp` | 36 |
| `systems/ui/hud/legacy_sin/sinInterFace.cpp` | 28 |
| `platform/windows/application/Winmain.cpp` | 23 |
| `systems/ui/v2/legacy/LegacyQuestGateway.cpp` | 20 |
| `core/client/legacy_sin_runtime/sinSubMain.cpp` | 14 |
| `systems/gameplay/events/bellatra/legacy_sin/sinSOD2.cpp` | 14 |
| `systems/ui/v2/legacy/LegacyUIV2Bridge.cpp` | 13 |
| `systems/presentation/opening/HoOpening.cpp` | 11 |
| `systems/gameplay/character/status_ui/sinCharStatus.cpp` | 7 |
| `compat/legacy/presentation/opening_off/HoOpeningOFF.cpp` | 6 |
| `systems/commerce/restoration/cRestaureWindow.cpp` | 6 |
| `core/gameplay/runtime/playmain.cpp` | 5 |
| `systems/commerce/coin_shop/CoinShop.cpp` | 5 |
| `systems/commerce/coin_shop/CoinShop_T.cpp` | 5 |
| `systems/crafting/professions/JOBS.cpp` | 4 |
| `systems/ui/v2/legacy/LegacyNotificationHud.cpp` | 4 |
| `systems/gameplay/teleport/ui/TCore.cpp` | 3 |
| `systems/progression/battle_pass/BattlePass.cpp` | 3 |
| `systems/combat/pvp/war_mode/WarMode.cpp` | 2 |
| `systems/presentation/opening/legacy_runtime/HoLinkHeader.cpp` | 2 |
| `systems/social/clan/tjclan.cpp` | 2 |
| `systems/social/party/ui/cPartyWindow.cpp` | 2 |
| `systems/gameplay/events/hells_gate/cHellsGate.cpp` | 1 |
| `systems/gameplay/world_events/legacy_sin/sinEvent.cpp` | 1 |
| `systems/inventory/legacy/sinItem.cpp` | 1 |
| `systems/social/clan/customization/ClanCustom.cpp` | 1 |
| `systems/ui/confirmation/ConfirmationBox.cpp` | 1 |
| `systems/ui/v2/legacy/LegacyMessageWindow.cpp` | 1 |
| `systems/ui/v2/legacy/LegacySkillGateway.cpp` | 1 |
| `systems/ui/v2/legacy/LegacySkillNpcWindow.cpp` | 1 |

CSV integral: 1749 linhas correspondentes (inclusive estado/dados sem desenho).

## Gates exigidos antes de cada migração

- Não activar `SupportedKind()` para tipos sem ações V2 completas.
- Confirmar fallback/ownership, compatibilidade com JSON e segurança de cliques.
- Cobrir permissões, condições, preço, teleporte, cancela/fecha, esc e retorno de pacote.
- Fazer commits por superfície, nunca um corte global de tudo o que contenha `legacy`.
