#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static void sleep_ms(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}