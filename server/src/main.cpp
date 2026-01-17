#include "tcp_server.hpp"

// REF: https://think-async.com/Asio/asio-1.36.0/doc/asio/tutorial/tutdaytime3.html
// REF: https://github.com/alejandrofsevilla/boost-tcp-server-client 

int main() {
    tcp_server server;
    server.set_on_accept_callback([&](size_t id){
        server.send(id, {
            .id = 69,
            .is_alive = false
        });
    });
    server.start_listening(tcp::v4(), 8080);

    return 0;
}