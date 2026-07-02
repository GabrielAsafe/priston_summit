from pathlib import Path
import re
import tarfile
import csv
from collections import defaultdict

ROOT = Path("docker/web/wwwroot - EPIC SIMPLE")
AUDIT = Path("/tmp/summit_php_audit")
SCHEMA_DIR = Path("/tmp/db-schema/db-schema")
OUT = Path("docs/php")
OUT.mkdir(parents=True, exist_ok=True)

# extract schema if needed
schema_tar = Path("db-schema.tar.xz")
if not SCHEMA_DIR.exists() and schema_tar.exists():
    Path("/tmp/db-schema").mkdir(parents=True, exist_ok=True)
    with tarfile.open(schema_tar, "r:xz") as t:
        t.extractall("/tmp/db-schema")

php_files = sorted([p for p in ROOT.rglob("*") if p.suffix.lower() in [".php", ".inc"]])

sql_re = re.compile(
    r"\b(SELECT|INSERT|UPDATE|DELETE)\b[\s\S]{0,700}",
    re.IGNORECASE
)

db_table_patterns = [
    re.compile(r"\[([A-Za-z0-9_]+)\]\.\[dbo\]\.\[([A-Za-z0-9_]+)\]", re.I),
    re.compile(r"\[([A-Za-z0-9_]+)\]\.\[dbo\]\.\[\"?\.\(.*?\)\.\"?([A-Za-z0-9_]+)\]", re.I),
    re.compile(r"\[([A-Za-z0-9_]+)\]\.\[dbo\]\.\[", re.I),
    re.compile(r"\bFROM\s+([A-Za-z0-9_]+)\.dbo\.([A-Za-z0-9_]+)", re.I),
    re.compile(r"\bINTO\s+([A-Za-z0-9_]+)\.dbo\.([A-Za-z0-9_]+)", re.I),
    re.compile(r"\bUPDATE\s+([A-Za-z0-9_]+)\.dbo\.([A-Za-z0-9_]+)", re.I),
    re.compile(r"\bDELETE\s+FROM\s+([A-Za-z0-9_]+)\.dbo\.([A-Za-z0-9_]+)", re.I),
]

known_dbs = [
    "ADM","AccountDB","BillingDb","BillingLogDb","ClanDB","CoinShopDB",
    "GameLogDB","ItemLogDB","LogDB","rPTDB","SoD2DB","SodDB",
    "ServerDB","SecurityDB","QuestDB","StartUserDB","PristonDB"
]

file_map = defaultdict(lambda: defaultdict(set))
queries_by_file = defaultdict(list)
legacy_by_file = defaultdict(list)
routes = []

legacy_patterns = [
    "session_register", "session_unregister", "session_is_registered",
    "mysql_escape_string", "mysql_", "mssql_", "ereg", "eregi",
    "ereg_replace", "eregi_replace", "each(", "odbc_do",
    "&$"
]

for p in php_files:
    rel = str(p.relative_to(ROOT))
    try:
        text = p.read_text(errors="ignore")
    except Exception:
        continue

    for m in re.finditer(r"<form[^>]+>", text, re.I):
        form = m.group(0)
        action = re.search(r'action=["\']?([^"\'>\s]+)', form, re.I)
        method = re.search(r'method=["\']?([^"\'>\s]+)', form, re.I)
        routes.append((rel, method.group(1) if method else "", action.group(1) if action else ""))

    for pat in legacy_patterns:
        if pat in text:
            for ln, line in enumerate(text.splitlines(), 1):
                if pat in line:
                    legacy_by_file[rel].append((ln, pat, line.strip()))

    for m in sql_re.finditer(text):
        q = " ".join(m.group(0).split())
        op = m.group(1).upper()
        queries_by_file[rel].append((op, q[:500]))

        found = False
        for pat in db_table_patterns:
            for dm in pat.finditer(q):
                db = dm.group(1)
                table = dm.group(2) if dm.lastindex and dm.lastindex >= 2 else "<dynamic>"
                file_map[rel][db].add((table, op))
                found = True

        for db in known_dbs:
            if re.search(r"\[" + re.escape(db) + r"\]|\b" + re.escape(db) + r"\b", q, re.I):
                if not found:
                    file_map[rel][db].add(("<unknown/dynamic>", op))

# schema
schema = defaultdict(lambda: defaultdict(list))
if SCHEMA_DIR.exists():
    for f in sorted(SCHEMA_DIR.glob("*_columns.psv")):
        with f.open(errors="ignore") as fh:
            reader = csv.DictReader(fh, delimiter="|")
            for row in reader:
                db = row.get("DatabaseName") or f.name.replace("_columns.psv", "")
                table = row.get("TableName", "")
                col = row.get("ColumnName", "")
                typ = row.get("DataType", "")
                nullable = row.get("IsNullable", "")
                schema[db][table].append((col, typ, nullable))

def write(name, content):
    (OUT / name).write_text(content, encoding="utf-8")

# 00 overview
write("00_overview.md", f"""# Summit Tale — PHP/DB reverse map

## Objetivo

Documentar automaticamente:

- ficheiro PHP/INC;
- queries SQL encontradas;
- base de dados;
- tabela;
- operação SQL;
- problemas PHP legado;
- dependências prováveis do servidor.

## Totais

- Ficheiros PHP/INC analisados: {len(php_files)}
- Ficheiros com queries detectadas: {len(file_map)}
- Ficheiros com padrões legados: {len(legacy_by_file)}
- Bases no schema importado: {len(schema)}

## Estado conhecido

O painel normal `/painel/` já foi validado com a conta `alface`.
O fluxo de login usa `AllPersonalMember` e `<LETRA>GameUser`.
O erro 500 inicial foi causado por sintaxe antiga PHP e cursor ODBC.
""")

# routes
lines = ["# Rotas/forms PHP\n"]
for rel, method, action in routes:
    lines.append(f"- `{rel}` → method `{method}` → action `{action}`")
write("01_routes.md", "\n".join(lines) + "\n")

# db map
lines = ["# Mapa PHP → DB → tabela → operação\n"]
for rel in sorted(file_map):
    lines.append(f"\n## `{rel}`")
    for db in sorted(file_map[rel]):
        lines.append(f"\n### {db}")
        for table, op in sorted(file_map[rel][db]):
            lines.append(f"- `{op}` → `{db}.dbo.{table}`")
write("02_database_map.md", "\n".join(lines) + "\n")

# player/admin split
player = ["# Painel normal `/painel/`\n"]
admin = ["# Painel admin `/padm/`\n"]
for rel in sorted(file_map):
    target = player if rel.startswith("painel/") else admin if rel.startswith("padm/") else None
    if target is None:
        continue
    target.append(f"\n## `{rel}`")
    for db in sorted(file_map[rel]):
        target.append(f"\n### {db}")
        for table, op in sorted(file_map[rel][db]):
            target.append(f"- `{op}` → `{db}.dbo.{table}`")
write("03_player_panel.md", "\n".join(player) + "\n")
write("04_admin_panel.md", "\n".join(admin) + "\n")

# tables
lines = ["# Dicionário de tabelas/colunas\n"]
for db in sorted(schema):
    lines.append(f"\n## {db}")
    for table in sorted(schema[db]):
        lines.append(f"\n### {db}.dbo.{table}")
        for col, typ, nullable in schema[db][table]:
            lines.append(f"- `{col}` `{typ}` nullable={nullable}")
write("05_tables.md", "\n".join(lines) + "\n")

# dependencies
lines = ["# Dependências SQL encontradas\n"]
by_db = defaultdict(lambda: defaultdict(set))
for rel, dbs in file_map.items():
    for db, tables in dbs.items():
        for table, op in tables:
            by_db[db][table].add((rel, op))

for db in sorted(by_db):
    lines.append(f"\n## {db}")
    for table in sorted(by_db[db]):
        lines.append(f"\n### {db}.dbo.{table}")
        for rel, op in sorted(by_db[db][table]):
            lines.append(f"- `{op}` em `{rel}`")
write("06_sql_dependencies.md", "\n".join(lines) + "\n")

# legacy
lines = ["# Problemas PHP legado\n"]
for rel in sorted(legacy_by_file):
    lines.append(f"\n## `{rel}`")
    for ln, pat, line in legacy_by_file[rel][:50]:
        lines.append(f"- linha {ln}: `{pat}` → `{line}`")
write("07_legacy_php_issues.md", "\n".join(lines) + "\n")

# server startup DB check
server_dbs = [
    "AccountDB","BillingDb","BillingLogDb","ClanDB","GameLogDB","ItemLogDB",
    "LogDB","rPTDB","SoD2DB","SodDB","ServerDB","SecurityDB","CoinShopDB",
    "QuestDB","StartUserDB"
]
lines = ["# Checklist DB para arranque do Server.exe\n"]
lines.append("O binário menciona as seguintes bases. Este ficheiro cruza isso com o schema importado.\n")
for db in server_dbs:
    # case-insensitive match
    match = None
    for sdb in schema:
        if sdb.lower() == db.lower():
            match = sdb
            break
    if match:
        lines.append(f"\n## {db} — OK no schema")
        for table in sorted(schema[match]):
            cols = ", ".join(c for c, _, _ in schema[match][table][:8])
            lines.append(f"- `{match}.dbo.{table}` → {cols}")
    else:
        lines.append(f"\n## {db} — NÃO encontrado no schema")
write("08_server_db_startup_check.md", "\n".join(lines) + "\n")

print("OK: documentação gerada em docs/php/")
