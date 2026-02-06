#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include <esp_log.h>

#include "wifi_setup.h"
#include "tcp_handler.h"
#include "motion_control/motor_control.h"

// https://mciau.com/how-i-used-freertos-queues-for-safe-task-communication-in-a-real-project/
// https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/02-Queues-mutexes-and-semaphores/01-Queues
// https://www.freertos.org/Documentation/02-Kernel/04-API-references/06-Queues/00-QueueManagement

motor_t init_motor_config() {
    motor_t motor = {
        .current_duty = MOTOR_MIN_DUTY,
        .target_duty = MOTOR_MAX_DUTY,
        .current_duty_time = 0.0f,
        .current_direction = motor_direction_clockwise,
    };
    gpio_num_t a1_pin = 23;
    gpio_num_t a2_pin = 22;

    motor_init(a1_pin, a2_pin);

    return motor;
}

void app_main(void) {
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

    motor_t motor = init_motor_config();
    xTaskCreate(motor_task, "MOTOR_CONTROL", 8192, &motor, 1, NULL);
}