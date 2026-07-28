#ifndef CONTROL_BALANCE_CONTROLLER_H
#define CONTROL_BALANCE_CONTROLLER_H

#include <stdint.h>
#include "control_config.h"
#include "filters.h"
#include "motor_mixer.h"
#include "pid.h"

typedef struct
{
    float pitch_deg;
    float gyro_y_dps;
    float gyro_z_dps;
} BalanceControllerImuInput;

typedef struct
{
    float target_speed_counts_per_s;
    float target_yaw_rate_dps;
} BalanceControllerCommand;

typedef struct
{
    float measured_speed_counts_per_s;
    float target_speed_counts_per_s;
    float pitch_target_deg;
    float balance_output;
    float turn_output;
} BalanceControllerTelemetry;

typedef struct
{
    const BalanceControlConfig *config;
    PiController speed_pi;
    FirstOrderFilter speed_filter;
    float ramped_speed_counts_per_s;
    float target_pitch_deg;
    float speed_loop_elapsed_s;
    BalanceControllerTelemetry telemetry;
} BalanceController;

void BalanceController_Init(BalanceController *controller, ControlProfile profile);
void BalanceController_Reset(BalanceController *controller);
MotorMixOutput BalanceController_Update(BalanceController *controller,
                                        const BalanceControllerImuInput *imu,
                                        float left_counts_per_s,
                                        float right_counts_per_s,
                                        const BalanceControllerCommand *command,
                                        float dt_s);
const BalanceControlConfig *BalanceController_GetConfig(const BalanceController *controller);
BalanceControllerTelemetry BalanceController_GetTelemetry(const BalanceController *controller);

#endif
