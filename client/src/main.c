#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>

#include <esp_system.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <driver/gpio.h>

#include "wifi_setup.h"
#include "tcp_setup.h"
#include "net_status.h"
#include "entity_io.h"

#define LED_PIN 12
#define RESET_BUTTON_PIN 13

int socket = 0;
esp_info_t entity;

bool has_server_conformation() {
    return entity.esp_id > 0;
}

void sleep_ms(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

// init esp storage allowing to store information such as wifi configs etc
void init_flash_storage() {
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);
}

void try_notify_server_on_reset() {
     // this is created to avoid changes due to multiple tasks running
    esp_info_t reset_entity = {
        .esp_id = entity.esp_id,
        .is_restarting = true
    };

    // attempt to notify server about restarting
    net_status_t status = send_entity(socket, reset_entity); 

    if (status == NET_TCP_SUCCESS) {
        sleep_ms(200); // bit rough but just wait a little for server to know
    }

     // restart esp
    shutdown_wifi();
    esp_restart();
}

void reset_esp_on_button(void* parameters) {
    while (true) {
        bool level = gpio_get_level(RESET_BUTTON_PIN);

        if (!level) {
            try_notify_server_on_reset();
        }

        sleep_ms(100);

        //ESP_LOGI("STACK", "reset_esp_on_button stack use: %d", uxTaskGetStackHighWaterMark(NULL));
    }
}

void recv_entity_from_server(void* parameters) {
    tcp_result_t tcp_result = connect_to_tcp_server("192.168.1.115", 8080);

    if (tcp_result.status == NET_TCP_FAILURE) {
        ESP_LOGI("TCP", "Failed to associate to TCP server, dying...");
        vTaskDelete(NULL);
    }

    socket = tcp_result.socket;

    ESP_LOGI("TCP", "Attempting to receive conformation from server...");

    while (true) {
        net_status_t status = recv_entity(socket, &entity);

        if (status == NET_TCP_SUCCESS) {
            ESP_LOGI("TCP", "Recv: esp_id: %d, is_restarting: %d", entity.esp_id, entity.is_restarting);

            if (has_server_conformation()) {
                gpio_set_level(LED_PIN, entity.is_led_on);
            }
        }
        else if (status == NET_TCP_CLOSE_CONNECTION) {
            ESP_LOGI("TCP", "Connection closed, dying...");
            vTaskDelete(NULL);
        }
        else 
        {
            ESP_LOGE("TCP", "Failed to receive from server!");
        }

        sleep_ms(1000);

        //ESP_LOGI("STACK", "recv_entity_from_server stack use: %d", uxTaskGetStackHighWaterMark(NULL));
    }
}

void app_main(void) {
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

    init_flash_storage();

    int network_status = connect_to_wifi();
    
    if (network_status != NET_WIFI_SUCCESS) {
        ESP_LOGI(WIFI_LOG_TAG, "Failed to associate to AP, dying...");

        return;
    }

    xTaskCreate(
        recv_entity_from_server,
        "SERVER_ENTITY_SEND",
        8192, // stack depth
        NULL,
        1, // task priority
        NULL
    );

    xTaskCreate(
        reset_esp_on_button,
        "ESP_RESET",
        2048, // stack depth
        NULL,
        2, // task priority
        NULL
    );
}