# ── Build stage ──
FROM ubuntu:22.04 AS builder

# Install build deps
RUN apt-get update && apt-get install -y \
    build-essential cmake libpqxx-dev libssl-dev curl libasio-dev

WORKDIR /app
COPY . .

# Build the service
RUN mkdir build && cd build && \
    cmake .. && make -j4

# ── Final stage ──
FROM ubuntu:22.04

# Install runtime deps
RUN apt-get update && apt-get install -y libpq5 libpqxx-6.4

# Copy the compiled binary
COPY --from=builder /app/build/focus /usr/local/bin/focus

EXPOSE 8080
ENTRYPOINT ["focus"]
