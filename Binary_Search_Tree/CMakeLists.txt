#The root CMakeLists.txt file.
cmake_minimum_required(VERSION 3.28)

#The project name is set here. In this case it's Rocket, but you can change it to whatever you want.
project(Rocket  VERSION 1.0.0 LANGUAGES CXX)

#Require C++23
set(CXX_STANDARD_REQUIRED ON) #Make C++23 a hard requirement
set(CMAKE_CXX_STANDARD 23) # Default C++ standard for targets
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME}
    main.cpp
    binarytree.cpp
    binarytree.h
    binarysearchtree.cpp 
    binarysearchtree.h 
)

# Optional: useful compile flags for warnings (adjust to taste)
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
