#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// creates a packed esp_info struct to allow sending data over network
// no padding occurs so bytes are as presented 
typedef struct __attribute__((packed)) { 
    uint16_t esp_id;
    int8_t is_led_on;
    int8_t is_restarting;
} net_esp_info_t;

typedef struct {
    uint16_t esp_id;
    int8_t is_led_on;
    int8_t is_restarting;
} esp_info_t;

net_esp_info_t esp_info_host_to_network(esp_info_t esp_info);
esp_info_t esp_info_network_to_host(net_esp_info_t net_esp_info);

bool esp_has_server_conformation(esp_info_t esp_info);

#ifdef __cplusplus
}
#endif