cmake_minimum_required(VERSION 3.16)
project(Focus VERSION 0.1 LANGUAGES CXX)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include our own headers (crow_all.h, etc.)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find libpqxx via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

# Define the executable
add_executable(focus src/main.cpp)

# Add libpqxx include dirs
target_include_directories(focus PRIVATE ${PQXX_INCLUDE_DIRS})

# Link against libpqxx (and its dependent libpq) plus pthread for Crow
target_link_libraries(focus
  PRIVATE
    ${PQXX_LIBRARIES}
    pthread
)
