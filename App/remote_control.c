#include <stdbool.h>
#include <stdint.h>
#include "app_config.h"
#include "remote_control.h"

static float RemoteControl_SpeedForLevel(uint8_t level)
{
    /* Internal level is intuitive: higher level means higher speed. */
    uint8_t legacy_divisor = (uint8_t)(11U - level);
    return APP_REMOTE_MAX_SPEED_COUNTS_PER_S / (float)legacy_divisor;
}

void RemoteControl_Init(RemoteControl *remote, uint32_t now_ms)
{
    remote->speed_level = 6U;
    remote->motion_direction = 0;
    remote->turn_direction = 0;
    remote->last_valid_command_ms = now_ms;
    remote->neutral_event = false;
}

bool RemoteControl_ProcessByte(RemoteControl *remote, uint8_t byte, uint32_t now_ms)
{
    bool valid = true;

    switch (byte)
    {
        case 'Z':
            remote->motion_direction = 0;
            remote->turn_direction = 0;
            remote->neutral_event = true;
            break;
        case 'E':
            remote->motion_direction = 1;
            break;
        case 'A':
            remote->motion_direction = -1;
            break;
        case 'C':
        case 'G':
            remote->turn_direction = -1;
            break;
        case 'B':
        case 'H':
            remote->turn_direction = 1;
            break;
        case 'X':
            /* Preserve old phone behavior: old X increased a speed divisor. */
            if (remote->speed_level > 1U)
            {
                remote->speed_level--;
            }
            break;
        case 'Y':
            /* Preserve old phone behavior: old Y decreased a speed divisor. */
            if (remote->speed_level < 10U)
            {
                remote->speed_level++;
            }
            break;
        default:
            valid = false;
            break;
    }

    if (valid)
    {
        remote->last_valid_command_ms = now_ms;
    }
    return valid;
}

RemoteCommand RemoteControl_GetCommand(const RemoteControl *remote, uint32_t now_ms)
{
    RemoteCommand command;

    command.target_speed_counts_per_s = 0.0f;
    command.target_yaw_rate_dps = 0.0f;
    command.neutral_requested = false;
    if (RemoteControl_IsTimedOut(remote, now_ms))
    {
        return command;
    }

    command.target_speed_counts_per_s = (float)remote->motion_direction *
                                         RemoteControl_SpeedForLevel(remote->speed_level);
    command.target_yaw_rate_dps = (float)remote->turn_direction * APP_REMOTE_MAX_YAW_RATE_DPS;
    return command;
}

bool RemoteControl_TakeNeutralEvent(RemoteControl *remote)
{
    bool received = remote->neutral_event;
    remote->neutral_event = false;
    return received;
}

bool RemoteControl_IsTimedOut(const RemoteControl *remote, uint32_t now_ms)
{
    return (uint32_t)(now_ms - remote->last_valid_command_ms) >= APP_REMOTE_TIMEOUT_MS;
}
