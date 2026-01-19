#pragma once

#include <driver/gpio.h>

#include "esp_info.h"

#define LED_PIN 12

void handle_request(esp_info_t esp_info) {
    gpio_set_level(LED_PIN, esp_info.is_led_on);
}