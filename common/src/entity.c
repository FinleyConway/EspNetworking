#include "entity.h"

#include <endian.h>

net_entity_t host_to_network_entity(entity_t entity) {
    net_entity_t net_entity = {
        .esp_id = htobe16(entity.esp_id), 
        .is_restarting = entity.is_restarting, 
    };

    return net_entity;
}

entity_t network_to_host_entity(net_entity_t net_entity) {
    entity_t e = {
        .esp_id = be16toh(net_entity.esp_id),
        .is_restarting = net_entity.is_restarting,
    };

    return e;
}