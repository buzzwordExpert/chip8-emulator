cmake_minimum_required(VERSION 3.16)

project(chip8-emulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build optimised by default; pass -DCMAKE_BUILD_TYPE=Debug for a debug build
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(SDL3 REQUIRED CONFIG)

add_executable(chip8
    src/main.cpp
    src/chip8.cpp
    src/platform.cpp
)

target_link_libraries(chip8 PRIVATE SDL3::SDL3)

if(MSVC)
    target_compile_options(chip8 PRIVATE /W4)
else()
    target_compile_options(chip8 PRIVATE -Wall -Wextra)
endif()
