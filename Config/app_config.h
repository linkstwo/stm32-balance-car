#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>

#define APP_CONTROL_PERIOD_MS              10U
#define APP_SPEED_LOOP_PERIOD_MS           50U
#define APP_DISPLAY_PERIOD_MS              200U
#define APP_BATTERY_PERIOD_MS              1000U
#define APP_REMOTE_TIMEOUT_MS              300U
#define APP_IMU_STALE_TIMEOUT_MS           40U
#define APP_IMU_FAILURE_LIMIT              3U
#define APP_ARM_STABLE_SAMPLES             50U
#define APP_BALANCE_REFERENCE_PITCH_DEG     -8.0f
#define APP_ARM_PITCH_WINDOW_DEG           3.0f
#define APP_FALL_TRIP_ANGLE_DEG            35.0f
#define APP_REARM_ANGLE_DEG                10.0f
#define APP_FALL_TRIP_TIME_MS              100U

/* Values below need hardware calibration before operational use. */
#define APP_REMOTE_MAX_SPEED_COUNTS_PER_S  300.0f
#define APP_REMOTE_MAX_YAW_RATE_DPS        30.0f

#endif
