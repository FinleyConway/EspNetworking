#pragma once

#include <freertos/FreeRTOS.h>

#include "task_events/tcp_send_event.hpp"
#include "networking/tcp_client.hpp"

namespace client {
    class tcp_task_t {
    public:
        static void init(UBaseType_t priority_offset) {
            xTaskCreate(
                run,
                "tcp_task_t",
                s_stack_size,
                nullptr,
                tskIDLE_PRIORITY + priority_offset,
                &s_handle
            );
        }
        
        static TaskHandle_t get_handle() {
            return s_handle;
        }

    private:
        static void run(void* parameters) {
            tcp_client_t client;

            register_messages(client);

            while (true) {
                try_connect(client);

                while (client.is_connected()) {
                    if (!handle_send(client)) {
                        break;
                    }

                    tcp_status_t status = client.listen_to_server();

                    switch (status) {

                        case tcp_status_t::success: break;

                        case tcp_status_t::timeout:
                            ESP_LOGW("TCP_TASK", "Timeout, trying again");
                            break;

                        case tcp_status_t::unknown_packet:
                            ESP_LOGW("TCP_TASK", "Unknown packet received");
                            continue;

                        case tcp_status_t::connection_closed:
                            ESP_LOGW("TCP_TASK", "Server closed connection");
                            goto disconnect;

                        case tcp_status_t::failed_to_connect:
                        case tcp_status_t::socket_failure:
                        case tcp_status_t::failure:
                        default:
                            ESP_LOGE("TCP_TASK", "Fatal socket error in recv loop (%d)", static_cast<int>(status));
                            goto disconnect; 

                    }
                }
            
            disconnect:
                ESP_LOGI("TCP_TASK", "Disconnected");

                client.disconnect();
            }

            vTaskDelete(nullptr);
        }

        static void register_messages(tcp_client_t& client) {
            client.register_receieve_callback<common::esp_init_request_t, &on_init_request>(nullptr);
        }

        static void try_connect(tcp_client_t& client) {
            // TODO: Add retry attempts

            constexpr TickType_t retry_delay = pdMS_TO_TICKS(5000);
            constexpr int32_t tcp_timeout_sec = 60;
            
            ESP_LOGI("TCP_TASK", "Connecting...");

            while (client.try_connect(tcp_timeout_sec) != tcp_status_t::success) {
                vTaskDelay(retry_delay);
            }

            ESP_LOGI("TCP_TASK", "Connected");
        }

        static bool handle_send(tcp_client_t& client) {
            tcp_status_t status{};
            tcp_event_data_t event_data;

            if (!tcp_send_event_t::receive(event_data, 0)) return true; // nothing to send

            switch (event_data.type) {

                case tcp_event_data_t::type_t::init_respond: {
                    status = client.send_to_server(event_data.data.init_respond);
                    break;
                }

            }

            if (status == tcp_status_t::success) {
                return true;
            }

            return handle_send_errors(status);
        }

        static bool handle_send_errors(tcp_status_t status) {
            switch (status) {

                case tcp_status_t::timeout:
                    ESP_LOGW("TCP_TASK", "Timeout, trying again");
                    return true; 

                case tcp_status_t::connection_closed:
                    ESP_LOGW("TCP_TASK", "Connection closed during send");
                    return false;

                case tcp_status_t::socket_failure:
                    ESP_LOGE("TCP_TASK", "Socket failure during send (errno=%d)", errno);
                    return false;

                case tcp_status_t::failure:
                default:
                    ESP_LOGE("TCP_TASK", "Unknown send failure (status=%d, errno=%d)", static_cast<int>(status), errno);
                    return false;

            }
        }

    private:
        static void on_init_request(void* ctx, const common::esp_init_request_t& init_request) {
            ESP_LOGI("TCP_TASK", "Received server response, sending ack...");

            tcp_event_data_t event_data;
            event_data.type = tcp_event_data_t::type_t::init_respond;
            event_data.data.init_respond = {
                .id = init_request.id
            };

            tcp_send_event_t::send(event_data);

            // store id somewhere
        }

    private:
        inline static TaskHandle_t s_handle = nullptr;
        inline static uint32_t s_stack_size = 8192;
    };        
}