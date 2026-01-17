#pragma once

typedef enum {
    NET_WIFI_SUCCESS         = 1 << 0,
    NET_WIFI_FAILURE         = 1 << 1,
    NET_TCP_SUCCESS          = 1 << 2, 
    NET_TCP_FAILURE          = 1 << 3,
    NET_TCP_CLOSE_CONNECTION = 1 << 4,
} net_status_t;