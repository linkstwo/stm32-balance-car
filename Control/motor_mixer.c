#include <stdint.h>
#include "motor_mixer.h"

static float MotorMixer_Clamp(float value, float limit)
{
    if (value > limit)
    {
        return limit;
    }
    if (value < -limit)
    {
        return -limit;
    }
    return value;
}

MotorMixOutput MotorMixer_Mix(float balance_output, float turn_output, int16_t pwm_limit)
{
    MotorMixOutput output;
    float limit = (float)pwm_limit;
    float base = MotorMixer_Clamp(balance_output, limit);
    float turn_limit = limit - ((base < 0.0f) ? -base : base);
    float turn = MotorMixer_Clamp(turn_output, turn_limit);

    output.left = (int16_t)MotorMixer_Clamp(base - turn, limit);
    output.right = (int16_t)MotorMixer_Clamp(base + turn, limit);
    return output;
}
