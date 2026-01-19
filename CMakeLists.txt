cmake_minimum_required(VERSION 3.22...3.30)
project(esp_networking LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(vendors/spdlog)
add_subdirectory(common)

add_executable(server_headless
    server/src/main.cpp
    server/src/logger.hpp
    server/src/tcp_server.hpp
    server/src/tcp_client_observer.hpp
    server/src/tcp_connection.hpp
)

target_include_directories(server_headless PRIVATE
    ${CMAKE_SOURCE_DIR}/vendors/asio-1.36.0/include
)

target_compile_definitions(server_headless PRIVATE
    ASIO_STANDALONE
)

target_link_libraries(server_headless PRIVATE
    spdlog::spdlog_header_only
    common
)