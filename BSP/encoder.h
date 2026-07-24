#ifndef BSP_ENCODER_H
#define BSP_ENCODER_H

#include <stdint.h>

typedef struct
{
    int16_t left_delta_counts;
    int16_t right_delta_counts;
} EncoderDeltaCounts;

void Encoder_Init(void);
EncoderDeltaCounts Encoder_ReadDeltaCounts(void);
float Encoder_CountsPerSecond(int16_t delta_counts, uint32_t sample_period_ms);

#endif
