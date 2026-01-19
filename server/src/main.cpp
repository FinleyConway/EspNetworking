#include "tcp_server.hpp"
#include "tcp_client_observer.hpp"

// REF: https://think-async.com/Asio/asio-1.36.0/doc/asio/tutorial/tutdaytime3.html
// REF: https://github.com/alejandrofsevilla/boost-tcp-server-client 

class tcp_observer final : public tcp_client_observer_base 
{
public:
    void on_client_connect(uint16_t client_id) override {
        get_tcp_server()->send_to_client_by(client_id, {
            .esp_id = client_id,
            .is_restarting = false
        });
    }

    void on_receive_from(const entity_t& entity) override {
        printf("Entity Recieved: id: %d, is_restarting: %d\n", entity.esp_id, entity.is_restarting);

        if (entity.is_restarting) {
            get_tcp_server()->disconnect_client_by(entity.esp_id);
        }
    }

    void on_client_disconnect(uint16_t client_id) override {
        printf("Client %d disconnected\n", client_id);
    }
}; 

/**
 * TODO: 
 * - Handle esp re-assign if the esp is just restarting? take advantage of esp_restet_reason()?
 * - More meaningful data i/o
 * - Start non headless server?
 * - Better Logging
 * - Rename entity to better name
 */

int main() {
    tcp_observer observer;
    tcp_server server(observer);
    server.start_listening(tcp::v4(), 8080);

    return 0;
}