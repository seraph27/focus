cmake_minimum_required(VERSION 3.16)
project(Focus VERSION 0.1 LANGUAGES CXX)

# Compile with C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define the executable
add_executable(focus src/main.cpp)

# Allow including crow_all.h from the project include/ directory
target_include_directories(focus PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link against pthread (for Crow), libpqxx (PostgreSQL C++ client), and libpq
target_link_libraries(focus
  PRIVATE
    pthread
    pqxx
    pq
)
