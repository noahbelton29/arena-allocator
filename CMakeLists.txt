cmake_minimum_required(VERSION 3.16)
project(ArenaAllocator CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(arena_allocator src/ArenaAllocator.cpp)
target_include_directories(arena_allocator PUBLIC include)

add_executable(demo examples/demo.cpp)
target_link_libraries(demo PRIVATE arena_allocator)

add_executable(tests tests/tests.cpp)
target_link_libraries(tests PRIVATE arena_allocator)