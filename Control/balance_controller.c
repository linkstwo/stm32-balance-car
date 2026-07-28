#include <stdbool.h>
#include <stdint.h>
#include "app_config.h"
#include "balance_controller.h"
#include "filters.h"
#include "pid.h"

static const BalanceControlConfig k_safe_bringup_config =
{
    -8.0f, -80.0f, -1.0f, 0.004f, 0.001f, 1000.0f, 2.0f,
    3.0f, 300.0f, 600
};

static const BalanceControlConfig k_cascade_v1_config =
{
    -8.0f, -100.0f, -1.5f, 0.008f, 0.002f, 1500.0f, 3.0f,
    4.0f, 500.0f, 2200
};

static float BalanceController_Ramp(float current, float target, float max_rate, float dt_s)
{
    float max_step = max_rate * dt_s;
    float delta = target - current;

    if (delta > max_step)
    {
        return current + max_step;
    }
    if (delta < -max_step)
    {
        return current - max_step;
    }
    return target;
}

void BalanceController_Init(BalanceController *controller, ControlProfile profile)
{
    if (profile == CONTROL_PROFILE_CASCADE_V1)
    {
        controller->config = &k_cascade_v1_config;
    }
    else
    {
        controller->config = &k_safe_bringup_config;
    }
    controller->speed_pi.kp = controller->config->speed_kp;
    controller->speed_pi.ki = controller->config->speed_ki;
    controller->speed_pi.integral_limit = controller->config->speed_integral_limit;
    controller->speed_pi.output_limit = controller->config->speed_output_angle_limit_deg;
    BalanceController_Reset(controller);
}

void BalanceController_Reset(BalanceController *controller)
{
    PiController_Reset(&controller->speed_pi);
    controller->speed_filter.initialized = false;
    controller->ramped_speed_counts_per_s = 0.0f;
    controller->target_pitch_deg = controller->config->balance_angle_deg;
    controller->speed_loop_elapsed_s = 0.0f;
    controller->telemetry.measured_speed_counts_per_s = 0.0f;
    controller->telemetry.target_speed_counts_per_s = 0.0f;
    controller->telemetry.pitch_target_deg = controller->config->balance_angle_deg;
    controller->telemetry.balance_output = 0.0f;
    controller->telemetry.turn_output = 0.0f;
}

MotorMixOutput BalanceController_Update(BalanceController *controller,
                                        const BalanceControllerImuInput *imu,
                                        float left_counts_per_s,
                                        float right_counts_per_s,
                                        const BalanceControllerCommand *command,
                                        float dt_s)
{
    MotorMixOutput output;
    float measured_speed;
    float angle_error;
    float balance_output;
    float turn_output;
    const float speed_loop_period_s = (float)APP_SPEED_LOOP_PERIOD_MS / 1000.0f;

    if ((dt_s <= 0.0f) || (dt_s > 0.050f))
    {
        output.left = 0;
        output.right = 0;
        return output;
    }

    controller->ramped_speed_counts_per_s = BalanceController_Ramp(
        controller->ramped_speed_counts_per_s, command->target_speed_counts_per_s,
        controller->config->command_accel_limit_counts_per_s2, dt_s);
    measured_speed = FirstOrderFilter_Update(&controller->speed_filter,
        0.5f * (left_counts_per_s + right_counts_per_s), 5.0f, dt_s);

    controller->speed_loop_elapsed_s += dt_s;
    if ((controller->speed_loop_elapsed_s + 0.000001f) >= speed_loop_period_s)
    {
        /* The outer speed loop owns the target consumed by the inner balance loop. */
        controller->target_pitch_deg = controller->config->balance_angle_deg +
            PiController_Update(&controller->speed_pi,
                controller->ramped_speed_counts_per_s - measured_speed,
                controller->speed_loop_elapsed_s);
        controller->speed_loop_elapsed_s -= speed_loop_period_s;
    }

    angle_error = imu->pitch_deg - controller->target_pitch_deg;
    balance_output = controller->config->angle_kp * angle_error +
                     controller->config->angle_kd * imu->gyro_y_dps;
    turn_output = controller->config->yaw_rate_kp *
                  (command->target_yaw_rate_dps - imu->gyro_z_dps);

    controller->telemetry.measured_speed_counts_per_s = measured_speed;
    controller->telemetry.target_speed_counts_per_s = controller->ramped_speed_counts_per_s;
    controller->telemetry.pitch_target_deg = controller->target_pitch_deg;
    controller->telemetry.balance_output = balance_output;
    controller->telemetry.turn_output = turn_output;
    return MotorMixer_Mix(balance_output, turn_output, controller->config->pwm_limit);
}

const BalanceControlConfig *BalanceController_GetConfig(const BalanceController *controller)
{
    return controller->config;
}

BalanceControllerTelemetry BalanceController_GetTelemetry(const BalanceController *controller)
{
    return controller->telemetry;
}
