#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "app_config.h"
#include "app_state.h"
#include "balance_controller.h"
#include "motor_mixer.h"
#include "pid.h"
#include "remote_control.h"

static void TestPiZeroErrorDoesNotDrift(void)
{
    PiController controller = { 1.0f, 2.0f, 0.0f, 10.0f, 100.0f };
    PiController_Update(&controller, 0.0f, 0.01f);
    assert(fabsf(controller.integral) < 0.0001f);
}

static void TestPiAntiWindup(void)
{
    PiController controller = { 5.0f, 10.0f, 0.0f, 2.0f, 1.0f };
    unsigned int index;

    for (index = 0U; index < 100U; ++index)
    {
        assert(PiController_Update(&controller, 10.0f, 0.01f) <= 1.0f);
    }
    assert(controller.integral <= 2.0f);
}

static void TestRemoteTimeoutRampsCommand(void)
{
    BalanceController controller;
    BalanceControllerImuInput imu = { -8.0f, 0.0f, 0.0f };
    BalanceControllerCommand command = { 120.0f, 0.0f };
    BalanceControllerTelemetry before_timeout;
    BalanceControllerTelemetry after_timeout;
    unsigned int index;

    BalanceController_Init(&controller, CONTROL_PROFILE_SAFE_BRINGUP);
    for (index = 0U; index < 10U; ++index)
    {
        BalanceController_Update(&controller, &imu, 0.0f, 0.0f, &command, 0.01f);
    }
    before_timeout = BalanceController_GetTelemetry(&controller);
    command.target_speed_counts_per_s = 0.0f;
    BalanceController_Update(&controller, &imu, 0.0f, 0.0f, &command, 0.01f);
    after_timeout = BalanceController_GetTelemetry(&controller);
    assert(before_timeout.target_speed_counts_per_s > after_timeout.target_speed_counts_per_s);
    assert(after_timeout.target_speed_counts_per_s > 0.0f);
}

static void TestFaultAndFallenState(void)
{
    AppStateMachine state;

    AppState_Init(&state);
    state.state = APP_STATE_ARMED;
    AppState_OnImuReadFailure(&state);
    assert(!AppState_IsMotorAuthorized(&state));

    state.state = APP_STATE_FALLEN;
    AppState_OnImuSample(&state, APP_BALANCE_REFERENCE_PITCH_DEG, 100U, false, false);
    assert(state.state == APP_STATE_FALLEN);
    AppState_OnImuSample(&state, APP_BALANCE_REFERENCE_PITCH_DEG, 110U, true, false);
    assert(state.state == APP_STATE_DISARMED);
}

static void TestMixerLimit(void)
{
    MotorMixOutput output = MotorMixer_Mix(900.0f, 900.0f, 600);

    assert(output.left >= -600 && output.left <= 600);
    assert(output.right >= -600 && output.right <= 600);
}

static void TestTimeoutWraparound(void)
{
    RemoteControl remote;

    RemoteControl_Init(&remote, 0xFFFFFFF0U);
    assert(!RemoteControl_IsTimedOut(&remote, 0x00000010U));
    assert(RemoteControl_IsTimedOut(&remote, 0x00000130U));
}

int main(void)
{
    TestPiZeroErrorDoesNotDrift();
    TestPiAntiWindup();
    TestRemoteTimeoutRampsCommand();
    TestFaultAndFallenState();
    TestMixerLimit();
    TestTimeoutWraparound();
    puts("host control tests passed");
    return 0;
}
