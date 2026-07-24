#include "filters.h"

#define FILTER_TWO_PI 6.283185307f

void FirstOrderFilter_Reset(FirstOrderFilter *filter, float value)
{
    filter->value = value;
    filter->initialized = true;
}

float FirstOrderFilter_Update(FirstOrderFilter *filter, float input, float cutoff_hz, float dt_s)
{
    float alpha;

    if ((!filter->initialized) || (cutoff_hz <= 0.0f) || (dt_s <= 0.0f))
    {
        FirstOrderFilter_Reset(filter, input);
        return input;
    }
    alpha = (FILTER_TWO_PI * cutoff_hz * dt_s) /
            (1.0f + FILTER_TWO_PI * cutoff_hz * dt_s);
    filter->value += alpha * (input - filter->value);
    return filter->value;
}
