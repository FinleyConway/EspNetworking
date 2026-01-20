#pragma once

#include <stdint.h>

#include "net_status.h"

typedef struct {
    const char* ip_address;
    const char* port;
} tcp_config_t;

void receive_from_tcp_server(void* parameters);
void reset_esp_on_button(void* parameters);
