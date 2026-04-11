cmake_minimum_required(VERSION 4.0)
project(nanoparser)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(SOURCES
        replay.cpp
        itch.hpp
        itch.cpp
        moldudp64.hpp
        moldudp64.cpp
        networking.hpp
        networking.cpp
        reader.hpp
        reader.cpp
        replayer.hpp
        replayer.cpp
        time.hpp
        time.cpp
)

add_executable(replay ${SOURCES})

target_compile_options(replay PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wshadow
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(replay PRIVATE -O0 -g -fsanitize=address,undefined)
    target_link_options(replay PRIVATE -fsanitize=address,undefined)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(replay PRIVATE -O3 -march=native)
endif()