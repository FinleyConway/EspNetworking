#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "net_status.h"

// creates a packed entity struct to allow sending data over network
// no padding occurs so bytes are as presented 
typedef struct __attribute__((packed)) { 
    uint64_t id;
    int8_t is_alive;
} net_entity_t;

net_entity_t host_to_network_entity(entity_t entity) {
    net_entity_t net_entity = {
        .id = htobe64(entity.id), // convert endians for network (swap byte sequence)
        .is_alive = entity.is_alive, // not needed for byte since swapping byte is the name
    };

    return net_entity;
}

entity_t network_to_host_entity(net_entity_t net_entity) {
    entity_t e = {
        .id = be64toh(net_entity.id), // convert endians for host (swap byte sequence)
        .is_alive = net_entity.is_alive,
    };

    return e;
}

typedef struct {
    uint64_t id;
    int8_t is_alive;
} entity_t;

net_status_t send_entity(int socket, entity_t entity);
net_status_t recv_entity(int socket, entity_t* out_entity);

#ifdef __cplusplus
}
#endif