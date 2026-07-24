#ifndef DRIVERS_IMU_IMU_H
#define DRIVERS_IMU_IMU_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    IMU_OK = 0,
    IMU_ERROR_INIT,
    IMU_ERROR_SENSOR_CONFIG,
    IMU_ERROR_FIFO_CONFIG,
    IMU_ERROR_DMP_LOAD,
    IMU_ERROR_ORIENTATION,
    IMU_ERROR_FEATURE_CONFIG,
    IMU_ERROR_RATE_CONFIG,
    IMU_ERROR_SELF_TEST,
    IMU_ERROR_DMP_ENABLE,
    IMU_ERROR_FIFO_READ,
    IMU_ERROR_NO_QUATERNION
} ImuResult;

typedef struct
{
    float pitch_deg;
    float roll_deg;
    float yaw_deg;
    float gyro_x_dps;
    float gyro_y_dps;
    float gyro_z_dps;
    uint32_t timestamp_ms;
    uint32_t sequence;
} ImuSample;

ImuResult Imu_Init(void);
void Imu_EnableDataReadyInterrupt(void);
void Imu_NotifyDataReadyFromIsr(uint32_t timestamp_ms);
bool Imu_HasPendingSample(void);
ImuResult Imu_ReadLatest(ImuSample *sample);
uint32_t Imu_GetLastEventMs(void);
uint32_t Imu_GetDroppedEventCount(void);

#endif
