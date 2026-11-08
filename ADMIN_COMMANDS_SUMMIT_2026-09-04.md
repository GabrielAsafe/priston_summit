/level # Summit Tale — inventário de comandos administrativos

Fonte auditada: parser `RecvCommand()` em `SrcServer/src/core/server/legacy_runtime/OnSever.cpp`, mais os comandos de desenvolvimento Tier 5 adicionados durante esta sessão.

> Aviso: este é um inventário de código, não uma certificação de runtime. Vários comandos são legados e alguns são destrutivos. Testar em ambiente de desenvolvimento.

## Autenticação / desenvolvimento

| Comando | Nível | Função |
|---|---:|---|
| `/ativar <senha>` | público, valida credencial GM | ativa/desativa AdminMode usando `GameMasters` |
| `/level <nível>` | Moderator+ no cliente | altera Level/EXP para teste; requer relog para reconstruir pontos |
| `/Levelup <nível>` | Moderator+ no cliente | alias/fluxo legado equivalente |
| `/skiptier` | Moderator+ | conclui 10010–10013 para teste e libera Rank 5 |
| `/tier5test` | Moderator+ | alias de `/skiptier` |

## Moderator — security >= 2

| Comando | Função resumida |
|---|---|
| `/reskill` | recarrega configs de skills e redistribui dados aos jogadores |
| `/caravan` | ativa/adiciona Caravan por 3 horas |
| `/get_guid` | mostra GUID/GpCode/PCR/GpType da sessão |
| `/get_event_level` | mostra nível de evento do jogador |
| `/dm_info` | mostra limites de damage/critical da sessão |
| `/hide` | ativa HideMode |
| `/show` | desativa HideMode |
| `/user` | mostra contagem de usuários/players ativos |
| `/near <char>` | teleporta o GM para o jogador |
| `/call <char>` | teleporta o jogador até o GM |
| `/usercode <code>` | consulta usuário por código interno |
| `/username <char>` | consulta conta/IP/tempo por nome de personagem |
| `/userid <account>` | consulta personagem/IP/tempo por account ID |
| `/sod_view` | consulta score/estado SOD em tempo real |
| `/sod_enter <param>` | força entrada/param de evento SOD/HardCore |
| `/localtime` | mostra hora local do servidor |
| `/get_forceorb` | mostra estado do Force Orb atual |
| `/get_primitem` | mostra timers do Prime Item |
| `/get_currency` | mostra acumuladores econômicos do servidor |
| `/spec <job>` | ativa job/spec temporário por ~10 minutos |
| `/itemdaysrevision <dias>` | ajusta temporariamente revisão de dias de item (-30..30) |
| `/get_potion [char]` | mostra contadores de poções server-side |

## Game Master — security >= 3

| Comando | Função resumida |
|---|---|
| `/get_money` | mostra money server-side da sessão |
| `/show_blackuser [id]` | consulta blacklist |
| `/reset_blackuser` | reseta blacklist |
| `/free_blackuser <id>` | remove ID da blacklist |
| `/area_monster` | mostra estatísticas de spawn/monstros da área |
| `/agefreeOn` | liga evento de aging free |
| `/agefreeOff` | desliga evento de aging free |
| `/modomanu` | alterna modo manutenção; expulsa Player/VIP quando ativa |
| `/GlobalOFF` | desliga chat global |
| `/GlobalON` | liga chat global |
| `/monster <nome> [qtd]` | cria monstro(s) perto do GM; qtd adicional limitada a 20 |
| `/mymonster <nome> [qtd]` | cria monstro(s) associados ao GM |
| `/area_users` | conta usuários na área atual |
| `/arena_monster` | abre monstro da arena de quest legada |
| `/where <char>` | mostra mapa/coordenadas do jogador |
| `/kick <char>` | desconecta jogador |
| `/kill <char>` | envia estado de morte ao jogador |
| `/open_warehouse [char] [modo]` | abre/carrega warehouse, inclusive consulta administrativa |
| `/>mensagem` | broadcast administrativo |
| `/>>mensagem` | notice global via `rsSetNoticeMessage` |

## Administrator — security >= 4

| Comando | Função resumida |
|---|---|
| `/event_monster <nome> <min> <permil>` | configura monstro de evento na área |
| `/start_castle [min]` | inicia Bless Castle e opcionalmente ajusta duração |
| `/end_castle` | termina Bless Castle |
| `/easy_castle` | ativa modo easy do castelo |
| `/set_castle_time <min>` | redefine tempo restante do castelo |
| `/set_castle_master [clanCode]` | define clan master do castelo |
| `/get_castle_info` | mostra estado/tax/master do Bless Castle |
| `/get_fountain_info` | mostra estado da Soul Fountain |
| `/reset_fountain_time` | zera timers da Soul Fountain |
| `/set_castle_tax` | aplica `NextTax` e skill selecionada |
| `/shutdownpk` | inicia shutdown do servidor |
| `/@get <item>` | cria item para o próprio admin |
| `/inv <char> <item>` | cria item para outro jogador |
| `/setdelaychat <seg>` | altera delay do chat global em `Sistemas.ini` |
| `/Bosstime <minuto>` | altera minuto/configuração de boss time |
| `/setlevelpt ...` | altera configuração de diferença/nível de party |
| `/pegarpots ...` | comando legado relacionado a criação/obtenção de poções |
| `/givemoney ...` | comando legado de money |
| `/autopromocao ...` | controla auto-promoção/configuração relacionada |
| `/reload ...` | recarrega configuração/dados administrativos |
| `/rates ...` | consulta/ajusta rates conforme implementação atual |
| `/settimelogado ...` | ajusta tempo logado |
| `/settimelogado_coin ...` | ajusta tempo logado para coin |
| `/setcoinlogado ...` | ajusta coin ligada ao tempo logado |
| `/aging <item> <+nível> <spec>` | cria item aged no chão |
| `/getcoins <char>` | consulta SCoins do jogador |
| `/addcoins <char> <qtd>` | adiciona SCoins |
| `/removecoins <char> <qtd>` | remove SCoins |
| `/quantidade <item> <qtd>` | cria múltiplos itens, máximo 12 |
| `/giveexp <nível>` | ajusta o valor de EXP inicial server-side para o patamar informado |
| `/givegold <qtd>` | cria gold no chão para o admin |
| `/init_premiumitem <id>` | reinicializa determinado premium item ativo |

## Comandos não administrativos no mesmo parser

O mesmo `RecvCommand()` também reconhece comandos de jogador/chat como `/T>`, `//party`, `/disable_whisper`, `/enable_whisper`, `/CLAN>`, `/get_bosstime`, `/get_expgametime`, `/time`, `/cancelar_force`, `/killme` e `/G>`. Eles aparecem antes dos gates de security e não devem ser tratados como comandos GM.
