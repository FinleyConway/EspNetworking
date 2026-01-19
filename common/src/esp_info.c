#include "esp_info.h"

#include <endian.h>

net_esp_info_t esp_info_host_to_network(esp_info_t esp_info) {
    net_esp_info_t net_esp_info = {
        .esp_id = htobe16(esp_info.esp_id), 
        .is_led_on = esp_info.is_led_on,
        .is_restarting = esp_info.is_restarting, 
    };

    return net_esp_info;
}

esp_info_t esp_info_network_to_host(net_esp_info_t net_esp_info) {
    esp_info_t e = {
        .esp_id = be16toh(net_esp_info.esp_id),
        .is_led_on = net_esp_info.is_led_on,
        .is_restarting = net_esp_info.is_restarting,
    };

    return e;
}

bool esp_has_server_conformation(esp_info_t esp_info) {
    return esp_info.esp_id > 0;
}