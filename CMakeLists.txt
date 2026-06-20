cmake_minimum_required(VERSION 3.20)
project(nanoparser)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(SOURCES
        src/replay.cpp
        src/itch.hpp
        src/itch.cpp
        src/moldudp64.hpp
        src/moldudp64.cpp
        src/networking/null_tx_udp_socket.hpp
        src/networking/null_tx_udp_socket.cpp
        src/reader_checkpoint.hpp
        src/reader_stream.hpp
        src/reader_stream.cpp
        src/replayer.hpp
        src/time.hpp
        src/time.cpp
        src/reader_mmap.hpp
        src/reader_mmap.cpp
        src/read_result.hpp
        src/constants.hpp
        src/networking/udp_socket.cpp
        src/networking/udp_socket.hpp
        src/networking/tx_udp_socket.cpp
        src/networking/tx_udp_socket.hpp
        src/networking/message_header.hpp
        src/networking/cmsg_handlers.cpp
        src/networking/cmsg_handlers.hpp
        src/networking/poller.cpp
        src/networking/poller.hpp
)

add_executable(replay ${SOURCES})
add_executable(receive
        src/receive.cpp
        src/itch.hpp
        src/itch.cpp
        src/moldudp64.hpp
        src/moldudp64.cpp
        src/networking/null_tx_udp_socket.hpp
        src/networking/null_tx_udp_socket.cpp
        src/time.hpp
        src/time.cpp
        src/constants.hpp
        src/networking/rx_udp_socket.cpp
        src/networking/rx_udp_socket.hpp
        src/networking/udp_socket.cpp
        src/networking/udp_socket.hpp
        src/receiver.cpp
        src/receiver.hpp
        src/networking/message_header.hpp
        src/networking/cmsg_handlers.cpp
        src/networking/cmsg_handlers.hpp)

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
    target_compile_options(replay PRIVATE -O0 -g -fsanitize=thread)
    target_link_options(replay PRIVATE -fsanitize=thread)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(replay PRIVATE -O3 -march=native)
endif()