cmake_minimum_required(VERSION 3.22...3.30)
project(common LANGUAGES C)

set(CMAKE_C_STANDARD 17)

add_library(common STATIC
    src/entity.h
    src/entity.c
)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
