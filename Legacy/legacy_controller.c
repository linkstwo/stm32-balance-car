#include <stdint.h>
#include "legacy_controller.h"

/* Reference-only implementation of the pre-refactor controller. Units are legacy raw units. */
#define LEGACY_BALANCE_ANGLE_DEG (-8.0f)
#define LEGACY_VERTICAL_KP (-330.0f)
#define LEGACY_VERTICAL_KD (-2.2f)
#define LEGACY_VELOCITY_KP (120.0f)
#define LEGACY_VELOCITY_KI (0.6f)
#define LEGACY_TURN_KP (40.0f)
#define LEGACY_TURN_KD (-0.8f)

void LegacyController_Init(LegacyController *controller)
{
    controller->encoder_integral = 0;
}

MotorMixOutput LegacyController_Update(LegacyController *controller,
                                       const LegacyControllerInput *input,
                                       int16_t pwm_limit)
{
    float vertical;
    float velocity;
    float turn_target;
    float turn;
    int32_t base;
    MotorMixOutput output;

    controller->encoder_integral += -input->encoder_sum_delta +
        (int32_t)input->motion_direction * 60;
    if (controller->encoder_integral > 10000) { controller->encoder_integral = 10000; }
    if (controller->encoder_integral < -10000) { controller->encoder_integral = -10000; }
    vertical = LEGACY_VERTICAL_KP * (input->pitch_deg - LEGACY_BALANCE_ANGLE_DEG) +
               LEGACY_VERTICAL_KD * input->gyro_y_raw;
    velocity = LEGACY_VELOCITY_KP * (-input->encoder_sum_delta) +
               LEGACY_VELOCITY_KI * controller->encoder_integral;
    turn_target = (float)input->turn_direction * 15.0f;
    turn = turn_target * LEGACY_TURN_KP + (float)input->gyro_z_raw * LEGACY_TURN_KD;
    base = (int32_t)(vertical + velocity);
    output.left = (int16_t)(base - (int32_t)turn);
    output.right = (int16_t)(base + (int32_t)turn);
    if (output.left > pwm_limit) { output.left = pwm_limit; }
    if (output.left < -pwm_limit) { output.left = (int16_t)-pwm_limit; }
    if (output.right > pwm_limit) { output.right = pwm_limit; }
    if (output.right < -pwm_limit) { output.right = (int16_t)-pwm_limit; }
    return output;
}
