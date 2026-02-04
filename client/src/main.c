#include <freertos/FreeRTOS.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_log.h>

#include "wifi_setup.h"
#include "tcp_handler.h"

#include <math.h>
#include "motor.h"

// init esp storage allowing to store information such as wifi configs etc
void init_flash_storage() {
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);
}

static float ease(float x) {
    return -(cosf(M_PI * x) - 1) / 2;
}

static float lerp(float a, float b, float t) {
    if (t > 1.0f) t = 1.0f;
    if (t < 0.0f) t = 0.0f;

    return a + t * (b - a);
}

void app_main(void) {
    // init_flash_storage();

    // net_status_t network_status = connect_to_wifi();
    // if (network_status != NET_WIFI_SUCCESS) {
    //     ESP_LOGI(WIFI_LOG_TAG, "Failed to associate to AP, dying...");

    //     return;
    // }

    // // lesser hard coded? atleast both my pc and laptop have hostname fedora
    // // sudo systemctl start avahi-daemon.socket avahi-daemon.service
    // tcp_config_t tcp_config = {
    //     .ip_address = "fedora.local",
    //     .port = "8080"
    // };
    // xTaskCreate(receive_from_tcp_server, "SERVER_SEND", 8192, &tcp_config, 1, NULL);

    vTaskDelay(pdMS_TO_TICKS(5000));

    motor_t motor = {
        .current_direction = motor_direction_clockwise,
        .current_speed = 0,
        .target_speed = 0,
        .is_active = true
    };
    gpio_num_t a1_pin = 23;
    gpio_num_t a2_pin = 22;

    init_motor(a1_pin, a2_pin);

    #define RAMP_TIME_MS 7500
    #define LOOP_MS 50

    float time_step = (float)LOOP_MS / RAMP_TIME_MS;
    float time = 0.0f;
    float ramp_start = 700.0f; // duty where it initally starts turning
    float ramp_end = MOTOR_MAX_DUTY;

    while (true) {
        float t = ease(time);
        uint32_t speed = (uint32_t)lerp(ramp_start, ramp_end, t);

        ESP_LOGI("SPEED", "%d", speed);

        set_motor_speed(&motor, speed);

        time += time_step;
        if (time >= 1.0f) {
            time = 0.0f;

            float start = ramp_start;
            ramp_start = ramp_end;
            ramp_end = start;
        }

        vTaskDelay(pdMS_TO_TICKS(LOOP_MS));
    }
}