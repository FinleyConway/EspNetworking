#pragma once

#include <math.h>

#include <freertos/task.h>

#include "motor.h"

static float ease_in_out_cubic(float x) {
    return x < 0.5 ? 4 * x * x * x : 1 - powf(-2 * x + 2, 3) / 2;
}

static float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

void adjust_motor_speed(motor_t* motor) {
    const int loop_ms = 50;
    const int ramp_time_ms = 7500;
    const float time_step = ramp_time_ms / loop_ms;

    while (motor->current_duty_time <= 1.0f) {
        // de/accelerating to target
        float easing_time = ease_in_out_cubic(motor->current_duty_time);
        uint32_t current_duty = (uint32_t)lerp(motor->current_duty, motor->target_duty, easing_time);

        // update time to target duty
        motor->current_duty_time += time_step;

        if (motor->current_duty_time >= 1.0f) {
            motor->current_duty_time = 1.0f;
        }

        // update motor duty and wait
        motor_set_duty(motor, current_duty);
        vTaskDelay(pdMS_TO_TICKS(loop_ms));

        // check for changes mid way de/accelerating
        if (motor_state_receive(motor, 0)) {
            motor->current_duty_time = 0.0f;
        }
    }
}

void motor_task(void* parameters /* motor_t */) {
    motor_t motor = *(motor_t*)parameters; 

    while (true) {
        // check if we recieved a notification, else we block the thread until recieved
        motor_state_receive(&motor, portMAX_DELAY);

        // adjust motor duty over time
        adjust_motor_speed(&motor);
    }
}