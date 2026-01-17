#include "tcp_server.hpp"

// REF: https://think-async.com/Asio/asio-1.36.0/doc/asio/tutorial/tutdaytime3.html
// REF: https://github.com/alejandrofsevilla/boost-tcp-server-client 

int main() {
    tcp_server server;

    server.set_on_accept_callback([&](size_t id) {
        server.send_to_client_by(id, {
            .id = 69,
            .is_alive = false
        });
    });

    server.set_on_receive_callback([&](const entity_t& entity) {
        printf("Entity Recieved: id: %ull, is_alive: %d\n");
    });

    server.set_on_close_connection_callback([&](size_t id) {
        printf("Connection id lost: %ull", id);
    });

    server.start_listening(tcp::v4(), 8080);

    return 0;
}