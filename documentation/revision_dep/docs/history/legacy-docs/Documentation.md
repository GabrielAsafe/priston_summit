# Summit Tale — Documento de Retomada do Projeto

## Estado Atual

Data: 01/07/2026

O ambiente Docker foi configurado com sucesso.

Serviços ativos:

* SQL Server 2022
* Site principal Summit Tale
* Painel de administração
* Painel web secundário
* CloudBeaver

---

# Credenciais

## SQL Server

Servidor:

```text
summittale-mssql
```

Utilizador:

```text
sa
```

Senha:

```text
SummitTale@12345
```

Porta:

```text
1433
```

---

## CloudBeaver

Utilizador:

```text
summittale
```

Senha:

```text
Admin123
```

URL:

```text
http://localhost:8978
```

Ligação SQL Server:

```text
Host: summittale-mssql
Port: 1433
Database: AccountDB
User: sa
Password: SummitTale@12345
```

Importante:

Não utilizar "localhost" como host dentro do CloudBeaver.

O host correto é:

```text
summittale-mssql
```

---

# URLs

## Painel Administrativo

```text
http://localhost:8089/padm/
```

Resposta:

```text
HTTP/1.1 200 OK
```

---

## Painel Principal

```text
http://localhost:8089/painel/
```

Resposta:

```text
HTTP/1.1 200 OK
```

---

## Website Principal

```text
http://localhost:8088/
```

Resposta:

```text
HTTP/1.1 200 OK
```

---

# Bases de Dados Restauradas

## Contagem de tabelas

| Base           | Tabelas |
| -------------- | ------: |
| AccountDB      |      69 |
| BattlePass     |       3 |
| BillingDb      |      43 |
| BillingLogDb   |       6 |
| ClanDB         |      28 |
| CoinShopDB     |       6 |
| GameLogDB      |      17 |
| ItemLogDB      |       3 |
| Jobs           |       2 |
| LogadoPremiado |       1 |
| LogDB          |       5 |
| PCRoom         |       3 |
| PCRoomLog      |      19 |
| PristonDB      |      19 |
| QuestDB        |       8 |
| rPTDB          |       1 |
| SecurityDB     |       6 |
| ServerDB       |       6 |
| SoD2DB         |      10 |
| SodDb          |      69 |
| StartUserDB    |       2 |

---

# Bases mais importantes

## AccountDB

69 tabelas.

Tabelas principais:

```text
AllGameUser
MK_User_DATA
GameMasters
...
```

Observação:

As tabelas existem mas não contêm utilizadores.

Contagem verificada:

```sql
SELECT COUNT(*) FROM AllGameUser;
SELECT COUNT(*) FROM MK_User_DATA;
```

Resultado:

```text
0 registos
```

---

## PristonDB

19 tabelas.

Contém configurações e sistemas do servidor.

Exemplos:

```text
PremiumData
NoticeSystem
Security
TopLevel
PVP
```

---

## ServerDB

6 tabelas.

Exemplos:

```text
BoxRewards
DropLogAlert
NoticeSystem
PremiumData
PVP
Vip
```

---

# Processo Executado

## 1. Descoberta

Inicialmente as bases existiam mas estavam vazias.

Os ficheiros SQL apenas criavam as databases.

Exemplo:

```sql
CREATE DATABASE AccountDB
```

Não existiam:

```sql
CREATE TABLE
INSERT INTO
```

---

## 2. Identificação dos Backups

Os dados reais encontravam-se nos ficheiros:

```text
@DB BAK/*.bak
```

---

## 3. Validação dos Backups

Comando utilizado:

```bash
docker cp "./@DB BAK/AccountDB202405120913.bak" summittale-mssql:/tmp/AccountDB.bak

docker exec -it summittale-mssql \
/opt/mssql-tools18/bin/sqlcmd \
-S localhost -U sa -P 'SummitTale@12345' -C \
-Q "RESTORE FILELISTONLY FROM DISK='/tmp/AccountDB.bak'"
```

---

## 4. Restore das Bases

Exemplo utilizado:

```sql
RESTORE DATABASE [PristonDB]
FROM DISK = N'/tmp/PristonDB.bak'
WITH REPLACE,
MOVE N'PristonDB' TO N'/var/opt/mssql/data/PristonDB.mdf',
MOVE N'PristonDB_log' TO N'/var/opt/mssql/data/PristonDB_log.ldf';
```

---

## 5. Validação

Comando utilizado:

```bash
for db in AccountDB BattlePass BillingDb BillingLogDb ClanDB CoinShopDB GameLogDB ItemLogDB Jobs LogadoPremiado LogDB PCRoom PCRoomLog PristonDB QuestDB rPTDB SecurityDB ServerDB SoD2DB SodDb StartUserDB; do
  docker exec -i summittale-mssql \
    /opt/mssql-tools18/bin/sqlcmd \
    -S localhost \
    -U sa \
    -P 'SummitTale@12345' \
    -C \
    -d "$db" \
    -Q "SELECT DB_NAME() AS database_name, COUNT(*) AS user_tables FROM sys.tables;"
done
```

---

# Containers

```text
summittale-mssql
summit-web-epic
summit-web-bpt
summittale-dbadmin
portainer
```

---

# Próximos Passos

1. Identificar a estrutura das tabelas de contas.
2. Criar conta administrativa de teste.
3. Validar login no painel.
4. Validar login no cliente.
5. Mapear relações entre:

   * AccountDB
   * PristonDB
   * ServerDB
   * ClanDB
   * CoinShopDB
6. Automatizar restauração das bases.
7. Documentar toda a arquitetura do projeto.

