# Focus

Simple C++ microservice using Crow and PostgreSQL for tracking tasks.

## Features

- **Health check** via `GET /healthz`.
- **List tasks** with `GET /tasks`.
- **Create tasks** using `POST /tasks` with `name`, `start_at` and `end_at` fields.
- **Toggle or set completion** with `PATCH /tasks/<id>`.
- **Delete tasks** using `DELETE /tasks/<id>`.

Database connection parameters can be overridden using the `DB_HOST`, `DB_PORT`, `DB_USER`, `DB_PASS` and `DB_NAME` environment variables. Defaults match the provided `docker-compose.yml` service.

## Building

```bash
cmake -S . -B build
cmake --build build
```

Run the unit tests:

```bash
ctest --test-dir build --output-on-failure
```

## Running locally

Start the service against a local PostgreSQL instance:

```bash
DB_HOST=127.0.0.1 ./build/focus
```

The default host is `db`, which matches the service name when running with `docker-compose`:

```bash
docker compose up --build
```

A smoke test script is provided in `scripts/smoke.sh` to exercise the API.
