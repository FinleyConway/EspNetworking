#include "entity.h"

#include <string.h>
#include <stdalign.h>

#include <endian.h>
#include <sys/socket.h>

//#include <esp_log.h>

// ==========private==========
// // creates a packed entity struct to allow sending data over network
// // no padding occurs so bytes are as presented 
// typedef struct __attribute__((packed)) { 
//     uint64_t id;
//     int8_t is_alive;
// } net_entity_t;

// net_entity_t host_to_network_entity(entity_t entity) {
//     net_entity_t net_entity = {
//         .id = htobe64(entity.id), // convert endians for network (swap byte sequence)
//         .is_alive = entity.is_alive, // not needed for byte since swapping byte is the name
//     };

//     return net_entity;
// }

// entity_t network_to_host_entity(net_entity_t net_entity) {
//     entity_t e = {
//         .id = be64toh(net_entity.id), // convert endians for host (swap byte sequence)
//         .is_alive = net_entity.is_alive,
//     };

//     return e;
// }
// ==========private==========

// impl ref: https://stackoverflow.com/questions/78331087/can-a-tcp-ip-socket-send-less-than-the-bytes-requested-via-write-send

net_status_t send_entity(int socket, entity_t entity) {
    net_entity_t net_entity = host_to_network_entity(entity);
    uint8_t buffer[sizeof(net_entity_t)];
    size_t total_bytes_sent = 0;

    memcpy(buffer, &net_entity, sizeof(net_entity_t)); 

    // wait until all bytes have been sent successfully 
    while (total_bytes_sent < sizeof(net_entity_t)) {
        ssize_t bytes_sent = send(
            socket, 
            buffer + total_bytes_sent, 
            sizeof(net_entity_t) - total_bytes_sent, 
            0
        );

        if (bytes_sent == -1) return NET_TCP_FAILURE;
        if (bytes_sent == 0) return NET_TCP_CLOSE_CONNECTION;

        total_bytes_sent += bytes_sent;
    }

    return NET_TCP_SUCCESS;
}

net_status_t recv_entity(int socket, entity_t* out_entity) {
    if (out_entity == NULL) return NET_TCP_FAILURE;

    net_entity_t net_entity = {0};
    uint8_t buffer[sizeof(net_entity_t)];
    size_t total_bytes_received = 0;

    // wait until all bytes have arrived successfully
    while (total_bytes_received < sizeof(net_entity_t)) {
        ssize_t bytes_received = recv(
            socket, 
            buffer + total_bytes_received, 
            sizeof(net_entity_t) - total_bytes_received, 
            0
        );

        if (bytes_received == -1) return NET_TCP_FAILURE;
        if (bytes_received == 0) return NET_TCP_CLOSE_CONNECTION;

        total_bytes_received += bytes_received;
    }

    memcpy(&net_entity, buffer, sizeof(net_entity_t));
    *out_entity = network_to_host_entity(net_entity);

    return NET_TCP_SUCCESS;
}