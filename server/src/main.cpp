#include <thread>

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
            .is_led_on = false,
            .is_restarting = false
        });
    }

    void on_receive_from(const esp_info_t& entity) override {
        if (entity.is_restarting) {
            get_tcp_server()->disconnect_client_by(entity.esp_id);
        }
    }

    void on_client_disconnect(uint16_t client_id) override {
    }
}; 

/**
 * TODO: 
 * - Start non headless server?
 */

int main() {
    tcp_observer observer;
    tcp_server server(observer);

    std::thread tcp_thread([&]() {
        server.start_listening(tcp::v4(), 8080);
    });

    while (true) {
        std::string out;

        std::cout << "Command: \n";
        std::cin >> out;

        if (out == "close") {
            server.close();
            break;
        }

        if (out == "1") {
            server.send_to_client_by(1, {
                .esp_id = 1,
                .is_led_on = true,
                .is_restarting = false
            });
        }
        if (out == "0") {
            server.send_to_client_by(1, {
                .esp_id = 1,
                .is_led_on = false,
                .is_restarting = false
            });
        }
    }

    tcp_thread.join();

    return 0;
}