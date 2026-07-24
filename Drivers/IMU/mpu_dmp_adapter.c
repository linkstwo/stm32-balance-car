#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include "stm32f10x.h"
#include "board_config.h"
#include "timebase.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "inv_mpu_port.h"
#include "imu.h"

#define IMU_DMP_RATE_HZ 100U
#define IMU_MAX_FIFO_DRAIN_PACKETS 8U
#define IMU_Q30_SCALE 1073741824.0f
#define IMU_RAD_TO_DEG 57.295779513f

static volatile uint8_t g_pending_count;
static volatile uint32_t g_last_event_ms;
static volatile uint32_t g_dropped_events;
static uint32_t g_sequence;

static unsigned short Imu_OrientationScalar(const signed char *matrix)
{
    unsigned short scalar = 0U;
    uint8_t row;

    for (row = 0U; row < 3U; ++row)
    {
        const signed char *value = &matrix[row * 3U];
        unsigned short scale = 7U;
        if (value[0] > 0) { scale = 0U; }
        else if (value[0] < 0) { scale = 4U; }
        else if (value[1] > 0) { scale = 1U; }
        else if (value[1] < 0) { scale = 5U; }
        else if (value[2] > 0) { scale = 2U; }
        else if (value[2] < 0) { scale = 6U; }
        scalar |= (unsigned short)(scale << (row * 3U));
    }
    return scalar;
}

static ImuResult Imu_ApplySelfTest(void)
{
    long gyro_bias[3];
    long accel_bias[3];
    float gyro_sensitivity;
    unsigned short accel_sensitivity;
    uint8_t index;

    if (mpu_run_self_test(gyro_bias, accel_bias) != 0x7)
    {
        return IMU_ERROR_SELF_TEST;
    }
    if ((mpu_get_gyro_sens(&gyro_sensitivity) != 0) ||
        (mpu_get_accel_sens(&accel_sensitivity) != 0))
    {
        return IMU_ERROR_SELF_TEST;
    }
    for (index = 0U; index < 3U; ++index)
    {
        gyro_bias[index] = (long)(gyro_bias[index] * gyro_sensitivity);
        accel_bias[index] *= accel_sensitivity;
    }
    if ((dmp_set_gyro_bias(gyro_bias) != 0) || (dmp_set_accel_bias(accel_bias) != 0))
    {
        return IMU_ERROR_SELF_TEST;
    }
    return IMU_OK;
}

ImuResult Imu_Init(void)
{
    static const signed char orientation_matrix[9] = BOARD_IMU_ORIENTATION_MATRIX;
    ImuResult result;

    InvMpuPort_Init();
    if (mpu_init() != 0)
    {
        return IMU_ERROR_INIT;
    }
    if (mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL) != 0)
    {
        return IMU_ERROR_SENSOR_CONFIG;
    }
    if (mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL) != 0)
    {
        return IMU_ERROR_FIFO_CONFIG;
    }
    if (mpu_set_sample_rate(IMU_DMP_RATE_HZ) != 0)
    {
        return IMU_ERROR_RATE_CONFIG;
    }
    if (dmp_load_motion_driver_firmware() != 0)
    {
        return IMU_ERROR_DMP_LOAD;
    }
    if (dmp_set_orientation(Imu_OrientationScalar(orientation_matrix)) != 0)
    {
        return IMU_ERROR_ORIENTATION;
    }
    if (dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |
                           DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL |
                           DMP_FEATURE_SEND_CAL_GYRO | DMP_FEATURE_GYRO_CAL) != 0)
    {
        return IMU_ERROR_FEATURE_CONFIG;
    }
    if (dmp_set_fifo_rate(IMU_DMP_RATE_HZ) != 0)
    {
        return IMU_ERROR_RATE_CONFIG;
    }
    result = Imu_ApplySelfTest();
    if (result != IMU_OK)
    {
        return result;
    }
    if ((mpu_set_int_level(1U) != 0) || (mpu_set_dmp_state(1U) != 0))
    {
        return IMU_ERROR_DMP_ENABLE;
    }
    g_pending_count = 0U;
    g_last_event_ms = Timebase_GetMs();
    g_dropped_events = 0U;
    g_sequence = 0U;
    return IMU_OK;
}

void Imu_EnableDataReadyInterrupt(void)
{
    GPIO_InitTypeDef gpio;
    EXTI_InitTypeDef exti;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_5;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &gpio);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);
    exti.EXTI_Line = EXTI_Line5;
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    exti.EXTI_Trigger = EXTI_Trigger_Falling;
    exti.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti);
}

void Imu_NotifyDataReadyFromIsr(uint32_t timestamp_ms)
{
    if (g_pending_count < 255U)
    {
        g_pending_count++;
    }
    else
    {
        g_dropped_events++;
    }
    g_last_event_ms = timestamp_ms;
}

bool Imu_HasPendingSample(void)
{
    return g_pending_count != 0U;
}

ImuResult Imu_ReadLatest(ImuSample *sample)
{
    short gyro[3];
    short accel[3];
    long quaternion[4];
    unsigned long sensor_timestamp;
    short sensors;
    unsigned char more;
    float q0;
    float q1;
    float q2;
    float q3;
    float gyro_sensitivity;
    uint8_t read_count = 0U;
    uint32_t primask = __get_PRIMASK();

    __disable_irq();
    if (g_pending_count > 0U)
    {
        g_pending_count--;
    }
    if (primask == 0U)
    {
        __enable_irq();
    }

    do
    {
        if (dmp_read_fifo(gyro, accel, quaternion, &sensor_timestamp, &sensors, &more) != 0)
        {
            return IMU_ERROR_FIFO_READ;
        }
        read_count++;
    } while ((more != 0U) && (read_count < IMU_MAX_FIFO_DRAIN_PACKETS));

    if ((sensors & INV_WXYZ_QUAT) == 0)
    {
        return IMU_ERROR_NO_QUATERNION;
    }
    if (mpu_get_gyro_sens(&gyro_sensitivity) != 0)
    {
        return IMU_ERROR_FIFO_READ;
    }

    q0 = (float)quaternion[0] / IMU_Q30_SCALE;
    q1 = (float)quaternion[1] / IMU_Q30_SCALE;
    q2 = (float)quaternion[2] / IMU_Q30_SCALE;
    q3 = (float)quaternion[3] / IMU_Q30_SCALE;
    sample->pitch_deg = BOARD_IMU_PITCH_SIGN * asinf(-2.0f * q1 * q3 + 2.0f * q0 * q2) * IMU_RAD_TO_DEG;
    sample->roll_deg = BOARD_IMU_ROLL_SIGN * atan2f(2.0f * q2 * q3 + 2.0f * q0 * q1,
                                                      1.0f - 2.0f * q1 * q1 - 2.0f * q2 * q2) * IMU_RAD_TO_DEG;
    sample->yaw_deg = BOARD_IMU_YAW_SIGN * atan2f(2.0f * (q1 * q2 + q0 * q3),
                                                    q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) * IMU_RAD_TO_DEG;
    sample->gyro_x_dps = BOARD_IMU_GYRO_X_SIGN * ((float)gyro[0] / gyro_sensitivity);
    sample->gyro_y_dps = BOARD_IMU_GYRO_Y_SIGN * ((float)gyro[1] / gyro_sensitivity);
    sample->gyro_z_dps = BOARD_IMU_GYRO_Z_SIGN * ((float)gyro[2] / gyro_sensitivity);
    sample->timestamp_ms = Timebase_GetMs();
    sample->sequence = ++g_sequence;
    return IMU_OK;
}

uint32_t Imu_GetLastEventMs(void)
{
    return g_last_event_ms;
}

uint32_t Imu_GetDroppedEventCount(void)
{
    return g_dropped_events;
}
