cmake_minimum_required(VERSION 3.22...3.30)
project(server_headless LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(server_headless
    src/main.cpp
)

message(PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

target_include_directories(server_headless PRIVATE
    ${CMAKE_SOURCE_DIR}/../vendors/asio-1.36.0/include
)

target_compile_definitions(server_headless PRIVATE
    ASIO_STANDALONE
)