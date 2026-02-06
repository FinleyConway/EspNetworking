#include "tcp_connection.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <esp_log.h>

static const char* s_tag = "TCP"; 
static int s_socket = -1;

int tcp_socket(void) {
    return s_socket;
}

net_status_t tcp_connect_to(const char* address, const char* port) {
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM
    };
    struct addrinfo* res;
    int error = getaddrinfo(address, port, &hints, &res);
    
    if (error == 0) {
        int socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

        if (socket_fd < 0) {
            ESP_LOGE(s_tag, "Failed to create a socket");

            return NET_TCP_FAILURE;
        }

        if (connect(socket_fd, res->ai_addr, res->ai_addrlen) < 0) {
            ESP_LOGE(s_tag, "Failed to connect to server");

            tcp_disconnect();

            return NET_TCP_FAILURE;
        }

        s_socket = socket_fd;
    }

    return NET_TCP_SUCCESS;
}

net_status_t tcp_disconnect(void) {
    if (close(s_socket) < 0) {
        ESP_LOGE(s_tag, "Failed to close socket");
        return NET_TCP_FAILURE;
    }

    return NET_TCP_SUCCESS;
}

bool tcp_is_connected(void) {
    return s_socket >= 0;
}