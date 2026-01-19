#include "tcp_handler.h"

#include <sys/socket.h>

#include <esp_log.h>

#include "sleep.h"
#include "esp_info.h"
#include "wifi_setup.h"
#include "handle_tcp_request.h"

// ===================== Private ===================== 
#define RESET_BUTTON_PIN 13

static const char* TCP_LOG_TAG = "TCP"; 
int tcp_socket = 0;
esp_info_t working_esp_info = {0};

typedef struct {
    int socket;
    net_status_t status;
} tcp_result_t;

tcp_result_t connect_to_tcp_server(const char* address, uint16_t port) {
    tcp_result_t result = { .socket = -1, .status = NET_TCP_FAILURE };
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0) {
        ESP_LOGE(TCP_LOG_TAG, "Failed to create a socket");

        return result;
    }

    struct sockaddr_in server_address = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = inet_addr(address)
    }; 

    if (connect(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        ESP_LOGE(TCP_LOG_TAG, "Failed to connect to server");

        close(socket_fd);

        return result;
    }

    result.socket = socket_fd;
    result.status = NET_TCP_SUCCESS;

    return result;
}

net_status_t send_esp_info(int socket, esp_info_t esp_info) {
    net_esp_info_t net_esp_info = esp_info_host_to_network(esp_info);
    uint8_t buffer[sizeof(net_esp_info_t)];
    size_t total_bytes_sent = 0;

    memcpy(buffer, &net_esp_info, sizeof(net_esp_info_t)); 

    // wait until all bytes have been sent successfully 
    while (total_bytes_sent < sizeof(net_esp_info_t)) {
        ssize_t bytes_sent = send(
            socket, 
            buffer + total_bytes_sent, 
            sizeof(net_esp_info_t) - total_bytes_sent, 
            0
        );

        if (bytes_sent == -1) return NET_TCP_FAILURE;
        if (bytes_sent == 0) return NET_TCP_CLOSE_CONNECTION;

        total_bytes_sent += bytes_sent;
    }

    return NET_TCP_SUCCESS;
}

net_status_t recv_esp_info(int socket, esp_info_t* out_esp_info) {
    if (out_esp_info == NULL) return NET_TCP_FAILURE;

    net_esp_info_t net_esp_info = {0};
    uint8_t buffer[sizeof(net_esp_info_t)];
    size_t total_bytes_received = 0;

    // wait until all bytes have arrived successfully
    while (total_bytes_received < sizeof(net_esp_info_t)) {
        ssize_t bytes_received = recv(
            socket, 
            buffer + total_bytes_received, 
            sizeof(net_esp_info_t) - total_bytes_received, 
            0
        );

        if (bytes_received == -1) return NET_TCP_FAILURE;
        if (bytes_received == 0) return NET_TCP_CLOSE_CONNECTION;

        total_bytes_received += bytes_received;
    }

    memcpy(&net_esp_info, buffer, sizeof(net_esp_info_t));
    *out_esp_info = esp_info_network_to_host(net_esp_info);

    return NET_TCP_SUCCESS;
}

void try_notify_tcp_server_on_reset() {
     // this is created to avoid changes due to multiple tasks running
    esp_info_t reset_esp = {
        .esp_id = working_esp_info.esp_id,
        .is_led_on = false,
        .is_restarting = true
    };

    // attempt to notify server about restarting
    net_status_t status = send_esp_info(tcp_socket, reset_esp); 

    if (status == NET_TCP_SUCCESS) {
        sleep_ms(200); // bit rough but just wait a little for server to know
    }

     // restart esp
    shutdown_wifi();
    esp_restart();
}

// ===================== Private ===================== 

void receive_from_tcp_server(void* parameters /* tcp_config_t */) {
    tcp_config_t tcp_config = *(tcp_config_t*)parameters;
    tcp_result_t tcp_result = connect_to_tcp_server(tcp_config.ip_address, tcp_config.port);

    if (tcp_result.status == NET_TCP_FAILURE) {
        ESP_LOGE(TCP_LOG_TAG, "Failed to associate to TCP server, dying...");
        vTaskDelete(NULL);
    }

    tcp_socket = tcp_result.socket;

    ESP_LOGI(TCP_LOG_TAG, "Attempting to receive conformation from server...");

    while (true) {
        net_status_t status = recv_esp_info(tcp_socket, &working_esp_info);

        switch (status) {
            case NET_TCP_SUCCESS:
                handle_request(working_esp_info);
                break;

            case NET_TCP_FAILURE:
                ESP_LOGE(TCP_LOG_TAG, "Failed to receive from server!");
                // prehaps wait and retry rejoin with a certain amount of attempts?
                break;

            case NET_TCP_CLOSE_CONNECTION:
            default:
                ESP_LOGI(TCP_LOG_TAG, "Connection closed, dying...");
                vTaskDelete(NULL);
        }
    }
}

void reset_esp_on_button(void* parameters) {
    while (true) {
        bool level = gpio_get_level(RESET_BUTTON_PIN);

        if (!level) {
            try_notify_tcp_server_on_reset();
        }

        sleep_ms(100);
    }
}