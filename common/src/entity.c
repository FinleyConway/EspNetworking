#include "entity.h"

#include <endian.h>

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