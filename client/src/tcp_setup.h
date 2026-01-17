#pragma once

#include "net_status.h"

typedef struct {
    int socket;
    net_status_t status;
} tcp_result_t;

tcp_result_t connect_to_tcp_server(const char* address, int port);