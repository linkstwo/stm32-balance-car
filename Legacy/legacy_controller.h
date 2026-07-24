#ifndef LEGACY_CONTROLLER_H
#define LEGACY_CONTROLLER_H

#include <stdint.h>
#include "motor_mixer.h"

typedef struct
{
    float pitch_deg;
    int16_t gyro_y_raw;
    int16_t gyro_z_raw;
    int16_t encoder_sum_delta;
    int8_t motion_direction;
    int8_t turn_direction;
} LegacyControllerInput;

typedef struct
{
    int32_t encoder_integral;
} LegacyController;

void LegacyController_Init(LegacyController *controller);
MotorMixOutput LegacyController_Update(LegacyController *controller,
                                       const LegacyControllerInput *input,
                                       int16_t pwm_limit);

#endif
