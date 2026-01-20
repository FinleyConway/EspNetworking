#include <freertos/FreeRTOS.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_log.h>

#include "wifi_setup.h"
#include "tcp_handler.h"

#define LED_PIN 12
#define RESET_BUTTON_PIN 13

// init esp storage allowing to store information such as wifi configs etc
void init_flash_storage() {
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);
}

void init_gpio() {
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << RESET_BUTTON_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(RESET_BUTTON_PIN, GPIO_MODE_INPUT);
}

void app_main(void) {
    init_gpio();
    init_flash_storage();

    net_status_t network_status = connect_to_wifi();
    if (network_status != NET_WIFI_SUCCESS) {
        ESP_LOGI(WIFI_LOG_TAG, "Failed to associate to AP, dying...");

        return;
    }

    // lesser hard coded? atleast both my pc and laptop have hostname fedora
    // sudo systemctl start avahi-daemon.socket avahi-daemon.service
    tcp_config_t tcp_config = {
        .ip_address = "fedora.local",
        .port = "8080"
    };
    xTaskCreate(receive_from_tcp_server, "SERVER_SEND", 8192, &tcp_config, 1, NULL);
    xTaskCreate(reset_esp_on_button, "ESP_RESET", 2048, NULL, 1, NULL);
}