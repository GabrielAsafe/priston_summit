# Encoding Logic Priority Report

Triagem automática de corrupção com maior probabilidade de afetar gameplay/lookup.

## Resumo

- Achados de lógica priorizados: 4637
- Linhas com recuperação HIGH disponível: 862
- Candidatos de recuperação totais associados: 16252

## Domínios

- **OTHER_LOGIC**: 2762
- **ITEM**: 1402
- **PATH_ASSET**: 643
- **NPC_SHOP_WAREHOUSE**: 356
- **MONSTER_SPAWN**: 339
- **QUEST**: 33
- **NETWORK_PROTOCOL**: 14

## Arquivos prioritários

| Arquivo | Achados | Score máximo |
|---|---:|---:|
| `game:Server/OnSever.cpp` | 177 | 160 |
| `game:fileread.cpp` | 326 | 155 |
| `server:fileread.cpp` | 223 | 125 |
| `game:sinbaram/sinShop.cpp` | 1 | 125 |
| `game:sinbaram/sinItem.cpp` | 155 | 120 |
| `server:Server/OnSever.cpp` | 136 | 115 |
| `server:record.cpp` | 18 | 115 |
| `server:sinbaram/sinMsg.cpp` | 631 | 110 |
| `game:sinbaram/sinMsg.cpp` | 630 | 110 |
| `server:BattlePass.cpp` | 24 | 110 |
| `game:sinbaram/HaPremiumItem.cpp` | 1 | 110 |
| `server:sinbaram/sinInterFace.cpp` | 23 | 100 |
| `game:CoinShop.cpp` | 2 | 95 |
| `game:CoinShop_T.cpp` | 2 | 95 |
| `game:Server/E_serverlog.h` | 2 | 95 |
| `server:Server/E_serverlog.h` | 2 | 95 |
| `game:sinbaram/sinCraftItem_Server.h` | 127 | 90 |
| `server:sinbaram/sinCraftItem_Server.h` | 127 | 90 |
| `game:sinbaram/sinCraftItem_Client.h` | 115 | 90 |
| `server:sinbaram/sinCraftItem_Client.h` | 115 | 90 |
| `shared:b_sinCraftItem.h` | 24 | 90 |
| `game:sinbaram/sinInvenTory.cpp` | 14 | 90 |
| `game:sinbaram/sinEffect2.cpp` | 13 | 90 |
| `server:sinbaram/sinEffect2.cpp` | 13 | 90 |
| `server:sinbaram/sinInvenTory.cpp` | 9 | 90 |
| `game:HoBaram/HoOpening.cpp` | 7 | 90 |
| `game:sinbaram/sinHelp.cpp` | 5 | 90 |
| `server:sinbaram/sinHelp.cpp` | 5 | 90 |
| `game:sinbaram/AssaUtil.cpp` | 4 | 90 |
| `server:sinbaram/AssaUtil.cpp` | 4 | 90 |
| `game:cMixGuide.cpp` | 1 | 90 |
| `game:sinbaram/sinHelpPet.cpp` | 1 | 90 |
| `server:sinbaram/sinHelpPet.cpp` | 1 | 90 |
| `game:cSettingsWindow.cpp` | 16 | 85 |
| `server:cHellsGate.cpp` | 9 | 85 |
| `game:TJBOY/clanmenu/clan_Enti.cpp` | 36 | 80 |
| `server:TJBOY/clanmenu/clan_Enti.cpp` | 36 | 80 |
| `server:News/News.cpp` | 4 | 80 |
| `game:sinbaram/sinSkill.cpp` | 244 | 75 |
| `game:TJBOY/isaocheck/isaocheck.cpp` | 208 | 75 |
| `server:TJBOY/isaocheck/isaocheck.cpp` | 208 | 75 |
| `server:sinbaram/sinSkill.cpp` | 171 | 75 |
| `server:Server/Svr_Damge.cpp` | 21 | 75 |
| `server:JOBS.cpp` | 17 | 75 |
| `game:TJBOY/park/MicVolume.cpp` | 7 | 75 |
| `server:TJBOY/park/MicVolume.cpp` | 7 | 75 |
| `game:character.cpp` | 6 | 75 |
| `server:character.cpp` | 6 | 75 |
| `game:sinbaram/sinTrade.cpp` | 2 | 75 |
| `server:Server/CRollHandler.cpp` | 2 | 75 |
| `server:sinbaram/sinTrade.cpp` | 1 | 75 |
| `game:TJBOY/Ygy/Util.cpp` | 43 | 70 |
| `server:TJBOY/Ygy/Util.cpp` | 43 | 70 |
| `game:TJBOY/Ygy/Packet/PacketStruct.h` | 4 | 70 |
| `server:TJBOY/Ygy/Packet/PacketStruct.h` | 4 | 70 |
| `game:TJBOY/Ygy/Packet/Packet.cpp` | 1 | 70 |
| `server:TJBOY/Ygy/Packet/Packet.cpp` | 1 | 70 |
| `game:TJBOY/clanmenu/cE_chip.cpp` | 62 | 65 |
| `server:TJBOY/clanmenu/cE_chip.cpp` | 62 | 65 |
| `game:TJBOY/clanmenu/cE_user.cpp` | 30 | 65 |
| `server:TJBOY/clanmenu/cE_user.cpp` | 30 | 65 |
| `game:netplay.cpp` | 14 | 65 |
| `server:netplay.cpp` | 10 | 65 |
| `game:TJBOY/clanmenu/cE_CViewClanInfo.cpp` | 5 | 65 |
| `server:TJBOY/clanmenu/cE_CViewClanInfo.cpp` | 5 | 65 |
| `game:TJBOY/clanmenu/cE_chatlist.cpp` | 4 | 65 |
| `server:TJBOY/clanmenu/cE_chatlist.cpp` | 4 | 65 |
| `game:smLib3d/smTexture.cpp` | 3 | 65 |
| `server:smLib3d/smTexture.cpp` | 3 | 65 |
| `game:AreaServer.cpp` | 2 | 65 |
| `game:checkname.h` | 2 | 65 |
| `server:AreaServer.cpp` | 2 | 65 |
| `server:checkname.h` | 2 | 65 |
| `game:cQuestWindow.cpp` | 20 | 55 |
| `server:TJBOY/clanmenu/tjclan.cpp` | 13 | 55 |
| `game:TJBOY/clanmenu/tjclan.cpp` | 11 | 55 |
| `game:Ranking.cpp` | 7 | 55 |
| `game:playmain.cpp` | 6 | 55 |
| `game:TJBOY/clanmenu/cE_Notice.cpp` | 5 | 55 |
| `game:Winmain.bak2.cpp` | 5 | 55 |
| `game:Winmain.cpp` | 5 | 55 |
| `server:TJBOY/clanmenu/cE_Notice.cpp` | 5 | 55 |
| `game:Server/Svr_Damge.cpp` | 3 | 55 |
| `game:WarMode.cpp` | 3 | 55 |
| `game:sinbaram/sinInterFace.cpp` | 3 | 55 |
| `game:TJBOY/clanmenu/GuideHelp.cpp` | 2 | 55 |
| `game:TJBOY/clanmenu/cE_Cmake.cpp` | 2 | 55 |
| `game:cracker.cpp` | 2 | 55 |
| `server:TJBOY/clanmenu/GuideHelp.cpp` | 2 | 55 |
| `server:TJBOY/clanmenu/cE_Cmake.cpp` | 2 | 55 |
| `server:cracker.cpp` | 2 | 55 |
| `game:TJBOY/isaocheck/auth.cpp` | 59 | 45 |
| `server:TJBOY/isaocheck/auth.cpp` | 59 | 45 |
| `game:TJBOY/clanmenu/cE_CJoin.cpp` | 18 | 45 |
| `server:TJBOY/clanmenu/cE_CJoin.cpp` | 18 | 45 |
| `game:TCore.cpp` | 15 | 45 |
| `game:sinbaram/sinSubMain.cpp` | 11 | 45 |
| `server:sinbaram/sinSubMain.cpp` | 11 | 45 |
| `game:TJBOY/clanmenu/cE_isclanmember.cpp` | 10 | 45 |
| `server:TJBOY/clanmenu/cE_isclanmember.cpp` | 10 | 45 |

## Top 100 achados

| Score | Domínio | Target | Linha | Razões |
|---:|---|---|---:|---|
| 160 | ITEM,MONSTER_SPAWN,NETWORK_PROTOCOL,NPC_SHOP_WAREHOUSE | `game:Server/OnSever.cpp` | 18565 | arquivo:MONSTER_SPAWN; arquivo:NPC_SHOP_WAREHOUSE; contém U+FFFD; fora de Language |
| 155 | ITEM,MONSTER_SPAWN,PATH_ASSET | `game:fileread.cpp` | 4742 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 140 | ITEM,MONSTER_SPAWN,NPC_SHOP_WAREHOUSE | `game:Server/OnSever.cpp` | 24958 | arquivo:MONSTER_SPAWN; arquivo:NPC_SHOP_WAREHOUSE; contém U+FFFD; fora de Language |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 651 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 656 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 674 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 679 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 707 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 711 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 719 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 723 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 730 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 734 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 742 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 746 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1174 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1182 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1189 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1197 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1202 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1210 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1222 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1236 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1243 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1251 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1259 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1260 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1266 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1268 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1274 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1278 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1279 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1286 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1287 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1297 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1299 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1307 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1313 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1317 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1320 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1328 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1337 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1340 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1345 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1348 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1355 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1361 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1363 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1371 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1379 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1380 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1389 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1390 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1397 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1402 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1404 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1410 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1416 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1423 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1425 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1431 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1442 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1444 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1451 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1459 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1466 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1475 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1478 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1486 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1487 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1493 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1500 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1506 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1512 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1520 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1533 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1537 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1546 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1548 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1559 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1562 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1574 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1574 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1582 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1588 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1595 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1599 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1608 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1613 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1621 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1626 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1636 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1639 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1650 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1653 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1661 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1667 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1675 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `server:fileread.cpp` | 1680 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |
| 125 | ITEM,PATH_ASSET | `game:fileread.cpp` | 1688 | arquivo:ITEM; contém U+FFFD; fora de Language; literal em estrutura/tabela numérica; lookup/comparação:strcmp |

## Regra de uso

1. Começar por LOGIC + HIGH recovery.
2. Priorizar NPC/SHOP/WAREHOUSE e ITEM.
3. Depois MONSTER/SPAWN e PATH/ASSET.
4. Recuperar a semântica original da fonte; não copiar cegamente o idioma para a lógica.
5. Procurar ID/código ASCII equivalente antes de criar o fix.
