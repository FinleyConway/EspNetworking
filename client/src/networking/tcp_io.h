#pragma once

#include "net_status.h"
#include "esp_info.h"

net_status_t send_esp_info(esp_info_t esp_info);
net_status_t recv_esp_info(esp_info_t* out_esp_info);