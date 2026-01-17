#pragma once

#include "net_status.h"

// ref: https://www.youtube.com/watch?v=_dRrarmQiAM 

#define MAX_FAILURES 10

static const char* WIFI_LOG_TAG = "WIFI"; // wifi logging tag

net_status_t connect_to_wifi(void);