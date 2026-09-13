#!/usr/bin/env bash
set -Eeuo pipefail

BASE="$HOME/Transferências"
STAMP="$(date +%Y%m%d_%H%M%S)"
OUT="$BASE/summit_sqlserver_full_export_$STAMP"
ARCHIVE="${OUT}.tar.gz"
TABLES="$OUT/tables.tsv"
EXPORTS="$OUT/exports.tsv"
ERRORS="$OUT/errors.log"

die() {
    echo "[ERRO] $*" >&2
    exit 1
}

ok() {
    echo "[OK] $*"
}

command -v docker >/dev/null 2>&1 || die "docker não encontrado"

mkdir -p "$OUT/data"
: > "$ERRORS"

echo "===================================================================="
echo "R25 - EXPORT COMPLETO SQL SERVER DOCKER (READ ONLY)"
echo "===================================================================="
echo

if [[ -n "${MSSQL_CONTAINER:-}" ]]; then
    CTR="$MSSQL_CONTAINER"
    docker inspect "$CTR" >/dev/null 2>&1 \
        || die "MSSQL_CONTAINER não encontrado: $CTR"
else
    mapfile -t candidates < <(
        docker ps --format '{{.ID}}|{{.Image}}|{{.Names}}' |
        awk -F'|' '
            BEGIN { IGNORECASE=1 }
            $2 ~ /mssql|sqlserver|sql-server/ { print $1 }
        '
    )

    if (( ${#candidates[@]} == 0 )); then
        die "nenhum container SQL Server em execução foi detectado"
    elif (( ${#candidates[@]} > 1 )); then
        echo "[ERRO] mais de um container SQL Server foi detectado:" >&2
        docker ps --format '  {{.ID}}  {{.Image}}  {{.Names}}' |
            grep -Ei 'mssql|sqlserver|sql-server' >&2 || true
        echo "Rode novamente com MSSQL_CONTAINER=<nome-ou-id>." >&2
        exit 2
    fi

    CTR="${candidates[0]}"
fi

CTR_NAME="$(docker inspect -f '{{.Name}}' "$CTR" | sed 's#^/##')"
CTR_IMAGE="$(docker inspect -f '{{.Config.Image}}' "$CTR")"

echo "Container: $CTR_NAME"
echo "Imagem   : $CTR_IMAGE"
echo "Destino  : $OUT"
echo

sqlcmd_tsv() {
    docker exec -i "$CTR" sh -lc '
        set -eu

        if [ -x /opt/mssql-tools18/bin/sqlcmd ]; then
            SQLCMD=/opt/mssql-tools18/bin/sqlcmd
        elif [ -x /opt/mssql-tools/bin/sqlcmd ]; then
            SQLCMD=/opt/mssql-tools/bin/sqlcmd
        elif command -v sqlcmd >/dev/null 2>&1; then
            SQLCMD="$(command -v sqlcmd)"
        else
            echo "[ERRO] sqlcmd não encontrado dentro do container" >&2
            exit 90
        fi

        PASS="${MSSQL_SA_PASSWORD:-${SA_PASSWORD:-}}"
        if [ -z "$PASS" ]; then
            echo "[ERRO] MSSQL_SA_PASSWORD/SA_PASSWORD não existe no container" >&2
            exit 91
        fi

        export SQLCMDPASSWORD="$PASS"

        exec "$SQLCMD" \
            -S localhost \
            -U sa \
            -C \
            -b \
            -h-1 \
            -W \
            -s "|" \
            -w 65535
    '
}

sqlcmd_json() {
    docker exec -i "$CTR" sh -lc '
        set -eu

        if [ -x /opt/mssql-tools18/bin/sqlcmd ]; then
            SQLCMD=/opt/mssql-tools18/bin/sqlcmd
        elif [ -x /opt/mssql-tools/bin/sqlcmd ]; then
            SQLCMD=/opt/mssql-tools/bin/sqlcmd
        elif command -v sqlcmd >/dev/null 2>&1; then
            SQLCMD="$(command -v sqlcmd)"
        else
            echo "[ERRO] sqlcmd não encontrado dentro do container" >&2
            exit 90
        fi

        PASS="${MSSQL_SA_PASSWORD:-${SA_PASSWORD:-}}"
        if [ -z "$PASS" ]; then
            echo "[ERRO] MSSQL_SA_PASSWORD/SA_PASSWORD não existe no container" >&2
            exit 91
        fi

        export SQLCMDPASSWORD="$PASS"

        exec "$SQLCMD" \
            -S localhost \
            -U sa \
            -C \
            -b \
            -h-1 \
            -y 0 \
            -w 65535
    '
}

printf '%s\n' \
    "SET NOCOUNT ON;" \
    "SELECT CAST(SERVERPROPERTY('ProductVersion') AS nvarchar(128));" \
    "GO" |
    sqlcmd_tsv > "$OUT/sqlserver_version.txt"

ok "conexão estabelecida via sqlcmd"

cat <<'SQL' | sqlcmd_tsv > "$OUT/databases.tsv"
SET NOCOUNT ON;

SELECT
    name,
    state_desc,
    recovery_model_desc,
    compatibility_level,
    collation_name
FROM sys.databases
WHERE database_id > 4
  AND state_desc = 'ONLINE'
ORDER BY name;
GO
SQL

DB_COUNT="$(grep -cve '^[[:space:]]*$' "$OUT/databases.tsv" || true)"
echo "Bancos de usuário online: $DB_COUNT"

cat <<'SQL' | sqlcmd_tsv > "$TABLES"
SET NOCOUNT ON;

CREATE TABLE #all_tables
(
    database_name nvarchar(128) NOT NULL,
    schema_name   nvarchar(128) NOT NULL,
    table_name    nvarchar(128) NOT NULL,
    q_database    nvarchar(258) NOT NULL,
    q_schema      nvarchar(258) NOT NULL,
    q_table       nvarchar(258) NOT NULL,
    row_count     bigint        NOT NULL
);

DECLARE @db sysname;
DECLARE @sql nvarchar(max);

DECLARE db_cursor CURSOR LOCAL FAST_FORWARD FOR
SELECT name
FROM sys.databases
WHERE database_id > 4
  AND state_desc = 'ONLINE'
ORDER BY name;

OPEN db_cursor;
FETCH NEXT FROM db_cursor INTO @db;

WHILE @@FETCH_STATUS = 0
BEGIN
    SET @sql = N'
        INSERT INTO #all_tables
        (
            database_name,
            schema_name,
            table_name,
            q_database,
            q_schema,
            q_table,
            row_count
        )
        SELECT
            N''' + REPLACE(@db,'''','''''') + N''',
            s.name,
            t.name,
            N''' + REPLACE(QUOTENAME(@db),'''','''''') + N''',
            QUOTENAME(s.name),
            QUOTENAME(t.name),
            SUM(CASE WHEN p.index_id IN (0,1) THEN p.rows ELSE 0 END)
        FROM ' + QUOTENAME(@db) + N'.sys.tables AS t
        INNER JOIN ' + QUOTENAME(@db) + N'.sys.schemas AS s
            ON s.schema_id = t.schema_id
        LEFT JOIN ' + QUOTENAME(@db) + N'.sys.partitions AS p
            ON p.object_id = t.object_id
        WHERE t.is_ms_shipped = 0
        GROUP BY s.name, t.name;
    ';

    EXEC sys.sp_executesql @sql;

    FETCH NEXT FROM db_cursor INTO @db;
END

CLOSE db_cursor;
DEALLOCATE db_cursor;

SELECT
    database_name,
    schema_name,
    table_name,
    q_database,
    q_schema,
    q_table,
    row_count
FROM #all_tables
ORDER BY database_name, schema_name, table_name;
GO
SQL

TABLE_COUNT="$(grep -cve '^[[:space:]]*$' "$TABLES" || true)"
echo "Tabelas encontradas: $TABLE_COUNT"

cat <<'SQL' | sqlcmd_tsv > "$OUT/columns.tsv"
SET NOCOUNT ON;

CREATE TABLE #cols
(
    database_name  nvarchar(128),
    schema_name    nvarchar(128),
    table_name     nvarchar(128),
    ordinal        int,
    column_name    nvarchar(128),
    type_name      nvarchar(128),
    max_length     int,
    precision_val  int,
    scale_val      int,
    is_nullable    bit,
    is_identity    bit,
    is_computed    bit,
    collation_name nvarchar(128),
    default_text   nvarchar(max)
);

DECLARE @db sysname;
DECLARE @sql nvarchar(max);

DECLARE db_cursor CURSOR LOCAL FAST_FORWARD FOR
SELECT name
FROM sys.databases
WHERE database_id > 4
  AND state_desc = 'ONLINE'
ORDER BY name;

OPEN db_cursor;
FETCH NEXT FROM db_cursor INTO @db;

WHILE @@FETCH_STATUS = 0
BEGIN
    SET @sql = N'
        INSERT INTO #cols
        SELECT
            N''' + REPLACE(@db,'''','''''') + N''',
            s.name,
            t.name,
            c.column_id,
            c.name,
            ty.name,
            c.max_length,
            c.precision,
            c.scale,
            c.is_nullable,
            c.is_identity,
            c.is_computed,
            c.collation_name,
            dc.definition
        FROM ' + QUOTENAME(@db) + N'.sys.tables t
        INNER JOIN ' + QUOTENAME(@db) + N'.sys.schemas s
            ON s.schema_id = t.schema_id
        INNER JOIN ' + QUOTENAME(@db) + N'.sys.columns c
            ON c.object_id = t.object_id
        INNER JOIN ' + QUOTENAME(@db) + N'.sys.types ty
            ON ty.user_type_id = c.user_type_id
        LEFT JOIN ' + QUOTENAME(@db) + N'.sys.default_constraints dc
            ON dc.object_id = c.default_object_id
        WHERE t.is_ms_shipped = 0;
    ';

    EXEC sys.sp_executesql @sql;
    FETCH NEXT FROM db_cursor INTO @db;
END

CLOSE db_cursor;
DEALLOCATE db_cursor;

SELECT
    database_name,
    schema_name,
    table_name,
    ordinal,
    column_name,
    type_name,
    max_length,
    precision_val,
    scale_val,
    is_nullable,
    is_identity,
    is_computed,
    ISNULL(collation_name, N''),
    ISNULL(REPLACE(REPLACE(default_text, CHAR(13), N' '), CHAR(10), N' '), N'')
FROM #cols
ORDER BY database_name, schema_name, table_name, ordinal;
GO
SQL

cat <<'SQL' | sqlcmd_tsv > "$OUT/indexes.tsv"
SET NOCOUNT ON;

CREATE TABLE #idx
(
    database_name nvarchar(128),
    schema_name   nvarchar(128),
    table_name    nvarchar(128),
    index_name    nvarchar(128),
    index_type    nvarchar(128),
    is_unique     bit,
    is_primary_key bit,
    is_unique_constraint bit,
    key_ordinal   int,
    column_name   nvarchar(128),
    is_descending bit
);

DECLARE @db sysname;
DECLARE @sql nvarchar(max);

DECLARE db_cursor CURSOR LOCAL FAST_FORWARD FOR
SELECT name
FROM sys.databases
WHERE database_id > 4
  AND state_desc = 'ONLINE'
ORDER BY name;

OPEN db_cursor;
FETCH NEXT FROM db_cursor INTO @db;

WHILE @@FETCH_STATUS = 0
BEGIN
    SET @sql = N'
        INSERT INTO #idx
        SELECT
            N''' + REPLACE(@db,'''','''''') + N''',
            s.name,
            t.name,
            i.name,
            i.type_desc,
            i.is_unique,
            i.is_primary_key,
            i.is_unique_constraint,
            ic.key_ordinal,
            c.name,
            ic.is_descending_key
        FROM ' + QUOTENAME(@db) + N'.sys.tables t
        INNER JOIN ' + QUOTENAME(@db) + N'.sys.schemas s
            ON s.schema_id = t.schema_id
        INNER JOIN ' + QUOTENAME(@db) + N'.sys.indexes i
            ON i.object_id = t.object_id
        INNER JOIN ' + QUOTENAME(@db) + N'.sys.index_columns ic
            ON ic.object_id = i.object_id
           AND ic.index_id = i.index_id
        INNER JOIN ' + QUOTENAME(@db) + N'.sys.columns c
            ON c.object_id = ic.object_id
           AND c.column_id = ic.column_id
        WHERE t.is_ms_shipped = 0
          AND i.name IS NOT NULL;
    ';

    EXEC sys.sp_executesql @sql;
    FETCH NEXT FROM db_cursor INTO @db;
END

CLOSE db_cursor;
DEALLOCATE db_cursor;

SELECT *
FROM #idx
ORDER BY database_name, schema_name, table_name, index_name, key_ordinal;
GO
SQL

printf '%s\n' \
    "database|schema|table|rows_manifest|json_file|bytes|status" \
    > "$EXPORTS"

FAIL=0
N=0

while IFS='|' read -r db schema table qdb qschema qtable rows; do
    [[ -n "${db//[[:space:]]/}" ]] || continue

    db="$(printf '%s' "$db" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')"
    schema="$(printf '%s' "$schema" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')"
    table="$(printf '%s' "$table" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')"
    qdb="$(printf '%s' "$qdb" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')"
    qschema="$(printf '%s' "$qschema" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')"
    qtable="$(printf '%s' "$qtable" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')"
    rows="$(printf '%s' "$rows" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')"

    N=$((N+1))

    safe_db="$(printf '%s' "$db" | sed 's#[^A-Za-z0-9._-]#_#g')"
    safe_schema="$(printf '%s' "$schema" | sed 's#[^A-Za-z0-9._-]#_#g')"
    safe_table="$(printf '%s' "$table" | sed 's#[^A-Za-z0-9._-]#_#g')"

    mkdir -p "$OUT/data/$safe_db"

    rel="data/$safe_db/$(printf '%04d' "$N")__${safe_schema}__${safe_table}.json"
    dest="$OUT/$rel"
    tmp="${dest}.tmp"

    printf '[%d/%d] %s.%s.%s rows=%s ... ' \
        "$N" "$TABLE_COUNT" "$db" "$schema" "$table" "$rows"

    if {
        cat <<SQL
SET NOCOUNT ON;
SELECT *
FROM ${qdb}.${qschema}.${qtable}
FOR JSON PATH, INCLUDE_NULL_VALUES;
GO
SQL
    } | sqlcmd_json 2>>"$ERRORS" | tr -d '\r\n' > "$tmp"; then

        if [[ ! -s "$tmp" ]]; then
            printf '[]' > "$tmp"
        fi

        if python3 - "$tmp" <<'PY' >/dev/null 2>>"$ERRORS"
import json
import sys

with open(sys.argv[1], "r", encoding="utf-8-sig") as f:
    json.load(f)
PY
        then
            mv "$tmp" "$dest"
            bytes="$(stat -c '%s' "$dest")"
            echo "OK (${bytes} bytes)"
            printf '%s|%s|%s|%s|%s|%s|OK\n' \
                "$db" "$schema" "$table" "$rows" "$rel" "$bytes" \
                >> "$EXPORTS"
        else
            echo "FALHA JSON"
            mv "$tmp" "${dest}.invalid" 2>/dev/null || true
            printf '%s|%s|%s|%s|%s|0|INVALID_JSON\n' \
                "$db" "$schema" "$table" "$rows" "${rel}.invalid" \
                >> "$EXPORTS"
            FAIL=1
        fi
    else
        echo "FALHA SQL"
        rm -f "$tmp"
        printf '%s|%s|%s|%s|%s|0|SQL_ERROR\n' \
            "$db" "$schema" "$table" "$rows" "$rel" \
            >> "$EXPORTS"
        FAIL=1
    fi
done < "$TABLES"

echo
echo "== Resumo =="

OK_COUNT="$(grep -c '|OK$' "$EXPORTS" || true)"
BAD_COUNT="$(grep -Ec '\|(INVALID_JSON|SQL_ERROR)$' "$EXPORTS" || true)"

echo "Bancos : $DB_COUNT"
echo "Tabelas: $TABLE_COUNT"
echo "OK     : $OK_COUNT"
echo "Falhas : $BAD_COUNT"

du -sh "$OUT" || true

cat > "$OUT/README.txt" <<EOF
Summit SQL Server full export
=============================

Gerado em: $(date --iso-8601=seconds)
Container: $CTR_NAME
Imagem: $CTR_IMAGE

Modo: READ ONLY

Arquivos:
- databases.tsv
- tables.tsv
- columns.tsv
- indexes.tsv
- exports.tsv
- errors.log
- data/<database>/*.json

Cada JSON corresponde a uma tabela inteira e usa INCLUDE_NULL_VALUES.

Bancos de sistema não exportados:
- master
- model
- msdb
- tempdb
EOF

if [[ "$FAIL" -eq 0 ]]; then
    tar -czf "$ARCHIVE" -C "$BASE" "$(basename "$OUT")"

    echo
    echo "===================================================================="
    echo "EXPORT COMPLETO: PASS"
    echo "===================================================================="
    echo "Diretório: $OUT"
    echo "Arquivo  : $ARCHIVE"
    sha256sum "$ARCHIVE"
else
    echo
    echo "===================================================================="
    echo "EXPORT COMPLETO: PARCIAL"
    echo "===================================================================="
    echo "Diretório: $OUT"
    echo "Verifique: $ERRORS"
    exit 3
fi
