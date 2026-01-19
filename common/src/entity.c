#include "entity.h"

#include <endian.h>

net_esp_info_t host_to_network_entity(esp_info_t entity) {
    net_esp_info_t net_entity = {
        .esp_id = htobe16(entity.esp_id), 
        .is_led_on = entity.is_led_on,
        .is_restarting = entity.is_restarting, 
    };

    return net_entity;
}

esp_info_t network_to_host_entity(net_esp_info_t net_entity) {
    esp_info_t e = {
        .esp_id = be16toh(net_entity.esp_id),
        .is_led_on = net_entity.is_led_on,
        .is_restarting = net_entity.is_restarting,
    };

    return e;
}