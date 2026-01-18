#include "tcp_server.hpp"
#include "tcp_observer.hpp"

// REF: https://think-async.com/Asio/asio-1.36.0/doc/asio/tutorial/tutdaytime3.html
// REF: https://github.com/alejandrofsevilla/boost-tcp-server-client 

class tcp_observer final : public tcp_observer_base 
{
public:
    void on_client_connect(size_t client_id) override {
        get_tcp_server()->send_to_client_by(client_id, {
            .id = 69,
            .is_alive = false
        });
    }

    void on_receive_from(const entity_t& entity) override {
        printf("Entity Recieved: id: %zu, is_alive: %d\n", entity.id, entity.is_alive);
    }

    void on_client_disconnect(size_t client_id) override {
        printf("Connection id lost: %zu", client_id);
    }
}; 

int main() {
    tcp_observer observer;
    tcp_server server(observer);
    server.start_listening(tcp::v4(), 8080);

    return 0;
}