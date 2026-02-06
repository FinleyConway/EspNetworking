#include "tcp_io.h"

#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>

#include "tcp_connection.h"

net_status_t send_esp_info(esp_info_t esp_info) {
    net_esp_info_t net_esp_info = esp_info_host_to_network(esp_info);
    uint8_t buffer[sizeof(net_esp_info_t)];
    size_t total_bytes_sent = 0;

    memcpy(buffer, &net_esp_info, sizeof(net_esp_info_t)); 

    // wait until all bytes have been sent successfully 
    while (total_bytes_sent < sizeof(net_esp_info_t)) {
        ssize_t bytes_sent = send(
            tcp_socket(), 
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

net_status_t recv_esp_info(esp_info_t* out_esp_info) {
    if (out_esp_info == NULL) return NET_TCP_FAILURE;

    net_esp_info_t net_esp_info = {0};
    uint8_t buffer[sizeof(net_esp_info_t)];
    size_t total_bytes_received = 0;

    // wait until all bytes have arrived successfully
    while (total_bytes_received < sizeof(net_esp_info_t)) {
        ssize_t bytes_received = recv(
            tcp_socket(), 
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