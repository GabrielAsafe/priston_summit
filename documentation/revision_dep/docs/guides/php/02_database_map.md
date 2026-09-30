# Mapa PHP → DB → tabela → operação


## `padm/Distribuidor.inc.php`

### ADM
- `INSERT` → `ADM.dbo.<dynamic>`
- `SELECT` → `ADM.dbo.<dynamic>`
- `INSERT` → `ADM.dbo.LogsDistribuidor`
- `SELECT` → `ADM.dbo.loginGM`

## `padm/banir.inc.php`

### ADM
- `UPDATE` → `ADM.dbo.<dynamic>`
- `UPDATE` → `ADM.dbo.LogsBan`

### accountdb
- `SELECT` → `accountdb.dbo.<dynamic>`
- `UPDATE` → `accountdb.dbo.<dynamic>`
- `UPDATE` → `accountdb.dbo.AllGameUser`

## `padm/clan.inc.php`

### ClanDB
- `SELECT` → `ClanDB.dbo.<dynamic>`
- `SELECT` → `ClanDB.dbo.CL`
- `SELECT` → `ClanDB.dbo.UL`

## `padm/contas.inc.php`

### accountdb
- `SELECT` → `accountdb.dbo.<dynamic>`
- `UPDATE` → `accountdb.dbo.<dynamic>`
- `UPDATE` → `accountdb.dbo.AllGameUser`
- `SELECT` → `accountdb.dbo.AllPersonalMember`
- `UPDATE` → `accountdb.dbo.AllPersonalMember`

## `padm/dados.inc.php`

### accountdb
- `SELECT` → `accountdb.dbo.<dynamic>`
- `SELECT` → `accountdb.dbo.AllGameUser`
- `SELECT` → `accountdb.dbo.AllPersonalMember`

## `padm/delclan.inc.php`

### ClanDb
- `SELECT` → `ClanDb.dbo.<dynamic>`
- `SELECT` → `ClanDb.dbo.CL`
- `SELECT` → `ClanDb.dbo.UL`

## `padm/delconta.inc.php`

### accountdb
- `SELECT` → `accountdb.dbo.<dynamic>`
- `SELECT` → `accountdb.dbo.AllGameUser`
- `SELECT` → `accountdb.dbo.AllPersonalMember`

## `padm/deletaitemlog.inc.php`

### ItemLogDB
- `SELECT` → `ItemLogDB.dbo.<unknown/dynamic>`

## `padm/depositar.inc.php`

### accountdb
- `SELECT` → `accountdb.dbo.<dynamic>`
- `UPDATE` → `accountdb.dbo.<dynamic>`
- `SELECT` → `accountdb.dbo.AllGameUser`
- `SELECT` → `accountdb.dbo.AllPersonalMember`
- `UPDATE` → `accountdb.dbo.AllPersonalMember`

## `padm/depositarbc.inc.php`

### ClanDb
- `SELECT` → `ClanDb.dbo.<dynamic>`
- `UPDATE` → `ClanDb.dbo.<dynamic>`
- `SELECT` → `ClanDb.dbo.CL`
- `UPDATE` → `ClanDb.dbo.CL`

### accountdb
- `SELECT` → `accountdb.dbo.<dynamic>`
- `SELECT` → `accountdb.dbo.AllGameUser`

## `padm/depositarsod.inc.php`

### ClanDb
- `SELECT` → `ClanDb.dbo.<dynamic>`
- `UPDATE` → `ClanDb.dbo.<dynamic>`
- `SELECT` → `ClanDb.dbo.CL`
- `UPDATE` → `ClanDb.dbo.CL`

### accountdb
- `SELECT` → `accountdb.dbo.<dynamic>`
- `SELECT` → `accountdb.dbo.AllGameUser`

## `padm/index.php`

### ADM
- `SELECT` → `ADM.dbo.<dynamic>`
- `SELECT` → `ADM.dbo.loginGM`

## `padm/lista_permiados.php`

### ADM
- `SELECT` → `ADM.dbo.<dynamic>`
- `SELECT` → `ADM.dbo.LogsDistribuidor`

## `padm/logCOD.inc.php`

### ITEMLogDB
- `SELECT` → `ITEMLogDB.dbo.<dynamic>`
- `SELECT` → `ITEMLogDB.dbo.IL`

## `padm/logID.inc.php`

### ITEMLogDB
- `SELECT` → `ITEMLogDB.dbo.<dynamic>`
- `SELECT` → `ITEMLogDB.dbo.IL`

## `padm/logIP.inc.php`

### ITEMLogDB
- `SELECT` → `ITEMLogDB.dbo.<dynamic>`
- `SELECT` → `ITEMLogDB.dbo.IL`

## `padm/log_distribuidor.php`

### ADM
- `SELECT` → `ADM.dbo.<dynamic>`
- `SELECT` → `ADM.dbo.LogsDistribuidor`

## `padm/multimanager.inc.php`

### rPTDB
- `DELETE` → `rPTDB.dbo.<dynamic>`
- `INSERT` → `rPTDB.dbo.<dynamic>`
- `DELETE` → `rPTDB.dbo.LevelList`
- `INSERT` → `rPTDB.dbo.LevelList`

## `padm/premiado.php`

### Jeck
- `SELECT` → `Jeck.dbo.<dynamic>`
- `SELECT` → `Jeck.dbo.itens`
- `SELECT` → `Jeck.dbo.premiado`

## `padm/punidos.inc.php`

### ADM
- `SELECT` → `ADM.dbo.<dynamic>`
- `SELECT` → `ADM.dbo.LogsBan`

## `padm/statuscontas.inc.php`

### accountdb
- `DELETE` → `accountdb.dbo.<dynamic>`
- `SELECT` → `accountdb.dbo.<dynamic>`
- `DELETE` → `accountdb.dbo.AllGameUser`
- `SELECT` → `accountdb.dbo.AllGameUser`
- `DELETE` → `accountdb.dbo.AllPersonalMember`

## `padm/uon.php`

### ClanDB
- `SELECT` → `ClanDB.dbo.<dynamic>`
- `SELECT` → `ClanDB.dbo.CT`

## `padm/zerarank.php`

### rPTDB
- `DELETE` → `rPTDB.dbo.<dynamic>`
- `DELETE` → `rPTDB.dbo.LevelList`

## `padm/zerarlogdistribuidor.php`

### ADM
- `DELETE` → `ADM.dbo.<dynamic>`
- `DELETE` → `ADM.dbo.LogsDistribuidor`

## `padm/zerarsod.php`

### ClanDB
- `UPDATE` → `ClanDB.dbo.<dynamic>`
- `UPDATE` → `ClanDB.dbo.CL`

### SoD2Db
- `UPDATE` → `SoD2Db.dbo.<dynamic>`
- `UPDATE` → `SoD2Db.dbo.SOD2RecBySandurr`

## `painel/atualizar.inc.php`

### accountdb
- `SELECT` → `accountdb.dbo.<dynamic>`
- `UPDATE` → `accountdb.dbo.<dynamic>`
- `UPDATE` → `accountdb.dbo.AllGameUser`
- `SELECT` → `accountdb.dbo.AllPersonalMember`
- `UPDATE` → `accountdb.dbo.AllPersonalMember`

## `painel/bau/bau.inc.php`

### ClanDb
- `SELECT` → `ClanDb.dbo.<dynamic>`
- `SELECT` → `ClanDb.dbo.CT`

## `painel/cadastrar.php`

### accountdb
- `INSERT` → `accountdb.dbo.<dynamic>`
- `SELECT` → `accountdb.dbo.<dynamic>`
- `SELECT` → `accountdb.dbo.ALLPersonalMember`
- `INSERT` → `accountdb.dbo.AllGameUser`
- `INSERT` → `accountdb.dbo.AllPersonalMember`

## `painel/clan/admclan.inc.php`

### ArcadePT
- `DELETE` → `ArcadePT.dbo.<dynamic>`
- `DELETE` → `ArcadePT.dbo.ClanDebandar`

### ClanDb
- `DELETE` → `ClanDb.dbo.<dynamic>`
- `SELECT` → `ClanDb.dbo.<dynamic>`
- `UPDATE` → `ClanDb.dbo.<dynamic>`
- `SELECT` → `ClanDb.dbo.CL`
- `UPDATE` → `ClanDb.dbo.CL`
- `DELETE` → `ClanDb.dbo.UL`
- `SELECT` → `ClanDb.dbo.UL`
- `UPDATE` → `ClanDb.dbo.UL`

## `painel/clan/criar_clan.inc.php`

### ClanDb
- `INSERT` → `ClanDb.dbo.<dynamic>`
- `SELECT` → `ClanDb.dbo.<dynamic>`
- `UPDATE` → `ClanDb.dbo.<dynamic>`
- `INSERT` → `ClanDb.dbo.CL`
- `SELECT` → `ClanDb.dbo.CL`
- `SELECT` → `ClanDb.dbo.LI`
- `UPDATE` → `ClanDb.dbo.LI`
- `INSERT` → `ClanDb.dbo.UL`
- `SELECT` → `ClanDb.dbo.UL`

### accountdb
- `SELECT` → `accountdb.dbo.<dynamic>`
- `UPDATE` → `accountdb.dbo.<dynamic>`
- `SELECT` → `accountdb.dbo.AllPersonalMember`
- `UPDATE` → `accountdb.dbo.AllPersonalMember`

## `painel/clan/membrosdoclan.inc.php`

### ClanDB
- `DELETE` → `ClanDB.dbo.<dynamic>`
- `DELETE` → `ClanDB.dbo.CL`
- `DELETE` → `ClanDB.dbo.UL`

### ClanDb
- `SELECT` → `ClanDb.dbo.<dynamic>`
- `SELECT` → `ClanDb.dbo.CL`
- `SELECT` → `ClanDb.dbo.CT`
- `SELECT` → `ClanDb.dbo.UL`

## `painel/index - Copia.php`

### accountdb
- `SELECT` → `accountdb.dbo.<dynamic>`
- `SELECT` → `accountdb.dbo.AllPersonalMember`

## `painel/index.php`

### accountdb
- `SELECT` → `accountdb.dbo.<dynamic>`
- `SELECT` → `accountdb.dbo.AllPersonalMember`

## `painel/index2.php`

### ClanDb
- `SELECT` → `ClanDb.dbo.<dynamic>`
- `SELECT` → `ClanDb.dbo.CT`

### accountdb
- `SELECT` → `accountdb.dbo.<dynamic>`
- `SELECT` → `accountdb.dbo.AllGameUser`

## `painel/info.clan.php`

### clandb
- `SELECT` → `clandb.dbo.<dynamic>`
- `SELECT` → `clandb.dbo.UL`

## `painel/inicial.inc.php`

### ADM
- `SELECT` → `ADM.dbo.<dynamic>`
- `SELECT` → `ADM.dbo.LogsBan`

### ClanDb
- `SELECT` → `ClanDb.dbo.<dynamic>`
- `SELECT` → `ClanDb.dbo.CL`
- `SELECT` → `ClanDb.dbo.UL`

### accountdb
- `SELECT` → `accountdb.dbo.<dynamic>`

### clanDB
- `SELECT` → `clanDB.dbo.<dynamic>`
- `SELECT` → `clanDB.dbo.CL`

## `painel/recsenha.php`

### accountdb
- `SELECT` → `accountdb.dbo.<dynamic>`
- `SELECT` → `accountdb.dbo.ALLPersonalMember`

## `test_login_admin.php`

### ADM
- `SELECT` → `ADM.dbo.<dynamic>`
- `SELECT` → `ADM.dbo.loginGM`
