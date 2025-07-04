cmake_minimum_required(VERSION 3.16)
project(MyFocus VERSION 0.1 LANGUAGES CXX)

# Compile with C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

include_directories(include)
add_executable(a src/main.cpp)