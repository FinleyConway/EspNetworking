#pragma once

#include "driver/ledc.h"
#include "driver/gpio.h"

// REF: https://protonestiot.medium.com/controlling-dc-motors-with-esp-idf-a-comprehensive-guide-2fee1bc00c0e

#define MOTOR_PWM_FREQ 5000
#define MOTOR_PWM_CHANNEL LEDC_CHANNEL_0
#define MOTOR_PWM_MODE LEDC_HIGH_SPEED_MODE
#define MOTOR_PWM_TIMER LEDC_TIMER_0
#define MOTOR_PWM_RES LEDC_TIMER_10_BIT
#define MAX_DUTY_CYCLE 1023

typedef enum {
    motor_direction_none = 0,
    motor_direction_clockwise,
    motor_direction_anti_clockwise
} motor_direction_t;

typedef struct {
    gpio_num_t a1_pin;
    gpio_num_t a2_pin; 
    gpio_num_t motor_enable_pin;
    uint16_t current_speed;
    motor_direction_t current_direction;
} motor_t;

void init_motor(motor_t motor) {
    esp_rom_gpio_pad_select_gpio(motor.a1_pin);
    gpio_set_direction(motor.a1_pin, GPIO_MODE_OUTPUT);

    esp_rom_gpio_pad_select_gpio(motor.a2_pin);
    gpio_set_direction(motor.a2_pin, GPIO_MODE_OUTPUT);

    // configure pwm timer
    ledc_timer_config_t pwm_timer = {
        .speed_mode         = MOTOR_PWM_MODE,
        .duty_resolution    = MOTOR_PWM_RES,
        .timer_num          = MOTOR_PWM_TIMER,
        .freq_hz            = MOTOR_PWM_FREQ,
        .clk_cfg            = LEDC_AUTO_CLK
    };
    ledc_timer_config(&pwm_timer);

    ledc_channel_config_t pwm_channel = {
        .gpio_num       = motor.motor_enable_pin, 
        .speed_mode     = MOTOR_PWM_MODE,
        .channel        = MOTOR_PWM_CHANNEL,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = MOTOR_PWM_TIMER,
        .duty           = 0,
        .hpoint         = 0,
    };
    ledc_channel_config(&pwm_channel);
}

void set_motor_direction(motor_t* motor, motor_direction_t direction) {
    if (motor == NULL) return;

    if (direction == motor_direction_clockwise) {
        gpio_set_level(motor->a1_pin, 1);
        gpio_set_level(motor->a2_pin, 0);
    }
    else if (direction == motor_direction_anti_clockwise) {
        gpio_set_level(motor->a1_pin, 0);
        gpio_set_level(motor->a2_pin, 1);
    }
    else if (direction == motor_direction_none) {
        gpio_set_level(motor->a1_pin, 0);
        gpio_set_level(motor->a2_pin, 0);
    }
    else return; // probably add a assert or something here later

    motor->current_direction = direction;
}

void set_motor_speed(motor_t* motor, uint16_t speed) {
    if (motor == NULL) return;

    if (MAX_DUTY_CYCLE > motor->current_speed) {
        ledc_set_duty(MOTOR_PWM_MODE, MOTOR_PWM_CHANNEL, speed);
        ledc_update_duty(MOTOR_PWM_MODE, MOTOR_PWM_CHANNEL);

        motor->current_speed = speed;
    }
}

void offset_motor_speed(motor_t* motor, uint16_t speed) {
    if (motor == NULL) return;

    if (MAX_DUTY_CYCLE > motor->current_speed) {
        ledc_set_duty(MOTOR_PWM_MODE, MOTOR_PWM_CHANNEL, speed);
        ledc_update_duty(MOTOR_PWM_MODE, MOTOR_PWM_CHANNEL);

        motor->current_speed += speed;
    }
}