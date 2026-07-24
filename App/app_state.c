#include <stdbool.h>
#include <stdint.h>
#include "app_config.h"
#include "app_state.h"

static float AppState_Absolute(float value)
{
    return (value < 0.0f) ? -value : value;
}

void AppState_Init(AppStateMachine *machine)
{
    machine->state = APP_STATE_BOOT;
    machine->stable_sample_count = 0U;
    machine->imu_failure_count = 0U;
    machine->fall_start_ms = 0U;
    machine->fall_timer_active = false;
}

void AppState_SetImuInitResult(AppStateMachine *machine, bool initialized)
{
    machine->state = initialized ? APP_STATE_DISARMED : APP_STATE_IMU_FAULT;
}

void AppState_OnImuSample(AppStateMachine *machine, float pitch_deg, uint32_t now_ms,
                          bool neutral_command_received, bool remote_timed_out)
{
    float absolute_pitch = AppState_Absolute(pitch_deg - APP_BALANCE_REFERENCE_PITCH_DEG);

    machine->imu_failure_count = 0U;
    if ((machine->state == APP_STATE_IMU_FAULT) || (machine->state == APP_STATE_LOW_BATTERY))
    {
        return;
    }

    if (absolute_pitch > APP_FALL_TRIP_ANGLE_DEG)
    {
        if (!machine->fall_timer_active)
        {
            machine->fall_start_ms = now_ms;
            machine->fall_timer_active = true;
        }
        else if ((uint32_t)(now_ms - machine->fall_start_ms) >= APP_FALL_TRIP_TIME_MS)
        {
            machine->state = APP_STATE_FALLEN;
        }
    }
    else
    {
        machine->fall_timer_active = false;
    }

    if (machine->state == APP_STATE_FALLEN)
    {
        if ((absolute_pitch <= APP_REARM_ANGLE_DEG) && neutral_command_received)
        {
            machine->state = APP_STATE_DISARMED;
            machine->stable_sample_count = 0U;
        }
        return;
    }

    if (machine->state == APP_STATE_DISARMED)
    {
        if (absolute_pitch <= APP_ARM_PITCH_WINDOW_DEG)
        {
            if (machine->stable_sample_count < APP_ARM_STABLE_SAMPLES)
            {
                machine->stable_sample_count++;
            }
        }
        else
        {
            machine->stable_sample_count = 0U;
        }
        if ((machine->stable_sample_count >= APP_ARM_STABLE_SAMPLES) && neutral_command_received)
        {
            machine->state = APP_STATE_ARMED;
        }
    }
    else if ((machine->state == APP_STATE_ARMED) && remote_timed_out)
    {
        machine->state = APP_STATE_REMOTE_LOST;
    }
    else if ((machine->state == APP_STATE_REMOTE_LOST) && !remote_timed_out)
    {
        machine->state = APP_STATE_ARMED;
    }
}

void AppState_OnImuReadFailure(AppStateMachine *machine)
{
    if (machine->imu_failure_count < APP_IMU_FAILURE_LIMIT)
    {
        machine->imu_failure_count++;
    }
    /* A FIFO read failure invalidates the current control sample; fail safe now. */
    machine->state = APP_STATE_IMU_FAULT;
}

void AppState_OnImuStale(AppStateMachine *machine)
{
    machine->state = APP_STATE_IMU_FAULT;
}

bool AppState_IsMotorAuthorized(const AppStateMachine *machine)
{
    return (machine->state == APP_STATE_ARMED) || (machine->state == APP_STATE_REMOTE_LOST);
}

const char *AppState_ToString(AppState state)
{
    switch (state)
    {
        case APP_STATE_BOOT: return "BOOT";
        case APP_STATE_SELF_TEST: return "SELFTEST";
        case APP_STATE_DISARMED: return "DISARM";
        case APP_STATE_ARMED: return "ARMED";
        case APP_STATE_REMOTE_LOST: return "REMOTE LOST";
        case APP_STATE_FALLEN: return "FALLEN";
        case APP_STATE_IMU_FAULT: return "IMU FAULT";
        case APP_STATE_LOW_BATTERY: return "LOW BAT";
        default: return "UNKNOWN";
    }
}
