#include "tcp_setup.h"

#include <sys/socket.h>

#include <esp_log.h>

tcp_result_t connect_to_tcp_server(const char* address, int port) {
    tcp_result_t result = { .socket = -1, .status = NET_TCP_FAILURE };
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0) {
        ESP_LOGE("TCP", "Failed to create a socket");

        return result;
    }

    struct sockaddr_in server_address = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = inet_addr(address)
    }; 

    if (connect(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        ESP_LOGE("TCP", "Failed to connect to server");

        close(socket_fd);

        return result;
    }

    result.socket = socket_fd;
    result.status = NET_TCP_SUCCESS;

    return result;
}