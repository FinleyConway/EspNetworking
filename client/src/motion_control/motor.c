#include "motor.h"

#include <freertos/queue.h>
#include <esp_log.h>

static QueueHandle_t s_motor_queue = NULL;

void motor_init(gpio_num_t a1, gpio_num_t a2) {
    // configure pwm timer
    ledc_timer_config_t pwm_timer = {
        .speed_mode         = LEDC_HIGH_SPEED_MODE,
        .duty_resolution    = MOTOR_PWM_RES,
        .timer_num          = LEDC_TIMER_0,
        .freq_hz            = MOTOR_PWM_FREQ,
        .clk_cfg            = LEDC_AUTO_CLK
    };
    ledc_timer_config(&pwm_timer);

    // configure pwm channel
    ledc_channel_config_t pwm_channel_0 = {
        .gpio_num       = a1, 
        .speed_mode     = MOTOR_PWM_MODE,
        .channel        = LEDC_CHANNEL_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0,
        .hpoint         = 0,
    };
    ledc_channel_config(&pwm_channel_0);

    // configure pwm channel
    ledc_channel_config_t pwm_channel_1 = {
        .gpio_num       = a2, 
        .speed_mode     = MOTOR_PWM_MODE,
        .channel        = LEDC_CHANNEL_1,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0,
        .hpoint         = 0,
    };
    ledc_channel_config(&pwm_channel_1);

    // create a motor queue so other tasks can control the state of the motor
    s_motor_queue = xQueueCreate(1, sizeof(motor_t));

    if (s_motor_queue == NULL) {
        ESP_LOGE("MOTOR", "Motor queue handler failed to create");
    }
}

void motor_set_direction(motor_t* motor, motor_direction_t direction) {
    if (motor == NULL) {
        ESP_LOGE("MOTOR", "Cant change motor direction due to null motor object");
        return;
    }

    // [a1, a2]
    // [0,   1] - clockwise
    // [1,   0] - anti-clockwise
    // [0,   0] - stop

    if (direction == motor_direction_clockwise) {
        ledc_set_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_0, motor->current_duty);
        ledc_set_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_1, 0);
    }
    else if (direction == motor_direction_anti_clockwise) {
        ledc_set_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_0, 0);
        ledc_set_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_1, motor->current_duty);
    }
    else if (direction == motor_direction_none) {
        ledc_set_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_0, 0);
        ledc_set_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_1, 0);
    }
    else return; // probably add a assert or something here later

    ledc_update_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_0);
    ledc_update_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_1);

    motor->current_direction = direction;
}

void motor_set_duty(motor_t* motor, uint32_t duty) {
    if (motor == NULL) {
        ESP_LOGE("MOTOR", "Cant change motor duty due to null motor object");
        return;
    }

    motor->current_duty = duty;

    motor_set_direction(motor, motor->current_direction);
}

bool motor_update_state(motor_t motor) {
    if (s_motor_queue == NULL) {
        ESP_LOGE("MOTOR", "Motor queue handler is null");
        return false;
    }

    return xQueueOverwrite(s_motor_queue, &motor) == pdPASS;
}

bool motor_state_receive(motor_t* out_motor, TickType_t timeout) {
    if (s_motor_queue == NULL) {
        ESP_LOGE("MOTOR", "Motor queue handler is null");
        return false;
    }   

    if (out_motor == NULL) {
        ESP_LOGE("MOTOR", "Motor recieve object is null");
        return false;
    }

    return xQueueReceive(s_motor_queue, &(out_motor), timeout) == pdPASS;
}