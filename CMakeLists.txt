cmake_minimum_required(VERSION 3.16)
project(Focus VERSION 0.1 LANGUAGES CXX)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Locate libpqxx via pkg-config and create an imported target
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)

# Define the executable
add_executable(focus src/main.cpp)

# Tell the compiler where to find crow_all.h (and other headers)
target_include_directories(focus PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link against libpqxx (and its dependencies) and pthread for threading
target_link_libraries(focus
  PRIVATE
    PkgConfig::PQXX
    pthread
)
