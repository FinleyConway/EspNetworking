#pragma once

#include <stdint.h>

#include "net_status.h"

typedef struct {
    uint64_t id;
    int8_t is_alive;
} entity_t;

net_status_t send_entity(int socket, entity_t entity);
net_status_t recv_entity(int socket, entity_t* out_entity);