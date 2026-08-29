DECLARE @SchemaName sysname = 'dbo';

SELECT
    s.name AS schema_name,
    t.name AS table_name,

    'CREATE TABLE ' + QUOTENAME(s.name) + '.' + QUOTENAME(t.name) +
    CHAR(13) + CHAR(10) + '(' +
    CHAR(13) + CHAR(10) +

    STUFF((
        SELECT
            ',' + CHAR(13) + CHAR(10) +
            '    ' + QUOTENAME(c.name) + ' ' +

            CASE
                WHEN ty.name IN ('varchar', 'char', 'varbinary', 'binary')
                    THEN ty.name + '(' +
                        CASE
                            WHEN c.max_length = -1 THEN 'MAX'
                            ELSE CAST(c.max_length AS varchar(10))
                        END + ')'

                WHEN ty.name IN ('nvarchar', 'nchar')
                    THEN ty.name + '(' +
                        CASE
                            WHEN c.max_length = -1 THEN 'MAX'
                            ELSE CAST(c.max_length / 2 AS varchar(10))
                        END + ')'

                WHEN ty.name IN ('decimal', 'numeric')
                    THEN ty.name + '(' +
                        CAST(c.precision AS varchar(10)) + ',' +
                        CAST(c.scale AS varchar(10)) + ')'

                WHEN ty.name IN ('datetime2', 'datetimeoffset', 'time')
                    THEN ty.name + '(' +
                        CAST(c.scale AS varchar(10)) + ')'

                ELSE ty.name
            END +

            CASE
                WHEN ic.column_id IS NOT NULL
                    THEN ' IDENTITY(' +
                        CAST(ic.seed_value AS varchar(30)) + ',' +
                        CAST(ic.increment_value AS varchar(30)) + ')'
                ELSE ''
            END +

            CASE
                WHEN c.is_nullable = 1 THEN ' NULL'
                ELSE ' NOT NULL'
            END +

            CASE
                WHEN dc.definition IS NOT NULL
                    THEN ' DEFAULT ' + dc.definition
                ELSE ''
            END

        FROM sys.columns c
        JOIN sys.types ty
            ON c.user_type_id = ty.user_type_id

        LEFT JOIN sys.identity_columns ic
            ON ic.object_id = c.object_id
            AND ic.column_id = c.column_id

        LEFT JOIN sys.default_constraints dc
            ON dc.parent_object_id = c.object_id
            AND dc.parent_column_id = c.column_id

        WHERE c.object_id = t.object_id

        ORDER BY c.column_id

        FOR XML PATH(''), TYPE
    ).value('.', 'nvarchar(max)'), 1, 3, '') +

    CHAR(13) + CHAR(10) +
    ');' AS ddl

FROM sys.tables t
JOIN sys.schemas s
    ON t.schema_id = s.schema_id

WHERE s.name = @SchemaName

ORDER BY t.name;
