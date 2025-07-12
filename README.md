# Focus

Simple C++ microservice using Crow and PostgreSQL.

## Running locally

When not using `docker-compose`, set `DB_HOST` so the service knows where
PostgreSQL is running:

```bash
DB_HOST=127.0.0.1 ./build/focus
```

The default host is `db` which matches the service name in the provided
`docker-compose.yml`.
