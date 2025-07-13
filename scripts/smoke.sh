#!/usr/bin/env bash
set -euo pipefail

# Base API endpoint for the running service
API="${API:-http://localhost:8080}"
# Connection string for psql; defaults match docker-compose and CI
DB_CONN="${DB_CONN:-host=127.0.0.1 port=5432 user=user password=pass dbname=focus}"

echo "Resetting tasks table…"

if command -v psql >/dev/null 2>&1; then
  psql "$DB_CONN" <<'SQL'
TRUNCATE TABLE tasks RESTART IDENTITY;
SQL
elif command -v docker >/dev/null 2>&1; then
  docker compose exec -T db psql -U user -d focus <<'SQL'
TRUNCATE TABLE tasks RESTART IDENTITY;
SQL
else
  echo "Warning: psql not found; skipping DB reset" >&2
fi

echo "Health check…"
curl -fsS $API/healthz

echo "Create…"
curl -fsS -X POST $API/tasks \
     -H "Content-Type: application/json" \
     -d '{
           "name":"CI Test",
           "start_at":"2025-07-12T00:00:00Z",
           "end_at":"2025-07-12T01:00:00Z"
         }'

echo "Invalid create…"
status=$(curl -s -o /dev/null -w "%{http_code}" -X POST $API/tasks \
        -H "Content-Type: application/json" \
        -d '{
              "name":"Bad",
              "start_at":"2025-07-12T02:00:00Z",
              "end_at":"2025-07-12T01:00:00Z"
            }')
if [ "$status" != "400" ]; then
  echo "Expected 400, got $status" >&2
  exit 1
fi

echo "List…"
curl -fsS $API/tasks | grep '"id":1'

echo "Patch…"
curl -fsS -X PATCH $API/tasks/1 \
     -H "Content-Type: application/json" \
     -d '{"done":true}'

echo "Verify done…"
curl -fsS $API/tasks | grep '"done":true'

echo "Delete…"
curl -fsS -X DELETE $API/tasks/1

echo "Final list…"
curl -fsS $API/tasks

echo "All tests passed!"
