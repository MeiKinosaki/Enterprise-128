cmake_minimum_required(VERSION 3.16)
project(Enterprise128_360 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(enterprise128_core src/cpu/z80.cpp src/machine/memory.cpp)
target_include_directories(enterprise128_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(enterprise128_scaffold src/main.cpp)
target_link_libraries(enterprise128_scaffold PRIVATE enterprise128_core)

add_executable(z80_tests tests/z80_tests.cpp)
target_link_libraries(z80_tests PRIVATE enterprise128_core)
add_executable(memory_tests tests/memory_tests.cpp)
target_link_libraries(memory_tests PRIVATE enterprise128_core)

enable_testing()
add_test(NAME z80_basic_tests COMMAND z80_tests)
add_test(NAME memory_basic_tests COMMAND memory_tests)

if(MSVC)
  target_compile_options(enterprise128_core PRIVATE /W4)
  target_compile_options(enterprise128_scaffold PRIVATE /W4)
  target_compile_options(z80_tests PRIVATE /W4)
  target_compile_options(memory_tests PRIVATE /W4)
else()
  target_compile_options(enterprise128_core PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(enterprise128_scaffold PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(z80_tests PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(memory_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()
