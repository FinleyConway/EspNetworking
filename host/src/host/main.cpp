#include "host/networking/tcp/tcp_server.hpp"
#include "host/protocol/handshake_manager.hpp"
#include "host/logging/logger.hpp"

#include "common/messages/handshake.hpp"

void esp_init(void* ctx, const common::esp_init_response_t& res) {
    auto* handshake = static_cast<host::handshake_manager_t*>(ctx);

    handshake->on_response_received(res);
}

int main() {
    host::logger_t::init();
    host::tcp_server_t server;
    host::handshake_manager_t handshake(server, std::chrono::seconds(1));

    server.register_on_connect([&](common::esp_id_t id) {
        handshake.on_connect(id);
    });

    server.register_on_disconnect([&](common::esp_id_t id) {
        LOG_INFO("ESP: {} disconnected", id);
    });

    server.register_receive_callback<common::esp_init_response_t, esp_init>(&handshake);

    server.start();
    
    if (server.toggle_accepting(true) != host::tcp_status_t::success) {
        LOG_INFO("tcp_server not able to accept");
        return -1;
    }

    while(server.is_running()) {
    }
}