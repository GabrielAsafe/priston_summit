# Conhecimento consolidado — R21

R21 introduz a primeira origem local real para o perk histórico de PC-room.
Migration 030 adiciona `LocalAuth__GameplayEntitlement`; o key ativo é
`pcbang_gameplay`. O facade preserva o provider antigo por OR (`Bl_RNo > 0`) e
acrescenta a origem local, sem contaminar billing/reservation.

O grant é lido uma vez após login e guardado num cache por slot fora de
`rsPLAYINFO`, evitando mudança de ABI/layout e evitando queries SQLite em loops de
EXP/drop. O cache é limpo na alocação do slot e nos dois caminhos de disconnect.

Schema atual: v30, 333 tabelas totais = 332 reconstruídas + 1 local R21.
Controlled suite: 29/29 PASS. Upgrade 29->30 preserva a conta de teste.

Ainda obrigatório: Windows compile/link/runtime. R6.1 segue como oracle.
