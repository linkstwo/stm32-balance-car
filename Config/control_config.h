#ifndef CONTROL_CONFIG_H
#define CONTROL_CONFIG_H

#include <stdint.h>

typedef enum
{
    CONTROL_PROFILE_LEGACY = 0,
    CONTROL_PROFILE_SAFE_BRINGUP,
    CONTROL_PROFILE_CASCADE_V1
} ControlProfile;

typedef struct
{
    float balance_angle_deg;
    float angle_kp;
    float angle_kd;
    float speed_kp;
    float speed_ki;
    float speed_integral_limit;
    float speed_output_angle_limit_deg;
    float yaw_rate_kp;
    float yaw_rate_kd;
    float command_accel_limit_counts_per_s2;
    int16_t pwm_limit;
} BalanceControlConfig;

/* Default gains are UNVERIFIED_ON_HARDWARE. Start with wheels raised. */
#define CONTROL_DEFAULT_PROFILE CONTROL_PROFILE_SAFE_BRINGUP
#define CONTROL_USE_LEGACY 0

#endif
