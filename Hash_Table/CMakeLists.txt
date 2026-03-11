cmake_minimum_required(VERSION 3.28)

# 1. REMOVED: hardcoded /usr/bin/clang++ path. 
# Let CMake find the compiler automatically based on your environment.

project(Rocket VERSION 1.0.0 LANGUAGES CXX)

# Require C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define the executable and source files
add_executable(${PROJECT_NAME}
    main.cpp
    hashtable.cpp
)

# 2. IMPROVED: Compiler flags for both Windows (MSVC) and Unix (Clang/GCC)
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
)
