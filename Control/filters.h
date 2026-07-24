#ifndef CONTROL_FILTERS_H
#define CONTROL_FILTERS_H

#include <stdbool.h>

typedef struct
{
    float value;
    bool initialized;
} FirstOrderFilter;

void FirstOrderFilter_Reset(FirstOrderFilter *filter, float value);
float FirstOrderFilter_Update(FirstOrderFilter *filter, float input, float cutoff_hz, float dt_s);

#endif
