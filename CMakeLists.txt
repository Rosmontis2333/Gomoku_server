cmake_minimum_required(VERSION 3.30)
project(Gomoku_server)

set(CMAKE_CXX_STANDARD 20)

add_executable(Gomoku_server main.cpp
        Player.h
        Room.h
        Network.cpp
        Network.h
        json.hpp
        Room.cpp
        Player.cpp
)
target_link_libraries(Gomoku_server ws2_32)