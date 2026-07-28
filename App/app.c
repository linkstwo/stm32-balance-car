#include <stdbool.h>
#include <stdint.h>
#include "stm32f10x.h"
#include "app_config.h"
#include "board_config.h"
#include "control_config.h"
#include "board.h"
#include "timebase.h"
#include "encoder.h"
#include "uart3.h"
#include "adc_battery.h"
#include "tb6612.h"
#include "imu.h"
#include "ssd1306.h"
#include "balance_controller.h"
#include "legacy_controller.h"
#include "app_state.h"
#include "remote_control.h"
#include "app.h"

typedef struct
{
    AppStateMachine state_machine;
    RemoteControl remote;
    BalanceController controller;
    LegacyController legacy_controller;
    ImuSample latest_sample;
    uint32_t last_control_ms;
    uint32_t last_display_ms;
    uint32_t last_battery_ms;
    uint32_t control_overrun_count;
    uint32_t max_control_period_ms;
    uint16_t battery_raw;
    uint8_t display_page;
    bool imu_ready;
    bool has_sample;
    bool display_healthy;
} AppContext;

static AppContext g_app;

static void App_ConfigureInterruptPriorities(void)
{
    NVIC_InitTypeDef nvic;

    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannelPreemptionPriority = 0U;
    nvic.NVIC_IRQChannelSubPriority = 0U;
    nvic.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_Init(&nvic);

    nvic.NVIC_IRQChannelPreemptionPriority = 1U;
    nvic.NVIC_IRQChannelSubPriority = 0U;
    nvic.NVIC_IRQChannel = USART3_IRQn;
    NVIC_Init(&nvic);

    nvic.NVIC_IRQChannelPreemptionPriority = 2U;
    nvic.NVIC_IRQChannelSubPriority = 0U;
    nvic.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_Init(&nvic);
}

static void App_RunControl(const ImuSample *sample)
{
    EncoderDeltaCounts delta;
    BalanceControllerImuInput imu_input;
    BalanceControllerCommand command;
    MotorMixOutput motor_output;
    RemoteCommand remote_command;
    uint32_t period_ms;
    float dt_s;

    period_ms = (g_app.last_control_ms == 0U) ? APP_CONTROL_PERIOD_MS :
                (uint32_t)(sample->timestamp_ms - g_app.last_control_ms);
    g_app.last_control_ms = sample->timestamp_ms;
    if (period_ms > g_app.max_control_period_ms)
    {
        g_app.max_control_period_ms = period_ms;
    }
    if ((period_ms < 5U) || (period_ms > 20U))
    {
        g_app.control_overrun_count++;
        AppState_OnControlTimingFailure(&g_app.state_machine);
        MotorDriver_Disable();
        return;
    }

    remote_command = RemoteControl_GetCommand(&g_app.remote, sample->timestamp_ms);
    AppState_OnImuSample(&g_app.state_machine, sample->pitch_deg, sample->timestamp_ms,
                         RemoteControl_TakeNeutralEvent(&g_app.remote),
                         RemoteControl_IsTimedOut(&g_app.remote, sample->timestamp_ms));
    if (!AppState_IsMotorAuthorized(&g_app.state_machine))
    {
        BalanceController_Reset(&g_app.controller);
        MotorDriver_Disable();
        return;
    }

    delta = Encoder_ReadDeltaCounts();
    dt_s = (float)period_ms / 1000.0f;
    imu_input.pitch_deg = sample->pitch_deg;
    imu_input.gyro_y_dps = sample->gyro_y_dps;
    imu_input.gyro_z_dps = sample->gyro_z_dps;
    command.target_speed_counts_per_s = remote_command.target_speed_counts_per_s;
    command.target_yaw_rate_dps = remote_command.target_yaw_rate_dps;

#if CONTROL_USE_LEGACY
    {
        LegacyControllerInput legacy_input;
        legacy_input.pitch_deg = sample->pitch_deg;
        legacy_input.gyro_y_raw = (int16_t)sample->gyro_y_dps;
        legacy_input.gyro_z_raw = (int16_t)sample->gyro_z_dps;
        legacy_input.encoder_sum_delta = (int16_t)(delta.left_delta_counts + delta.right_delta_counts);
        legacy_input.motion_direction = g_app.remote.motion_direction;
        legacy_input.turn_direction = g_app.remote.turn_direction;
        motor_output = LegacyController_Update(&g_app.legacy_controller, &legacy_input,
                                               MotorDriver_GetOutputLimit());
    }
#else
    motor_output = BalanceController_Update(&g_app.controller, &imu_input,
        Encoder_CountsPerSecond(delta.left_delta_counts, period_ms),
        Encoder_CountsPerSecond(delta.right_delta_counts, period_ms), &command, dt_s);
#endif
    MotorDriver_Set(motor_output.left, motor_output.right);
}

static void App_DrawStatus(void)
{
    BalanceControllerTelemetry telemetry = BalanceController_GetTelemetry(&g_app.controller);

    Oled_Clear();
    Oled_DrawText(0U, 0U, AppState_ToString(g_app.state_machine.state));
    Oled_DrawText(0U, 1U, "P:");
    Oled_DrawFloat(18U, 1U, g_app.has_sample ? g_app.latest_sample.pitch_deg : 0.0f, 1U);
    Oled_DrawText(0U, 2U, "SPD:");
    Oled_DrawInt(24U, 2U, (int32_t)telemetry.measured_speed_counts_per_s);
    Oled_DrawText(0U, 3U, "BAT:");
    Oled_DrawInt(24U, 3U, g_app.battery_raw);
    Oled_DrawText(0U, 4U, "OVR:");
    Oled_DrawInt(24U, 4U, (int32_t)g_app.control_overrun_count);
    Oled_DrawText(0U, 5U, "I2C:");
    Oled_DrawInt(24U, 5U, (int32_t)Imu_GetDroppedEventCount());
}

void App_Init(void)
{
    ImuResult imu_result;
    uint32_t now_ms;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    Board_Init();
    Timebase_Init();
    MotorDriver_Init();
    MotorDriver_Disable();
    Encoder_Init();
    Uart3_Init(BOARD_USART3_BAUD);
    AdcBattery_Init();

    now_ms = Timebase_GetMs();
    AppState_Init(&g_app.state_machine);
    RemoteControl_Init(&g_app.remote, now_ms);
    BalanceController_Init(&g_app.controller, CONTROL_DEFAULT_PROFILE);
    LegacyController_Init(&g_app.legacy_controller);
    g_app.display_healthy = Oled_Init();

    imu_result = Imu_Init();
    g_app.imu_ready = (imu_result == IMU_OK);
    AppState_SetImuInitResult(&g_app.state_machine, g_app.imu_ready);
    if (g_app.imu_ready)
    {
        Imu_EnableDataReadyInterrupt();
    }
    App_ConfigureInterruptPriorities();
    MotorDriver_Disable();
}

void App_ProcessHighestPriorityEvents(void)
{
    ImuResult result;
    uint8_t processed = 0U;

    if (!g_app.imu_ready)
    {
        MotorDriver_Disable();
        return;
    }
    while (Imu_HasPendingSample() && (processed < 4U))
    {
        result = Imu_ReadLatest(&g_app.latest_sample);
        if (result != IMU_OK)
        {
            AppState_OnImuReadFailure(&g_app.state_machine);
            if (!AppState_IsMotorAuthorized(&g_app.state_machine))
            {
                MotorDriver_Disable();
            }
            return;
        }
        g_app.has_sample = true;
        App_RunControl(&g_app.latest_sample);
        processed++;
    }
}

void App_ProcessRemoteInput(void)
{
    uint8_t byte;
    uint32_t now_ms = Timebase_GetMs();

    while (Uart3_ReadByte(&byte))
    {
        RemoteControl_ProcessByte(&g_app.remote, byte, now_ms);
    }
}

void App_ProcessStateMachine(void)
{
    uint32_t now_ms = Timebase_GetMs();

    if (g_app.imu_ready &&
        ((uint32_t)(now_ms - Imu_GetLastEventMs()) >= APP_IMU_STALE_TIMEOUT_MS))
    {
        AppState_OnImuStale(&g_app.state_machine);
        MotorDriver_Disable();
    }
    if (!AppState_IsMotorAuthorized(&g_app.state_machine))
    {
        MotorDriver_Disable();
    }
    Board_SetStatusLed(AppState_IsMotorAuthorized(&g_app.state_machine));
}

void App_ProcessDisplay(void)
{
    uint32_t now_ms = Timebase_GetMs();

    if (!g_app.display_healthy)
    {
        return;
    }
    if ((uint32_t)(now_ms - g_app.last_battery_ms) >= APP_BATTERY_PERIOD_MS)
    {
        AdcBattery_ReadRaw(&g_app.battery_raw);
        g_app.last_battery_ms = now_ms;
    }
    if (g_app.display_page < 8U)
    {
        if (!Oled_FlushPage(g_app.display_page++))
        {
            g_app.display_healthy = false;
        }
        return;
    }
    if ((uint32_t)(now_ms - g_app.last_display_ms) >= APP_DISPLAY_PERIOD_MS)
    {
        App_DrawStatus();
        g_app.display_page = 0U;
        g_app.last_display_ms = now_ms;
    }
}

void App_Idle(void)
{
    __WFI();
}
