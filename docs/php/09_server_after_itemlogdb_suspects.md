# Suspeitos após ItemLogDB

## Conclusão atual

A `ItemLogDB` não parece ser o problema, porque:

- a base existe;
- a tabela `IL` existe;
- as colunas esperadas pelo `sql.dll` existem;
- o servidor consegue conectar até `ItemLogDB`.

O erro após isso provavelmente está numa query de inicialização posterior.

## Bases posteriores encontradas no schema

- LogDB
- rPTDB
- ServerDB
- SecurityDB
- CoinShopDB
- QuestDB
- StartUserDB

## Suspeitos principais

### LogDB

Tabelas:

- AccountLogin
- ChangePassword
- CheatLog
- ClanLogin
- Register

Possível uso:

- login de conta;
- logs de autenticação;
- registo;
- troca de senha;
- anti-cheat.

### rPTDB

Tabela:

- LevelList

Possível uso:

- ranking;
- lista de níveis;
- atualização de ranking.

### ServerDB

Tabelas:

- BoxRewards
- DropLogAlert
- NoticeSystem
- PVP
- PremiumData
- Vip

Possível uso:

- sistema premium;
- VIP;
- avisos;
- recompensas;
- PvP;
- drops especiais.

### SecurityDB

Tabelas:

- AgingFailed
- AttackLog
- CheatLog
- FunctionChecksumList
- Security
- WindowCheatList

Possível uso:

- anti-cheat;
- logs de ataque;
- checksum de funções;
- verificação de janelas proibidas;
- protocolo/IP.

### CoinShopDB

Tabelas:

- ItemList
- PendingPayments
- Transactions
- UserInfo

Possível uso:

- loja de coins;
- saldo de conta;
- pagamentos;
- itens vendáveis.

### QuestDB

Tabelas:

- Complete
- DailyQuest
- ItemRewards
- MonsterKilled
- Progress
- QuestMonsters
- Rewards
- SingleQuest

Possível uso:

- sistema de quests;
- progresso;
- recompensas;
- kills de monstros.

### StartUserDB

Tabelas:

- Items
- Level

Possível uso:

- itens iniciais;
- nível inicial;
- criação de personagem.

## Próximo diagnóstico recomendado

Extrair strings SQL do `Server.exe` e `sql.dll` filtrando por estas bases/tabelas:

```bash
strings -a Servidor/Server.exe | grep -Ei "LogDB|rPTDB|ServerDB|SecurityDB|CoinShopDB|QuestDB|StartUserDB|AccountLogin|LevelList|PremiumData|Vip|Security|Items|DailyQuest"
strings -a Servidor/sql.dll    | grep -Ei "LogDB|rPTDB|ServerDB|SecurityDB|CoinShopDB|QuestDB|StartUserDB|AccountLogin|LevelList|PremiumData|Vip|Security|Items|DailyQuest"
```


E agora roda:

```bash
cd ~/Área\ de\ Trabalho/summit

strings -a Servidor/Server.exe | grep -Ei "LogDB|rPTDB|ServerDB|SecurityDB|CoinShopDB|QuestDB|StartUserDB|AccountLogin|LevelList|PremiumData|Vip|Security|Items|DailyQuest" \
> /tmp/server_db_strings.txt

strings -a Servidor/sql.dll | grep -Ei "LogDB|rPTDB|ServerDB|SecurityDB|CoinShopDB|QuestDB|StartUserDB|AccountLogin|LevelList|PremiumData|Vip|Security|Items|DailyQuest" \
> /tmp/sql_dll_db_strings.txt

echo "===== Server.exe ====="
cat /tmp/server_db_strings.txt

echo
echo "===== sql.dll ====="
cat /tmp/sql_dll_db_strings.txt