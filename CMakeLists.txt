cmake_minimum_required(VERSION 3.20)
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
        src/networking/null_tx_udp_socket.hpp
        src/networking/null_tx_udp_socket.cpp
        reader_checkpoint.hpp
        reader_stream.hpp
        reader_stream.cpp
        replayer.hpp
        time.hpp
        time.cpp
        reader_mmap.hpp
        reader_mmap.cpp
        read_result.hpp
        constants.hpp
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
        receive.cpp
        itch.hpp
        itch.cpp
        moldudp64.hpp
        moldudp64.cpp
        src/networking/null_tx_udp_socket.hpp
        src/networking/null_tx_udp_socket.cpp
        time.hpp
        time.cpp
        constants.hpp
        src/networking/rx_udp_socket.cpp
        src/networking/rx_udp_socket.hpp
        src/networking/udp_socket.cpp
        src/networking/udp_socket.hpp
        receiver.cpp
        receiver.hpp
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