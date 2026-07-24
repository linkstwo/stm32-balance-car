#ifndef APP_REMOTE_CONTROL_H
#define APP_REMOTE_CONTROL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    float target_speed_counts_per_s;
    float target_yaw_rate_dps;
    bool neutral_requested;
} RemoteCommand;

typedef struct
{
    uint8_t speed_level;
    int8_t motion_direction;
    int8_t turn_direction;
    uint32_t last_valid_command_ms;
    bool neutral_event;
} RemoteControl;

void RemoteControl_Init(RemoteControl *remote, uint32_t now_ms);
bool RemoteControl_ProcessByte(RemoteControl *remote, uint8_t byte, uint32_t now_ms);
RemoteCommand RemoteControl_GetCommand(const RemoteControl *remote, uint32_t now_ms);
bool RemoteControl_TakeNeutralEvent(RemoteControl *remote);
bool RemoteControl_IsTimedOut(const RemoteControl *remote, uint32_t now_ms);

#endif
