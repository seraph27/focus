cmake_minimum_required(VERSION 3.16)
project(Focus VERSION 0.1 LANGUAGES CXX)

enable_testing()
include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#locate libpqxx
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)

# ─────────────────────────────────────────────────────────────────────────────
# Vendor Catch2 v3.8.1 via FetchContent
# ─────────────────────────────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.8.1
)
#------------------------------------------------------------------------------


FetchContent_MakeAvailable(Catch2)
add_library(focuslib src/tasks.cpp src/auth.cpp src/middleware.cpp)
target_include_directories(focuslib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(focuslib PUBLIC
  PkgConfig::PQXX
  pthread
)

add_executable(focus src/main.cpp)
target_link_libraries(focus PRIVATE focuslib)
# locatino of crow_all.h (and other headers)
target_include_directories(focus PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(task_tests
  tests/task_tests.cpp
)
target_link_libraries(task_tests PRIVATE focuslib Catch2::Catch2WithMain)
target_include_directories(task_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME task_tests COMMAND task_tests)
set_tests_properties(task_tests PROPERTIES
  ENVIRONMENT
    "DB_HOST=127.0.0.1;DB_PORT=5432;DB_USER=user;DB_PASS=pass;DB_NAME=focus"
)
