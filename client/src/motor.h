#pragma once

#include "driver/ledc.h"
#include "driver/gpio.h"

// REF: https://protonestiot.medium.com/controlling-dc-motors-with-esp-idf-a-comprehensive-guide-2fee1bc00c0e

#define MOTOR_PWM_FREQ 18000
#define MOTOR_PWM_MODE LEDC_HIGH_SPEED_MODE
#define MOTOR_PWM_RES LEDC_TIMER_10_BIT 
#define MOTOR_MAX_DUTY ((1 << MOTOR_PWM_RES) - 1) 

typedef enum {
    motor_direction_none = 0,
    motor_direction_clockwise,
    motor_direction_anti_clockwise
} motor_direction_t;

typedef struct {
    motor_direction_t current_direction;
    uint32_t current_speed;
    uint32_t target_speed;
    bool is_active;
} motor_t;

void init_motor(gpio_num_t a1, gpio_num_t a2);
void set_motor_direction(motor_t* motor, motor_direction_t direction);
void set_motor_speed(motor_t* motor, uint32_t speed);