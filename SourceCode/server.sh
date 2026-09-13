#!bin/bash

# R26_SQLITE_SQL_VALIDATION_GATE
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
python3 "$SCRIPT_DIR/validate_sqlite_runtime_sql.py" || exit $?
if [ -f "$SCRIPT_DIR/validate_sqlite_query_coverage.py" ]; then
    python3 "$SCRIPT_DIR/validate_sqlite_query_coverage.py" || exit $?
fi


bash build_server_mingw_i686_j8.sh

sh link_server_mingw_i686.sh

sh send_to_s.sh


#cat initandtestServerfile.txt
