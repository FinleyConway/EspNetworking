#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// creates a packed entity struct to allow sending data over network
// no padding occurs so bytes are as presented 
typedef struct __attribute__((packed)) { 
    uint64_t id;
    int8_t is_alive;
} net_entity_t;

typedef struct {
    uint64_t id;
    int8_t is_alive;
} entity_t;

net_entity_t host_to_network_entity(entity_t entity);
entity_t network_to_host_entity(net_entity_t net_entity);

#ifdef __cplusplus
}
#endif