cmake_minimum_required(VERSION 2.8)
project(joykeyd)
add_executable (joykeyd joykeyd.c)

target_link_libraries(joykeyd X11 Xtst)
