cmake_minimum_required(VERSION 3.22...3.30)
project(esp_networking LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")

add_subdirectory(vendors/imtui)
add_subdirectory(vendors/spdlog)
add_subdirectory(common)

add_executable(esp_networking
    server_gui/src/main.cpp
    server_gui/src/networking/tcp_logger.hpp
    server_gui/src/networking/tcp_server.hpp
    server_gui/src/networking/tcp_client_observer.hpp
    server_gui/src/networking/tcp_connection.hpp
    server_gui/src/gui/gui_window.hpp
)

target_include_directories(esp_networking PRIVATE
    ${CMAKE_SOURCE_DIR}/vendors/asio-1.36.0/include
)

target_compile_definitions(esp_networking PRIVATE
    ASIO_STANDALONE
)

target_link_libraries(esp_networking PRIVATE
    spdlog::spdlog_header_only
    imtui-ncurses
    common
)