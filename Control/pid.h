#ifndef CONTROL_PID_H
#define CONTROL_PID_H

#include <stdbool.h>

typedef struct
{
    float kp;
    float ki;
    float integral;
    float integral_limit;
    float output_limit;
} PiController;

void PiController_Reset(PiController *controller);
float PiController_Update(PiController *controller, float error, float dt_s);

#endif
