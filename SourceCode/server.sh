#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
cd "$SCRIPT_DIR"

# R26_SQLITE_SQL_VALIDATION_GATE
python3 "$SCRIPT_DIR/validate_sqlite_runtime_sql.py"
if [ -f "$SCRIPT_DIR/validate_sqlite_query_coverage.py" ]; then
    python3 "$SCRIPT_DIR/validate_sqlite_query_coverage.py"
fi

bash build_server_mingw_i686_j8.sh
bash link_server_mingw_i686.sh
sh send_to_s.sh
