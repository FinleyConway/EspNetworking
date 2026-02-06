#pragma once

#include <freertos/FreeRTOS.h>
#include <driver/ledc.h>
#include <driver/gpio.h>

// REF: https://protonestiot.medium.com/controlling-dc-motors-with-esp-idf-a-comprehensive-guide-2fee1bc00c0e

#define MOTOR_PWM_FREQ 18000
#define MOTOR_PWM_MODE LEDC_HIGH_SPEED_MODE
#define MOTOR_PWM_RES LEDC_TIMER_10_BIT 
#define MOTOR_MIN_DUTY 700 // roughly when the tt motor starts turning
#define MOTOR_MAX_DUTY 1023

typedef enum {
    motor_direction_none = 0,
    motor_direction_clockwise,
    motor_direction_anti_clockwise
} motor_direction_t;

typedef struct {
    uint32_t current_duty;
    uint32_t target_duty;
    float current_duty_time;
    motor_direction_t current_direction;
} motor_t;

void motor_init(gpio_num_t a1, gpio_num_t a2);
void motor_set_direction(motor_t* motor, motor_direction_t direction);
void motor_set_duty(motor_t* motor, uint32_t duty);

bool motor_update_state(motor_t motor);
bool motor_state_receive(motor_t* out_motor, TickType_t timeout);