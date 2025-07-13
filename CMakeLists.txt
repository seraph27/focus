cmake_minimum_required(VERSION 3.16)
project(Focus VERSION 0.1 LANGUAGES CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#locate libpqxx
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)

add_library(focuslib src/tasks.cpp)
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
  tests/catch_amalgamated.cpp
)
target_link_libraries(task_tests PRIVATE focuslib)
target_include_directories(task_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME task_tests COMMAND task_tests)
