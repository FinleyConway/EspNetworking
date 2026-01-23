#include <thread>

#include "gui/gui_window.hpp"

/*
TODO: 
    - Need to gracefuly close sockets without errors
*/

int main() {
    gui_window gui_window;
    tcp_server server(gui_window);

    std::thread tcp_thread([&]() {
        server.start_listening(tcp::v4(), 8080);
    });
    
    tcp_thread.detach();
    gui_window.run();

    return 0;
}