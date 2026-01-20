#include <thread>

#include "networking/tcp_server.hpp"
#include "networking/tcp_client_observer.hpp"

#include "gui/gui_main.hpp"

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

    void on_receive_from(const esp_info_t& esp_info) override {
        if (esp_info.is_restarting) {
            get_tcp_server()->disconnect_client_by(esp_info.esp_id);
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
    gui_main gui_main;

    std::thread tcp_thread([&]() {
        server.start_listening(tcp::v4(), 8080);
    });
    gui_main.run();

    tcp_thread.join();

    return 0;
}