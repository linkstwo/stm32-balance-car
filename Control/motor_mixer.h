#ifndef CONTROL_MOTOR_MIXER_H
#define CONTROL_MOTOR_MIXER_H

#include <stdint.h>

typedef struct
{
    int16_t left;
    int16_t right;
} MotorMixOutput;

MotorMixOutput MotorMixer_Mix(float balance_output, float turn_output, int16_t pwm_limit);

#endif
