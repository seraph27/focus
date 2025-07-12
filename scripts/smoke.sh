#!/usr/bin/env bash
set -e

API=http://localhost:8080

echo "Resetting tasks table…"
docker exec -i focus-db-1 psql -U user -d focus <<SQL
TRUNCATE TABLE tasks RESTART IDENTITY;
SQL

echo "Health check…"
curl -i -f $API/healthz

echo "Create…"
curl -i -f -X POST $API/tasks \
     -H "Content-Type: application/json" \
     -d '{
           "name":"CI Test",
           "start_at":"2025-07-12T00:00:00Z",
           "end_at":"2025-07-12T01:00:00Z"
         }'

echo "List…"
curl -i -f $API/tasks | grep '"id":1'

echo "Patch…"
curl -i -f -X PATCH $API/tasks/1 \
     -H "Content-Type: application/json" \
     -d '{"done":true}'

echo "Verify done…"
curl -i -f $API/tasks | grep '"done":true'

echo "Delete…"
curl -i -f -X DELETE $API/tasks/1

echo "Final list…"
curl -i -f $API/tasks

echo "All tests passed!"
