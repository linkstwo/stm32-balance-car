#include "board.h"

void Board_PinMux_Init(void)
{
    static u8 initialized = 0;

    if (initialized != 0)
    {
        return;
    }

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /* Release PA15/PB3/PB4 from JTAG while keeping PA13/PA14 SWD enabled. */
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    initialized = 1;
}
