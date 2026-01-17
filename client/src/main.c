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

void sleep_ms(int ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS); // convert ticks to ms
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

void app_main(void) {
    gpio_reset_pin(23);
    gpio_set_direction(23, GPIO_MODE_OUTPUT);

    init_flash_storage();

    int network_status = connect_to_wifi();
    
    if (network_status != NET_WIFI_SUCCESS) {
        ESP_LOGI(WIFI_LOG_TAG, "Failed to associate to AP, dying...");

        return;
    }

    tcp_result_t tcp_result = connect_to_tcp_server("192.168.1.191", 8080);

    if (tcp_result.status != NET_TCP_SUCCESS) {
        ESP_LOGI("TCP", "Failed to associate to TCP server, dying...");
    }

    entity_t entity;

    while (1) 
    { 
        ESP_LOGI("Entity", "Recieving...");

        net_status_t s = recv_entity(tcp_result.socket, &entity);

        if (s == NET_TCP_SUCCESS) {
            ESP_LOGI("Entity", "Entity: id:%lld, is_alive%d\n", entity.id, entity.is_alive);
        }
        else {
            ESP_LOGI("Entity", "Failed");
        }
    }
}