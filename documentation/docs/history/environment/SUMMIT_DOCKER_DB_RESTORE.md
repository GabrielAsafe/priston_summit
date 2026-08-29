cd docker
docker compose up -d
bash db/scripts/restore-all-dbs.sh
bash db/scripts/check-db-tables.sh
