#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# Check if PGPASSWORD is set, if not, exit
if [ -z "$PGPASSWORD" ]; then
  echo "Please set the PGPASSWORD environment variable."
  exit 1
fi

# Database connection details
DB_HOST="${DB_HOST:-localhost}"
DB_PORT="${DB_PORT:-5432}"
DB_USER="${DB_USER:-user}"
DB_NAME="${DB_NAME:-focus}"

# Path to the migrations directory
MIGRATIONS_DIR="$(dirname "$0")/../migrations"

# Run migrations
for migration_file in $(ls -v "$MIGRATIONS_DIR"/*.sql); do
  echo "Running migration: $migration_file"
  if command -v psql >/dev/null 2>&1; then
  psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -a -f "$migration_file"
elif command -v docker >/dev/null 2>&1; then
  docker compose exec -T db psql -U "$DB_USER" -d "$DB_NAME" -a -f "/app/migrations/$(basename $migration_file)"
else
  echo "Error: psql and docker not found. Cannot run migrations." >&2
  exit 1
fi
done

echo "Migrations completed successfully."
