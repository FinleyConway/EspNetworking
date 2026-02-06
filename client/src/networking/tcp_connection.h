#pragma once

#include <stdbool.h>

#include "net_status.h"

int tcp_socket(void);
net_status_t tcp_connect_to(const char* address, const char* port);
net_status_t tcp_disconnect(void);
bool tcp_is_connected(void);