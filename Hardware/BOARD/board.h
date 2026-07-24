#ifndef __BOARD_H
#define __BOARD_H

#include "stm32f10x.h"

/* Configure shared STM32F103 board pin functions before peripheral setup. */
void Board_PinMux_Init(void);

#endif
