# ── Build stage ──
FROM ubuntu:22.04 AS builder

# 1. Install system dependencies needed for vcpkg and basic build tools.
#    We remove libpqxx-dev, libssl-dev from here because vcpkg will handle them.
RUN apt-get update && apt-get install -y \
    build-essential cmake git curl zip unzip pkg-config autoconf bison flex ca-certificates libasio-dev \
    && rm -rf /var/lib/apt/lists/*

# 2. Install vcpkg itself.
ENV VCPKG_ROOT /opt/vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git ${VCPKG_ROOT} && \
    ${VCPKG_ROOT}/bootstrap-vcpkg.sh -disableMetrics && \
    ${VCPKG_ROOT}/vcpkg integrate install

# Set environment variable for CMake to find vcpkg toolchain.
ENV CMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake

WORKDIR /app
# 3. Copy your project source code into the builder.
COPY . .

# 4. Use vcpkg to install C++ dependencies required for your application.
#    This is where jwt-cpp, libpqxx, openssl, nlohmann-json are installed/built.
RUN ${VCPKG_ROOT}/vcpkg install jwt-cpp libpqxx --triplet x64-linux

# 5. Configure and Build your C++ application using CMake.
#    CMake will now use the vcpkg toolchain to find libraries.
RUN mkdir build && cd build && \
    cmake .. \
        -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake \
        -DVCPKG_TARGET_TRIPLET=x64-linux \
    && make -j$(nproc)

# ── Final stage ──
FROM ubuntu:22.04

# 1. Install minimal *runtime* system dependencies.
#    Only those not provided by vcpkg's copied files.
RUN apt-get update && apt-get install -y \
    libpq5 \
    libssl3 \
    postgresql-client \
    && rm -rf /var/lib/apt/lists/*

# 2. Copy the compiled application binary from the builder stage.
COPY --from=builder /app/build/focus /usr/local/bin/focus

# 3. Copy vcpkg's installed *runtime* libraries (shared objects like .so files).
COPY --from=builder /opt/vcpkg/installed/x64-linux /usr/local/vcpkg_installed

# 4. Adjust the dynamic linker search path so your app can find the copied libraries.
ENV LD_LIBRARY_PATH=/usr/local/vcpkg_installed/lib:/usr/local/vcpkg_installed/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH

# 5. Copy other non-C++-compiled assets like scripts and migrations.
COPY scripts /app/scripts
COPY migrations /app/migrations

EXPOSE 8080
ENTRYPOINT ["focus"]
