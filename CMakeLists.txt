cmake_minimum_required(VERSION 4.0)
project(nanoparser)

set(CMAKE_CXX_STANDARD 23)

add_executable(replay
        replay.cpp
        itch_protocol.hpp
        networking.cpp
        networking.hpp
        moldudp64_protocol.hpp)

add_executable(extract_timestamps
        itch_protocol.hpp
        extract_timestamps.cpp)
