#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    APP_STATE_BOOT = 0,
    APP_STATE_SELF_TEST,
    APP_STATE_DISARMED,
    APP_STATE_ARMED,
    APP_STATE_REMOTE_LOST,
    APP_STATE_FALLEN,
    APP_STATE_IMU_FAULT,
    APP_STATE_LOW_BATTERY
} AppState;

typedef struct
{
    AppState state;
    uint16_t stable_sample_count;
    uint8_t imu_failure_count;
    uint32_t fall_start_ms;
    bool fall_timer_active;
    bool auto_arm_pending;
} AppStateMachine;

void AppState_Init(AppStateMachine *machine);
void AppState_SetImuInitResult(AppStateMachine *machine, bool initialized);
void AppState_OnImuSample(AppStateMachine *machine, float pitch_deg, uint32_t now_ms,
                          bool neutral_command_received, bool remote_timed_out);
void AppState_OnImuReadFailure(AppStateMachine *machine);
void AppState_OnImuStale(AppStateMachine *machine);
void AppState_OnControlTimingFailure(AppStateMachine *machine);
bool AppState_IsMotorAuthorized(const AppStateMachine *machine);
const char *AppState_ToString(AppState state);

#endif
