#pragma once

#include <freertos/task.h>

#include <math.h>

#include "motor.h"
#include "sleep.h"

static float ease_in_cubic(float x) {
    return x * x * x;
}

static float lerp(float a, float b, float t) {
    if (t > 1.0f) t = 1.0f;
    if (t < 0.0f) t = 0.0f;

    return a + t * (b - a);
}

void adjust_motor_speed(motor_t* motor) {
    if (!motor->is_active) {
        
    }

    const uint32_t time_between_lerp = 10;
    const float time_step = 0.01f;
    float time = 0.0f;
    uint32_t start_speed = motor->current_speed;

    // lerp to target speed
    while (time < 1.0f) {
        motor->current_speed = lerp(start_speed, motor->target_speed, ease_in_cubic(time));
        set_motor_speed(motor, motor->current_speed);

        time += time_step;
        sleep_ms(10);

        // check if we recieved a new notification
        if (xTaskNotifyWait(0, ULONG_MAX, &motor->target_speed, 0)) {
            start_speed = motor->current_speed;
            time = 0.0f;
        }
    }

    motor->current_speed = motor->target_speed;
}

void motor_control(void* parameters /* motor_t */) {
    motor_t motor = *(motor_t*)parameters; 

    while (true) {
        // check if we recieved a notification, else we block the thread until recieved
        xTaskNotifyWait(0, ULONG_MAX, &motor.target_speed, portMAX_DELAY); 

        adjust_motor_speed(&motor);
    }
}