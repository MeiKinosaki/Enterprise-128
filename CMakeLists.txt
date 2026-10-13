cmake_minimum_required(VERSION 3.16)
project(Enterprise128_360 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(enterprise128_scaffold src/main.cpp)

if(MSVC)
  target_compile_options(enterprise128_scaffold PRIVATE /W4)
else()
  target_compile_options(enterprise128_scaffold PRIVATE -Wall -Wextra -Wpedantic)
endif()
