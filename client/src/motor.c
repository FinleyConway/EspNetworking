#include "motor.h"

void init_motor(gpio_num_t a1, gpio_num_t a2) {
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
}

void set_motor_direction(motor_t* motor, motor_direction_t direction) {
    if (motor == NULL) return;

    if (direction == motor_direction_clockwise) {
        ledc_set_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_0, motor->current_speed);
        ledc_set_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_1, 0);
    }
    else if (direction == motor_direction_anti_clockwise) {
        ledc_set_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_0, 0);
        ledc_set_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_1, motor->current_speed);
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

void set_motor_speed(motor_t* motor, uint32_t speed) {
    if (motor == NULL) return;

    motor->current_speed = speed;

    set_motor_direction(motor, motor->current_direction);
}