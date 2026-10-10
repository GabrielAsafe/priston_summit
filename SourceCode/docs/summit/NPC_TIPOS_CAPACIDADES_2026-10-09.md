# NPCs Summit — Tipos, capacidades e rotas de UI (auditoria 2026-10-09)

Fonte: 175 JSON NPC de `Content/canonical/text/resources/Servidor/GameServer/NPC/` do snapshot 2026-10-08. Contagens por **campo declarado** (não prova de NPC ativo).

## Modelo de capacidades

Uma definição NPC tem identidade (`name`), modelo (`model_file`), estado (`state`) e **capacidades independentes**; não existe apenas um tipo exclusivo. Campos de venda listam códigos de itens, não garantem que compra esteja funcional. Campos `unmapped.*` ainda requerem investigação antes de migrar.

| Capacidade/campo | Nº NPCs com o campo | Papel provável / cuidado |
|---|---:|---|
| `sell_weapon` | 48 | Vendedor de armas — compra deve validar servidor |
| `sell_defence` | 48 | Vendedor de armaduras/defesas |
| `sell_etc` | 45 | Consumíveis/outros itens |
| `quest_event` | 14 | Evento/quest |
| `event_npc` | 9 | NPC de evento |
| `dialog` | 7 | Texto/diálogo |
| `warehouse` | 6 | Armazém persistente |
| `event_code` | 6 | Requer inspeção da implementação legacy |
| `teleport` | 5 | Teleporte NPC (destinos, custo e requisitos legacy) |
| `shop` | 4 | Interação de loja genérica |
| `skill_master` | 4 | Mestre de skills |
| `skill_change_job` | 4 | Mudança de classe |
| `item_aging` | 3 | Aging/reforço |
| `event_info` | 3 | Informação do evento |
| `clan_npc` | 2 | Clãs |
| `gift_express` | 2 | Entrega de presentes |
| `event_gift` | 2 | Presente de evento |
| `refining` | 2 | Refinação |
| `time_shop` | 2 | Loja de tempo |
| `mixture_reset` | 2 | Reset de mistura |
| `quest_code` | 2 | Código de quest |
| `skin_change` | 1 | Mudança de aparência |
| `bless_castle` | 1 | Castelo |
| `battle_pass` | 1 | Battle Pass |
| `ranking` | 1 | Ranking |
| `polling` | 1 | Sondagem |
| `item_mix` | 1 | Mistura de itens |

## Newter, o Sábio

- JSON: `Newter, o S#U00e1bio.NPC.json`; campo `teleport: true`; `model_file: char\npc\MN-008\MN-008.INI`.
- `LegacyNpcDialogWindow::SupportedKind()` devolve `false` para `SIN_HELP_KIND_TELEPORT`, `SIN_HELP_KIND_C_TELEPORT`, `SIN_HELP_KIND_TELEPORT_SCROLL`, `SIN_HELP_KIND_TELEPORT_MILTER`, `SIN_HELP_KIND_WARPGATE`, `SIN_HELP_KIND_SOD`, `SIN_HELP_KIND_RESEARCH` e `SIN_HELP_KIND_TRADEBPEXP`.
- **Não ativar esses tipos só mudando `SupportedKind()`**: é necessária lista de destinos V2, escolha delegada à transação/teleporte legacy, requisitos, custos, fecho e cancelamento.

## Inventário NPC por definição (campos observados)

| Ficheiro NPC | Nome do campo | Capacidades declaradas |
|---|---|---|
| `7crystal.npc.json` | 노점상 델라 | `quest_event` |
| `Ahin.NPC.json` | 아힌 | `sell_etc` |
| `Arad.npc.json` | 착한소년 롱보 | `skin_change` |
| `Arena de Batalha.NPC.json` | 대장장이 고덴 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `BABELQUEST.NPC.json` | 수집가 홉고블린씨 | `quest_event` |
| `BC-Reporter.npc.json` | 블레스캐슬 리포터 | apenas interação/modelo |
| `Barba.NPC.json` | 각인사 바르바 | apenas interação/modelo |
| `Baurin.NPC.json` | 바우린 | `quest_event` |
| `Bcn01.npc.json` | 잡화상 아이시리스 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Bcn02.npc.json` | 노점상 쉘비 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Bcn03.npc.json` | 노점상 로잘린 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Bcn04.npc.json` | 노점상 로사 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Bcn05.npc.json` | 집사 앤더스 | `bless_castle` |
| `Bcn06.npc.json` | 창고지기 던컨 | `warehouse` |
| `Bellia.npc.json` | 집시 벨리아 | apenas interação/modelo |
| `Celina.NPC.json` | 현자 셀리나 | apenas interação/modelo |
| `Chocolate.npc.json` | 초코 상인 | `quest_event`, `dialog` |
| `ClanMaster.NPC.json` | 미스터 부마 | `clan_npc` |
| `Coin shop.NPC.json` | — | `shop` |
| `CovenantAltar.npc.json` | Covenant Altar | `event_code` |
| `Cristais.npc.json` | 노점상 델라 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Deamon sabio.NPC.json` | 안내자 데이몬 | `teleport` |
| `Distribuidor.NPC.json` | 물품 교부처 | `gift_express` |
| `Donation Box.NPC.json` | 잡화점 알리아 | `sell_weapon`, `sell_defence` |
| `Dos.NPC.json` | 소켓 장인 도스 | apenas interação/modelo |
| `Dzari.NPC.json` | 드자리 공주 | `event_npc` |
| `Enzo, o Restaure.NPC.json` | — | apenas interação/modelo |
| `Eto.npc.json` | 조사원 에토 | `quest_event` |
| `Eventgirl.NPC.json` | 이벤트 걸 | `event_gift` |
| `Force-master.NPC.json` | 포스 마스터 | `refining` |
| `HEventgirl.NPC.json` | 입장권 판매원 | `event_npc` |
| `Hermit.NPC.json` | 겁쟁이 헤밋 | `warehouse` |
| `Hosean.npc.json` | 장로 호세안 | apenas interação/modelo |
| `Hunter.NPC.json` | 사냥꾼 오웬 | apenas interação/modelo |
| `Item Shop.NPC.json` | — | `shop` |
| `Itens 60.NPC.json` | 대장장이 엄프 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Jin.NPC.json` | 모험가 진 | apenas interação/modelo |
| `Kamiyu.npc.json` | 조사원 카미유 | `quest_event` |
| `KarasTears.npc.json` | 카라의 후손 | `quest_event` |
| `Lista_mix.NPC.json` | — | `event_code` |
| `Loja VIP.NPC.json` | 잡화점 비안트 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Loja da Allya.NPC.json` | 잡화점 알리아 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Loja de Creditos.NPC.json` | — | `shop` |
| `Loja de Cristais.NPC.json` | 노점상 델라 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Loja de Pedras.NPC.json` | 잡화점 비안트 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Loja do Repot I1.NPC.json` | 잡화점 알리아 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Loja do Repot I2.NPC.json` | 잡화점 알리아 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Loja do Repot S1.NPC.json` | 잡화점 알리아 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Loja do Tempo.NPC.json` | — | `time_shop` |
| `Loja premiun.NPC.json` | 잡화점 비안트 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Lucy.NPC.json` | 믹스처리셋 장인 나디아 | `mixture_reset` |
| `MagicalGreen.npc.json` | 마력연구가 | `quest_event` |
| `Marina.npc.json` | 마리나 | `sell_etc` |
| `Master Raymon!.NPC.json` | 마법수련소 레이몬 | `skill_master`, `skill_change_job` |
| `Mestre Raymon.NPC.json` | 마법수련소 레이몬 | `skill_master`, `skill_change_job` |
| `Mestre Real Ray.NPC.json` | 로열메이지 레이 | apenas interação/modelo |
| `Moriph, o Misterioso.NPC.json` | 아케인 모리프 | `item_aging` |
| `Nadia.NPC.json` | 착한소년 롱보 | `battle_pass` |
| `Newter, o S#U00e1bio.NPC.json` | 현명한 뉴터 | `teleport` |
| `PRISTON.npc.json` | 롱보 여자친구 | `quest_event` |
| `Phillai Quest Shop.NPC.json` | 퀘스트 담당자 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Pillai Quest Shop.NPC.json` | 퀘스트 담당자 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Pumpkin.npc.json` | 호박 상인 | `quest_event` |
| `Quest De Tier.NPC.json` | 퀘스트 담당자 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Real_ClanMaster.NPC.json` | 클랜 마스터 | `clan_npc` |
| `Resercher.npc.json` | 마력연구가 | `quest_event` |
| `Ricio.NPC.json` | 룬스톤 장인 리시오 | apenas interação/modelo |
| `SHunter.NPC.json` | — | apenas interação/modelo |
| `Sal#U00e3o de Beleza.NPC.json` | Nadia | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Sanc-guard.npc.json` | 사원관리인 | `dialog` |
| `Sheltom Shop, Moriph's Son.NPC.json` | — | apenas interação/modelo |
| `Sheltons.NPC.json` | 잡화점 비안트 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Skadi.NPC.json` | 모험가 스카디 | apenas interação/modelo |
| `Soccerfan Nego Ney.NPC.json` | — | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Soccerfan.NPC.json` | — | `sell_weapon`, `sell_defence`, `sell_etc` |
| `Sod_01.NPC.json` | 카사 | `event_npc` |
| `Sod_02.NPC.json` | 아리엘 | `event_npc` |
| `Sod_03.NPC.json` | 실피 | `event_npc` |
| `Sod_04.NPC.json` | 노아스 | `event_npc` |
| `Sod_05.NPC.json` | Secretaria Karina | apenas interação/modelo |
| `Sod_Buma.NPC.json` | 부마 | `event_npc` |
| `Sod_Goblin.NPC.json` | 홉고블린 | `event_npc` |
| `Soul.NPC.json` | 떠도는 영혼 | apenas interação/modelo |
| `Star Girl.NPC.json` | 산타마리아 | `ranking` |
| `Status.NPC.json` | 이벤트 걸 | `event_gift` |
| `T_Messenger.NPC.json` | 왕국의 전령 이안 | apenas interação/modelo |
| `Teacher01.npc.json` | 도우미 프리아 | `event_code` |
| `Xmasguy.npc.json` | 크리스 마스 롱보 | apenas interação/modelo |
| `Zarad, o Elengante.NPC.json` | 대장장이 고덴 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `acasia-store.NPC.json` | 잡화점 린 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `amanda.NPC.json` | — | apenas interação/modelo |
| `blesscatle-guard.npc.json` | 블레스캐슬 가드 | apenas interação/modelo |
| `bridge-guard.npc.json` | 다리 경비병 | apenas interação/modelo |
| `bronzWolverin.npc.json` | 브론즈 울버린 | `event_code`, `event_info` |
| `coinshop.npc.json` | — | `shop` |
| `coward.npc.json` | 겁쟁이 | apenas interação/modelo |
| `cristal.NPC.json` | 노점상 델라 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `dailyquest.npc.json` | asd | `mixture_reset` |
| `derik.NPC.json` | 기사단장 데릭 | apenas interação/modelo |
| `desert-guard.npc.json` | 사막 경비병 | apenas interação/modelo |
| `donation-box.NPC.json` | 산타마리아 | apenas interação/modelo |
| `dungeon-keeper.npc.json` | 던전 키퍼 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `enquete.npc.json` | 설문조사 도우미 | `polling` |
| `flypitcher-store.npc.json` | 노점상 델라 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `fury.npc.json` | 퓨리 | apenas interação/modelo |
| `goldenWolverin.npc.json` | 골덴 울버린 | `event_code`, `event_info` |
| `itempost.npc.json` | 물품 교부처 | `gift_express` |
| `ji_woo.npc.json` | — | apenas interação/modelo |
| `lake-guard.npc.json` | 호수 경비병 | apenas interação/modelo |
| `lim.npc.json` | 잡화점 비안트 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `milter.npc.json` | 저주에 걸린 밀터 | `teleport`, `dialog` |
| `mn-013.npc.json` | 안내자 데이몬 | `teleport` |
| `mrcave-keeper.npc.json` | 동굴 경비병 | apenas interação/modelo |
| `navisko-civilian2.npc.json` | 마을주민 미뉴렌 | apenas interação/modelo |
| `navisko-dragoman.npc.json` | 네비스코 가드 | apenas interação/modelo |
| `navisko-equip.npc.json` | 대장장이 엄프 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `navisko-magic.npc.json` | 마론 마법상점 3호점 | `sell_weapon`, `sell_defence` |
| `navisko-store.npc.json` | 잡화점 비안트 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `navisko-warehouse.npc.json` | 창고지기 미프 | `warehouse`, `dialog` |
| `nevisko-magic.npc.json` | 마론 마법상점 3호점 | `sell_weapon`, `sell_defence` |
| `novic.npc.json` | 노비치 | apenas interação/modelo |
| `npc_Hobsanta.NPC.json` | 산타고블린 | `quest_event`, `dialog` |
| `npc_ninefox.NPC.json` | 프레이아 | `quest_event` |
| `phillay-civilian1.npc.json` | 몰리 할머니 | `quest_code` |
| `phillay-civilian2.npc.json` | 현명한 뉴터 | `teleport` |
| `phillay-civilian3.NPC.json` | 아케인 대리 뉴터 | `item_aging` |
| `phillay-dragoman.npc.json` | 필라이 가드 | apenas interação/modelo |
| `phillay-dragoman1.npc.json` | 필라이 가드 | apenas interação/modelo |
| `phillay-equip1.npc.json` | 대장장이 바츠 | `sell_weapon`, `sell_defence` |
| `phillay-equip2.npc.json` | 대장장이 드프레인 | `quest_code`, `sell_weapon`, `sell_defence` |
| `phillay-guard1.npc.json` | 필라이 경비병 | apenas interação/modelo |
| `phillay-guard2.npc.json` | 필라이 경비병 | apenas interação/modelo |
| `phillay-guard3.npc.json` | 감옥 경비병 | apenas interação/modelo |
| `phillay-imbue.npc.json` | 아케인 모리프 | `item_aging` |
| `phillay-magic.npc.json` | 마법상점 마론 | `sell_weapon`, `sell_defence` |
| `phillay-master.npc.json` | 마법수련소 레이몬 | `skill_master`, `skill_change_job` |
| `phillay-store.npc.json` | 잡화점 필리아 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `phillay-warehouse.npc.json` | 창고지기 라벤다 | `warehouse` |
| `premium.npc.json` | 잡화점 비안트 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `puzzleking.npc.json` | 퍼즐수집가 라몬트 | `quest_event` |
| `quest.npc.json` | 퀘스트 담당자 | apenas interação/modelo |
| `ray.NPC.json` | 로열메이지 레이 | apenas interação/modelo |
| `ricarden-civilian1.npc.json` | 착한소년 롱보 | apenas interação/modelo |
| `ricarden-civilian2.npc.json` | 참견하는 갈딘 | `dialog` |
| `ricarden-civilian3.npc.json` | 고민많은 자콘 | `dialog` |
| `ricarden-equip1.NPC.json` | 대장장이 고덴 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `ricarden-equip2.NPC.json` | 대장장이 거스 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `ricarden-guard.npc.json` | 리카르텐 가드 | apenas interação/modelo |
| `ricarden-guard1.npc.json` | 리카르텐 경비병 | apenas interação/modelo |
| `ricarden-guard2.npc.json` | 리카르텐 경비병 | apenas interação/modelo |
| `ricarden-imbue.NPC.json` | 조합장인 모랄드 | `item_mix` |
| `ricarden-master.npc.json` | 스킬마스터 베르칸 | `skill_master`, `skill_change_job` |
| `ricarden-store.NPC.json` | 잡화점 알리아 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `ricarden-warehouse.npc.json` | 창고지기 아렌트 | `warehouse` |
| `rosemari.npc.json` | 이벤트걸 별난소녀 | apenas interação/modelo |
| `ruiden-civilian1.npc.json` | 방랑자 로스트 | apenas interação/modelo |
| `ruiden-equip.npc.json` | 대장장이 루가 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `ruiden-magic.npc.json` | 마론 마법상점 2호점 | `sell_weapon`, `sell_defence` |
| `ruiden-store.NPC.json` | 노상잡화점 루네 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `salon.npc.json` | 착한소년 롱보 | apenas interação/modelo |
| `sellquest.NPC.json` | 부마 | `event_npc`, `sell_etc` |
| `shelton.npc.json` | 잡화점 비안트 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `sillverWolverin.npc.json` | 실버 울버린 | `event_code`, `event_info` |
| `templer.npc.json` | 신전사제 | apenas interação/modelo |
| `timeshop.npc.json` | — | `time_shop` |
| `tmcave-keeper.npc.json` | 동굴 경비병 | apenas interação/modelo |
| `toolspilai.NPC.json` | 대장장이 고덴 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `toolsric.NPC.json` | 대장장이 고덴 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `wfzone.npc.json` | 워프게이트 발명가 | `sell_etc` |
| `yura-Force-master.NPC.json` | 포스 마스터 | `refining` |
| `yura-guard.npc.json` | 유라 경비병 | apenas interação/modelo |
| `yura-guard1.npc.json` | 유라 경비병 | apenas interação/modelo |
| `yura-guard2.npc.json` | 유라 경비병 | apenas interação/modelo |
| `yura-store.npc.json` | 잡화점 로렌 | `sell_weapon`, `sell_defence`, `sell_etc` |
| `yura-warehouse.npc.json` | 창고지기 워렌트 | `warehouse` |

## Contrato de migração

1. Resolver apresentação e strings pelo V2/localização JSON; os dados canónicos permanecem separados da lógica de gameplay.
2. Uma janela V2 só assume o NPC quando implementa **todas** as suas ações, incluindo regressões de teleporte, compra, taxa, requisitos e cancelamento.
3. Não remover os comandos legacy do servidor: usar adaptadores seguros, mantendo a autoridade de transação no servidor.
4. Validar as capabilities declaradas contra os NPCs efetivamente publicados no runtime e os seus handlers.
5. Para migração completa, criar regression gates por tipo e exercitar pelo menos um NPC real de cada tipo.
