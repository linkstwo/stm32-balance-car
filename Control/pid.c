#include "pid.h"

static float PiController_Clamp(float value, float limit)
{
    if (value > limit)
    {
        return limit;
    }
    if (value < -limit)
    {
        return -limit;
    }
    return value;
}

void PiController_Reset(PiController *controller)
{
    controller->integral = 0.0f;
}

float PiController_Update(PiController *controller, float error, float dt_s)
{
    float proportional = controller->kp * error;
    float candidate_integral;
    float candidate_output;

    if (dt_s <= 0.0f)
    {
        return PiController_Clamp(proportional + controller->ki * controller->integral,
                                  controller->output_limit);
    }

    candidate_integral = PiController_Clamp(controller->integral + error * dt_s,
                                            controller->integral_limit);
    candidate_output = proportional + controller->ki * candidate_integral;

    /* Conditional integration prevents wind-up while output is saturated. */
    if ((candidate_output <= controller->output_limit) &&
        (candidate_output >= -controller->output_limit))
    {
        controller->integral = candidate_integral;
    }
    return PiController_Clamp(proportional + controller->ki * controller->integral,
                              controller->output_limit);
}
